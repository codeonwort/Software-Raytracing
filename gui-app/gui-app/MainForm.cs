using System.Reflection;

namespace gui_app
{
    public partial class MainForm : Form
    {
        public MainForm()
        {
            InitializeComponent();

            //string raylibPath = Path.GetFullPath("./raylib.dll");
            //raylibDLL = Assembly.LoadFile(raylibPath);
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            loggerBox.AppendText("= Software Raytracer =" + Environment.NewLine);
            
            if (raylibDLL == null)
            {
                loggerBox.AppendText("Failed to load raylib.dll" + Environment.NewLine);
            }
            else
            {
                loggerBox.AppendText("Load raylib.dll" + Environment.NewLine);
            }

            string[] sceneDescs = {
                "CornellBox",
                "BreakfastRoom",
                "DabrovicSponza",
                "FireplaceRoom",
                "LivingRoom",
                "SibenikCathedral",
                "SanMiguel",
            };
            foreach (string sceneDesc in sceneDescs)
            {
                sceneList.Items.Add(sceneDesc);
            }
            sceneList.SelectedIndex = 0;
        }

        private void executeButton_Click(object sender, EventArgs e)
        {
            loggerBox.AppendText(String.Format("viewport: {0}, {1}", viewport.Width, viewport.Height) + Environment.NewLine);
            
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

        private Assembly raylibDLL;
    }
}