using System.Diagnostics;
using System.Reflection;

namespace gui_app
{
    // I have to do this for every file? :/
    using OBJModelHandle = System.UInt64;
    using SceneHandle = System.UInt64;
    using CameraHandle = System.UInt64;
    using ImageHandle = System.UInt64;

    record vec3(float x, float y, float z);

    record OBJSceneDesc(
        string sceneName,
        string objPath,
        vec3 cameraLocation,
        vec3 cameraLookAt);

    public partial class MainForm : Form
    {
        private static OBJSceneDesc[] sceneDescs = {
            new(
                "CornellBox",
                "content/cornell_box/CornellBox-Mirror.obj",
                new(0.0f, 1.0f, 4.0f),
                new(0.0f, 1.0f, -1.0f)
            )
        };

        public MainForm()
        {
            InitializeComponent();
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            loggerBox.AppendText("= Software Raytracer =" + Environment.NewLine);

            if (RaylibWrapper.Raylib_Initialize() == 0)
            {
                loggerBox.AppendText("Failed to load raylib.dll" + Environment.NewLine);
            }
            else
            {
                loggerBox.AppendText("Load raylib.dll" + Environment.NewLine);
            }

#if false
            string[] sceneDescs = {
                "CornellBox",
                "BreakfastRoom",
                "DabrovicSponza",
                "FireplaceRoom",
                "LivingRoom",
                "SibenikCathedral",
                "SanMiguel",
            };
#endif
            foreach (OBJSceneDesc sceneDesc in sceneDescs)
            {
                sceneList.Items.Add(sceneDesc.sceneName);
            }
            sceneList.SelectedIndex = 0;
        }

        private void executeButton_Click(object sender, EventArgs e)
        {
            loggerBox.AppendText(String.Format("viewport: {0}, {1}", viewport.Width, viewport.Height) + Environment.NewLine);

            if (sceneList.SelectedIndex < 0 && sceneList.SelectedIndex >= sceneDescs.Length)
            {
                return;
            }

            uint viewportWidth = (uint)viewport.Width;
            uint viewportHeight = (uint)viewport.Height;

            SceneHandle sceneHandle = RaylibWrapper.Raylib_CreateScene();
            CameraHandle cameraHandle = RaylibWrapper.Raylib_CreateCamera();
            ImageHandle mainImage = RaylibWrapper.Raylib_CreateImage(viewportWidth, viewportHeight);

            // TODO: Hard-coded only for x64 config
            OBJSceneDesc sceneDesc = sceneDescs[sceneList.SelectedIndex];
            string fullpath = Path.GetFullPath("../../../../../../" + sceneDesc.objPath);

            OBJModelHandle objHandle = RaylibWrapper.Raylib_LoadOBJModel(fullpath);
            if (objHandle == 0)
            {
                loggerBox.AppendText("Failed to load " + fullpath + Environment.NewLine);
            }
            else
            {
                loggerBox.AppendText("Load " + fullpath + Environment.NewLine);
            }

            RaylibWrapper.Raylib_AddOBJModelToScene(sceneHandle, objHandle);

            RaylibWrapper.Raylib_FinalizeScene(sceneHandle);

            RaylibWrapper.Raylib_CameraSetPosition(cameraHandle, sceneDesc.cameraLocation.x, sceneDesc.cameraLocation.y, sceneDesc.cameraLocation.z);
            RaylibWrapper.Raylib_CameraSetLookAt(cameraHandle, sceneDesc.cameraLookAt.x, sceneDesc.cameraLookAt.y, sceneDesc.cameraLookAt.z);
            RaylibWrapper.Raylib_CameraSetPerspective(cameraHandle, 60.0f, (float)viewportWidth / viewportHeight);
            RaylibWrapper.Raylib_CameraSetLens(cameraHandle, 0.01f, 5.0f);
            RaylibWrapper.Raylib_CameraSetMotion(cameraHandle, 0.0f, 0.0f);

            RaylibWrapper.RendererSettings settings = new RaylibWrapper.RendererSettings();
            settings.viewportWidth = viewportWidth;
            settings.viewportHeight = viewportHeight;
            settings.samplesPerPixel = 10;
            settings.maxPathLength = 5;
            settings.rayTMin = 0.0001f;
            settings.renderMode = 0;

            loggerBox.AppendText("Render..." + Environment.NewLine);
            Debug.WriteLine("Render...");

            // TODO: Crashes here (access violation?)
            RaylibWrapper.Raylib_Render(ref settings, sceneHandle, cameraHandle, mainImage);

            loggerBox.AppendText("PostProcess..." + Environment.NewLine);
            Debug.WriteLine("PostProcess...");

            RaylibWrapper.Raylib_PostProcess(mainImage);

            loggerBox.AppendText("Display..." + Environment.NewLine);
            Debug.WriteLine("Display...");

            float[] mainImageData = new float[viewportWidth * viewportHeight * 3];
            RaylibWrapper.Raylib_DumpImageData(mainImage, mainImageData);

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

            // Fill random data.
            // TODO: Fill ray traced image.
            int k = 0;
            for (int y = 0; y < renderTarget.Height; ++y)
            {
                for (int x = 0; x < renderTarget.Width; ++x)
                {
                    int p = y * rawBuffer.Stride + (3 * x);
                    //byte k = (byte)((x ^ y) & 0xff);
                    //rgbValues[p + 0] = k;
                    //rgbValues[p + 1] = k;
                    //rgbValues[p + 2] = k;
                    rgbValues[p + 0] = (byte)(mainImageData[k + 0] * 255.0f);
                    rgbValues[p + 1] = (byte)(mainImageData[k + 1] * 255.0f);
                    rgbValues[p + 2] = (byte)(mainImageData[k + 2] * 255.0f);
                    k += 3;
                }
            }

            System.Runtime.InteropServices.Marshal.Copy(rgbValues, 0, rawBuffer.Scan0, nBytes);
            renderTarget.UnlockBits(rawBuffer);

            viewport.Image = renderTarget;

            // Cleanup
            RaylibWrapper.Raylib_UnloadOBJModel(objHandle);
            RaylibWrapper.Raylib_DestroyScene(sceneHandle);
            RaylibWrapper.Raylib_DestroyCamera(cameraHandle);
            RaylibWrapper.Raylib_DestroyImage(mainImage);
        }
    }
}