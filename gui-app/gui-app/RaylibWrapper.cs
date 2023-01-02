using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace gui_app
{
    using OBJModelHandle = System.UInt64;
    using SceneHandle = System.UInt64;
    using CameraHandle = System.UInt64;
    using ImageHandle = System.UInt64;

    // Wrapper for raylib.dll which is built from my C++ project.
    internal static class RaylibWrapper
    {
        internal enum ERenderMode : uint
        {
            Default            = 0, // Path tracing.
	        Albedo             = 1,
	        SurfaceNormal      = 2, // In world space.
	        MicrosurfaceNormal = 3, // In world space.
	        Texcoord           = 4, // Surface parameterization.
	        Emission           = 5,
	        Reflectance        = 6, // Can be very noisy for surfaces with diffuse materials.

	        MAX
        }

        [StructLayout(LayoutKind.Sequential)]
        internal struct RendererSettings
        {
            internal uint  viewportWidth;
            internal uint  viewportHeight;

            internal int   samplesPerPixel;
            internal int   maxPathLength;
            internal float rayTMin;

            internal uint  renderMode;
        }

        // -----------------------------------------------------------------------
        // Library initialization & termination

        [DllImport("raylib.dll")]
        internal static extern int Raylib_Initialize();

        [DllImport("raylib.dll")]
        internal static extern int Raylib_Terminate();

        // -----------------------------------------------------------------------
        // Manage media files

        [DllImport("raylib.dll")]
        internal static extern OBJModelHandle Raylib_LoadOBJModel(string objPath);

        //Raylib_TransformOBJModel

        [DllImport("raylib.dll")]
        internal static extern void Raylib_FinalizeOBJModel(OBJModelHandle objModel);

        [DllImport("raylib.dll")]
        internal static extern int Raylib_UnloadOBJModel(OBJModelHandle objHandle);

        //Raylib_LoadImage

        // -----------------------------------------------------------------------
        // Scene

        [DllImport("raylib.dll")]
        internal static extern SceneHandle Raylib_CreateScene();

        //Raylib_AddSceneElement

        [DllImport("raylib.dll")]
        internal static extern void Raylib_AddOBJModelToScene(SceneHandle scene, OBJModelHandle obj);

        //Raylib_SetSkyPanorama

        [DllImport("raylib.dll")]
        internal static extern void Raylib_SetSunIlluminance(SceneHandle scene, float r, float g, float b);

        [DllImport("raylib.dll")]
        internal static extern void Raylib_SetSunDirection(SceneHandle scene, float x, float y, float z);

        [DllImport("raylib.dll")]
        internal static extern void Raylib_FinalizeScene(SceneHandle scene);

        [DllImport("raylib.dll")]
        internal static extern int Raylib_DestroyScene(SceneHandle scene);

        [DllImport("raylib.dll")]
        internal static extern CameraHandle Raylib_CreateCamera();

        [DllImport("raylib.dll")]
        internal static extern void Raylib_CameraSetPosition(CameraHandle camera, float x, float y, float z);

        [DllImport("raylib.dll")]
        internal static extern void Raylib_CameraSetLookAt(CameraHandle camera, float x, float y, float z);

        [DllImport("raylib.dll")]
        internal static extern void Raylib_CameraSetPerspective(CameraHandle camera, float fovY_degree, float aspectWH);

        [DllImport("raylib.dll")]
        internal static extern void Raylib_CameraSetLens(CameraHandle camera, float aperture, float focalDistance);

        [DllImport("raylib.dll")]
        internal static extern void Raylib_CameraSetMotion(CameraHandle camera, float beginTime, float endTime);

        [DllImport("raylib.dll")]
        internal static extern void Raylib_CameraCopy(CameraHandle srcCamera, CameraHandle dstCamera);

        [DllImport("raylib.dll")]
        internal static extern void Raylib_DestroyCamera(CameraHandle camera);

        [DllImport("raylib.dll")]
        internal static extern ImageHandle Raylib_CreateImage(uint width, uint height);

        [DllImport("raylib.dll")]
        internal static extern void Raylib_DumpImageData(ImageHandle image, float[] imageData);

        [DllImport("raylib.dll")]
        internal static extern int Raylib_DestroyImage(ImageHandle image);

        // -----------------------------------------------------------------------
        // Rendering

        [DllImport("raylib.dll")]
        internal static extern void Raylib_Render(
            ref RendererSettings settings,
            SceneHandle scene,
            CameraHandle camera,
            ImageHandle outMainImage);

        [DllImport("raylib.dll")]
        internal static extern int Raylib_Denoise(
            ImageHandle mainImage,
            int bMainImageHDR,
            ImageHandle albedoImage,
            ImageHandle normalImage,
            ImageHandle outDenoisedImage);

        [DllImport("raylib.dll")]
        internal static extern void Raylib_PostProcess(ImageHandle image);

        [DllImport("raylib.dll")]
        internal static extern int Raylib_IsDenoiserSupported();

        // -----------------------------------------------------------------------
        // Utils

        //Raylib_GetRenderModeString
        //Raylib_WriteImageToDisk
        //Raylib_FlushLogThread
    }
}
