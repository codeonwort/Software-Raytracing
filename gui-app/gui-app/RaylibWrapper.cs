using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace gui_app
{
    using OBJModelHandle = System.UInt64;

    // Wrapper for raylib.dll which is built from my C++ project.
    internal static class RaylibWrapper
    {
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
        //Raylib_UnloadOBJModel
        //Raylib_LoadImage

        // -----------------------------------------------------------------------
        // Scene

        //Raylib_CreateScene
        //Raylib_AddSceneElement
        //Raylib_AddOBJModelToScene
        //Raylib_SetSkyPanorama
        //Raylib_SetSunIlluminance
        //Raylib_SetSunDirection
        //Raylib_FinalizeScene
        //Raylib_DestroyScene
        //Raylib_CreateCamera
        //Raylib_CameraSetPosition
        //Raylib_CameraSetLookAt
        //Raylib_CameraSetPerspective
        //Raylib_CameraSetLens
        //Raylib_CameraSetMotion
        //Raylib_CameraCopy
        //Raylib_DestroyCamera
        //Raylib_CreateImage
        //Raylib_DestroyImage

        // -----------------------------------------------------------------------
        // Rendering

        //Raylib_Render
        //Raylib_Denoise
        //Raylib_PostProcess
        //Raylib_IsDenoiserSupported

        // -----------------------------------------------------------------------
        // Utils

        //Raylib_GetRenderModeString
        //Raylib_WriteImageToDisk
        //Raylib_FlushLogThread
    }
}
