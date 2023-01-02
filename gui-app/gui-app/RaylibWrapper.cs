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
        [StructLayout(LayoutKind.Sequential)]
        internal struct RendererSettings
        {
            internal uint  viewportWidth;
            internal uint  viewportHeight;

            internal int   samplesPerPixel;
            internal int   maxPathLength;
            internal float rayTMin;

            internal uint  renderMode;
        };

        // -----------------------------------------------------------------------
        // Library initialization & termination

        [DllImport("raylib.dll")]
        internal static extern int Raylib_Initialize();

        [DllImport("raylib.dll")]
        internal static extern int Raylib_Terminate();

        // -----------------------------------------------------------------------
        // Manage media files

        [DllImport("raylib.dll")]
        internal static extern OBJModelHandle Raylib_LoadOBJModel(String objPath);

        //Raylib_TransformOBJModel
        //Raylib_FinalizeOBJModel

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
        //Raylib_SetSunIlluminance
        //Raylib_SetSunDirection

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

        //Raylib_CameraCopy

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

        //Raylib_Denoise

        [DllImport("raylib.dll")]
        internal static extern void Raylib_PostProcess(ImageHandle image);

        //Raylib_IsDenoiserSupported

        // -----------------------------------------------------------------------
        // Utils

        //Raylib_GetRenderModeString
        //Raylib_WriteImageToDisk
        //Raylib_FlushLogThread
    }
}
