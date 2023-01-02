using System.Reflection;

namespace gui_app
{
    // I have to do this for every file? :/
    using OBJModelHandle = System.UInt64;

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

            OBJSceneDesc sceneDesc = sceneDescs[sceneList.SelectedIndex];
            string fullpath = Path.GetFullPath("../../../../../" + sceneDesc.objPath);

            OBJModelHandle objHandle = RaylibWrapper.Raylib_LoadOBJModel(fullpath);
            if (objHandle == 0)
            {
                loggerBox.AppendText("Failed to load " + fullpath);
            }
            else
            {
                loggerBox.AppendText("Load " + fullpath);
            }

            // TODO: Render the loaded OBJ model
            // ...

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
            for (int y = 0; y < renderTarget.Height; ++y)
            {
                for (int x = 0; x < renderTarget.Width; ++x)
                {
                    int p = y * rawBuffer.Stride + (3 * x);
                    byte k = (byte)((x ^ y) & 0xff);
                    rgbValues[p + 0] = k;
                    rgbValues[p + 1] = k;
                    rgbValues[p + 2] = k;
                }
            }

            System.Runtime.InteropServices.Marshal.Copy(rgbValues, 0, rawBuffer.Scan0, nBytes);
            renderTarget.UnlockBits(rawBuffer);

            viewport.Image = renderTarget;
        }
    }
}