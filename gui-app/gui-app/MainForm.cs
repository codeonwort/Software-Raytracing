using System.Text.Json;

namespace GuiApp
{
    // I have to do this for every file? :/
    using OBJModelHandle = System.UInt64;
    using SceneHandle = System.UInt64;
    using CameraHandle = System.UInt64;
    using ImageHandle = System.UInt64;

    public partial class MainForm : Form
    {
        private const string SCENE_FILE = "scenes.json";

        private bool bRaylibValid;
        private SceneDescriptions sceneDescs = null;

        public MainForm()
        {
            InitializeComponent();
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            loggerBox.AppendText("= Software Raytracer =" + Environment.NewLine);

            bRaylibValid = false;
            CheckRaylibDLL();

            LoadSceneFile();

            foreach (var sceneDesc in sceneDescs.scenes)
            {
                sceneList.Items.Add(sceneDesc.name);
            }
            sceneList.SelectedIndex = 0;
        }

        private void CheckRaylibDLL()
        {
            try
            {
                if (RaylibWrapper.Raylib_Initialize() == 0)
                {
                    loggerBox.AppendText("Failed to load raylib.dll" + Environment.NewLine);
                }
                else
                {
                    loggerBox.AppendText("Load raylib.dll" + Environment.NewLine);
                    bRaylibValid = true;
                }
            }
            catch (System.DllNotFoundException)
            {
                loggerBox.AppendText("raylib.dll not found" + Environment.NewLine);
            }
        }

        private void LoadSceneFile()
        {
            string jsonPath = FindContentPath(SCENE_FILE);
            string document = File.ReadAllText(jsonPath);
            try
            {
                sceneDescs = JsonSerializer.Deserialize<SceneDescriptions>(document);
                if (sceneDescs == null)
                {
                    loggerBox.AppendText($"{SCENE_FILE} was not loaded" + Environment.NewLine);
                }
                else
                {
                    loggerBox.AppendText($"Load {SCENE_FILE}" + Environment.NewLine);
                }
            }
            catch (Exception)
            {
                loggerBox.AppendText($"{SCENE_FILE} was not loaded" + Environment.NewLine);
            }
        }

        private void ExecuteButton_Click(object sender, EventArgs e)
        {
            loggerBox.AppendText(String.Format("viewport: {0}, {1}", viewport.Width, viewport.Height) + Environment.NewLine);

            if (sceneDescs == null)
            {
                loggerBox.AppendText($"{SCENE_FILE} was not loaded. Can't start rendering." + Environment.NewLine);
                return;
            }
            if (sceneList.SelectedIndex < 0 && sceneList.SelectedIndex >= sceneDescs.scenes.Length)
            {
                loggerBox.AppendText($"Selected scene index {sceneList.SelectedIndex} is wrong");
                return;
            }

            if (bRaylibValid)
            {
                var sceneDesc = sceneDescs.scenes[sceneList.SelectedIndex];
                int spp = Math.Max(1, (int)inputSPP.Value);
                int pathLen = Math.Max(1, (int)inputPathLen.Value);

                RunRaytracer(sceneDesc, spp, pathLen);
            }
            else
            {
                loggerBox.AppendText("raylib.dll was not loaded. Can't start rendering." + Environment.NewLine);
            }
        }

        private void RunRaytracer(SceneDescriptions.Scene sceneDesc, int spp, int maxPathLen)
        {
            uint viewportWidth = (uint)viewport.Width;
            uint viewportHeight = (uint)viewport.Height;

            bool bRunDenoiser = (0 != RaylibWrapper.Raylib_IsDenoiserSupported());

            //
            // Scene
            //

            string fullpath = FindContentPath(sceneDesc.filepath);
            OBJModelHandle objHandle = 0;
            if (fullpath.Length > 0)
            {
                objHandle = RaylibWrapper.Raylib_LoadOBJModel(fullpath);
            }

            if (objHandle == 0)
            {
                loggerBox.AppendText("Failed to load " + sceneDesc.filepath + Environment.NewLine);
                return;
            }
            else
            {
                RaylibWrapper.Raylib_FinalizeOBJModel(objHandle);
                loggerBox.AppendText("Load " + fullpath + Environment.NewLine);
            }

            SceneHandle sceneHandle = RaylibWrapper.Raylib_CreateScene();
            CameraHandle cameraHandle = RaylibWrapper.Raylib_CreateCamera();
            ImageHandle mainImage = RaylibWrapper.Raylib_CreateImage(viewportWidth, viewportHeight);

            RaylibWrapper.Raylib_AddOBJModelToScene(sceneHandle, objHandle);

            vec3 normalize(vec3 v)
            {
                float k = (float)Math.Sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
                return new(v.x / k, v.y / k, v.z / k);
            }
            vec3 sunDir = normalize(sceneDesc.sunDirection);

            RaylibWrapper.Raylib_SetSunIlluminance(sceneHandle, sceneDesc.sunIlluminance.x, sceneDesc.sunIlluminance.y, sceneDesc.sunIlluminance.z);
            RaylibWrapper.Raylib_SetSunDirection(sceneHandle, sunDir.x, sunDir.y, sunDir.z);

            RaylibWrapper.Raylib_FinalizeScene(sceneHandle);

            // Camera
            RaylibWrapper.Raylib_CameraSetPosition(cameraHandle, sceneDesc.cameraLocation.x, sceneDesc.cameraLocation.y, sceneDesc.cameraLocation.z);
            RaylibWrapper.Raylib_CameraSetLookAt(cameraHandle, sceneDesc.cameraLookAt.x, sceneDesc.cameraLookAt.y, sceneDesc.cameraLookAt.z);
            RaylibWrapper.Raylib_CameraSetPerspective(cameraHandle, 60.0f, (float)viewportWidth / viewportHeight);
            RaylibWrapper.Raylib_CameraSetLens(cameraHandle, 0.0f, 1.0f);
            RaylibWrapper.Raylib_CameraSetMotion(cameraHandle, 0.0f, 0.0f);

            // RendererSettings
            RaylibWrapper.RendererSettings settings = new RaylibWrapper.RendererSettings();
            settings.viewportWidth   = viewportWidth;
            settings.viewportHeight  = viewportHeight;
            settings.samplesPerPixel = spp;
            settings.maxPathLength   = maxPathLen;
            settings.rayTMin         = 0.0001f;
            settings.renderMode      = (uint)RaylibWrapper.ERenderMode.Default;

            // Render the scene.
            loggerBox.AppendText("Render..." + Environment.NewLine);

            RaylibWrapper.Raylib_Render(ref settings, sceneHandle, cameraHandle, mainImage);

            float[] finalImageData = new float[viewportWidth * viewportHeight * 3];

            if (bRunDenoiser)
            {
                ImageHandle albedoImage = RaylibWrapper.Raylib_CreateImage(viewportWidth, viewportHeight);
                ImageHandle normalImage = RaylibWrapper.Raylib_CreateImage(viewportWidth, viewportHeight);
                ImageHandle denoisedImage = RaylibWrapper.Raylib_CreateImage(viewportWidth, viewportHeight);

                // Force aperture=0
                CameraHandle auxCamera = RaylibWrapper.Raylib_CreateCamera();
                RaylibWrapper.Raylib_CameraCopy(cameraHandle, auxCamera);
                RaylibWrapper.Raylib_CameraSetLens(auxCamera, 0.0f, 1.0f);

                // Render aux images
                RaylibWrapper.RendererSettings auxSettings = settings;
                auxSettings.renderMode = (uint)RaylibWrapper.ERenderMode.Albedo;
                RaylibWrapper.Raylib_Render(ref auxSettings, sceneHandle, auxCamera, albedoImage);
                auxSettings.renderMode = (uint)RaylibWrapper.ERenderMode.MicrosurfaceNormal;
                RaylibWrapper.Raylib_Render(ref auxSettings, sceneHandle, auxCamera, normalImage);

                RaylibWrapper.Raylib_Denoise(mainImage, 1, albedoImage, normalImage, denoisedImage);
                
                RaylibWrapper.Raylib_PostProcess(denoisedImage);
                RaylibWrapper.Raylib_PostProcess(mainImage);

                RaylibWrapper.Raylib_DumpImageData(denoisedImage, finalImageData);

                RaylibWrapper.Raylib_DestroyCamera(auxCamera);
                RaylibWrapper.Raylib_DestroyImage(albedoImage);
                RaylibWrapper.Raylib_DestroyImage(normalImage);
                RaylibWrapper.Raylib_DestroyImage(denoisedImage);
            }
            else
            {
                RaylibWrapper.Raylib_PostProcess(mainImage);
                RaylibWrapper.Raylib_DumpImageData(mainImage, finalImageData);
            }

            // Create a bitmap to update the viewport.
            Bitmap renderTarget = new Bitmap(
                viewport.Width,
                viewport.Height,
                System.Drawing.Imaging.PixelFormat.Format24bppRgb);

            Rectangle rect = new Rectangle(0, 0, renderTarget.Width, renderTarget.Height);
            System.Drawing.Imaging.BitmapData rawBuffer = renderTarget.LockBits(
                rect,
                System.Drawing.Imaging.ImageLockMode.ReadWrite,
                renderTarget.PixelFormat);

            int nBytes = Math.Abs(rawBuffer.Stride) * renderTarget.Height;
            byte[] rgbValues = new byte[nBytes];

            // Fill the bitmap with raytracing output.
            int k = 0;
            for (int y = 0; y < renderTarget.Height; ++y)
            {
                for (int x = 0; x < renderTarget.Width; ++x)
                {
                    int p = y * rawBuffer.Stride + (3 * x);
                    rgbValues[p + 2] = (byte)(finalImageData[k + 0] * 255.0f);
                    rgbValues[p + 1] = (byte)(finalImageData[k + 1] * 255.0f);
                    rgbValues[p + 0] = (byte)(finalImageData[k + 2] * 255.0f);
                    k += 3;
                }
            }

            System.Runtime.InteropServices.Marshal.Copy(rgbValues, 0, rawBuffer.Scan0, nBytes);
            renderTarget.UnlockBits(rawBuffer);

            viewport.Image = renderTarget;

            loggerBox.AppendText("Done." + Environment.NewLine);

            // Cleanup
            RaylibWrapper.Raylib_UnloadOBJModel(objHandle);
            RaylibWrapper.Raylib_DestroyScene(sceneHandle);
            RaylibWrapper.Raylib_DestroyCamera(cameraHandle);
            RaylibWrapper.Raylib_DestroyImage(mainImage);
        }

        private static string FindContentPath(string subpath)
        {
            string cd = "./";
            int cnt = 10;
            while (cnt-- > 0)
            {
                string fullpath = Path.Combine(cd, subpath);
                if (Path.Exists(fullpath))
                {
                    return Path.GetFullPath(fullpath);
                }
                cd = cd + "../";
            }
            return "";
        }
    }
}