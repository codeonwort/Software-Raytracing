#include "raylib.h"
#include "core/platform.h"
#include "core/concurrent_vector.h"
#include "core/logger.h"
#include "geom/scene.h"
#include "geom/transform.h"
#include "geom/static_mesh.h"
#include "render/camera.h"
#include "render/image.h"
#include "render/renderer.h"
#include "loader/obj_loader.h"
#include "loader/dll_loader.h"

#include <iostream>

// -----------------------------------------------------------------------

static concurrent_vector<OBJModel*> g_objModels;
static concurrent_vector<Camera*>   g_cameras;
static concurrent_vector<Image2D*>  g_images;
static concurrent_vector<Scene*>    g_scenes;

// -----------------------------------------------------------------------

int32_t Raylib_Initialize()
{
	std::cout << "Initialize raylib" << std::endl;

	Logger::StartLogThread();

	// Runtime-load thirdparty DLLs
	if (!FreeImage::LoadDLL())
	{
		std::cerr << __FUNCTION__ << ": Failed to load FreeImage.dll" << std::endl;
		return 0;
	}

	OBJLoader::Initialize();

	return 1;
}

int32_t Raylib_Terminate()
{
	std::cout << "Terminate raylib" << std::endl;

	OBJLoader::Destroy();
	Logger::KillAndWaitForLogThread();

	return 0;
}

// -----------------------------------------------------------------------
// Manage media files

OBJModelHandle Raylib_LoadOBJModel(const char* objPath)
{
	OBJModel* objModel = new OBJModel;
	if (OBJLoader::LoadModelFromFile(objPath, objModel))
	{
		g_objModels.push_back(objModel);
		return (OBJModelHandle)objModel;
	}
	else
	{
		delete objModel;
	}
	return NULL;
}

void Raylib_TransformOBJModel(
	OBJModelHandle objModelHandle,
	float translationX, float translationY, float translationZ,
	float yaw, float pitch, float roll,
	float scaleX, float scaleY, float scaleZ)
{
	OBJModel* objModel = (OBJModel*)objModelHandle;

	Transform transform;
	transform.Init(
		vec3(translationX, translationY, translationZ),
		Rotator(yaw, pitch, roll),
		vec3(scaleX, scaleY, scaleZ));

	std::for_each(
		objModel->staticMeshes.begin(),
		objModel->staticMeshes.end(),
		[&transform](StaticMesh* mesh) { mesh->ApplyTransform(transform); }
	);
}

void Raylib_FinalizeOBJModel(OBJModelHandle objModel)
{
	((OBJModel*)objModel)->FinalizeAllMeshes();
}

int32_t Raylib_UnloadOBJModel(OBJModelHandle objHandle)
{
	OBJModel* objModel = (OBJModel*)objHandle;
	if (g_objModels.erase_first(objModel))
	{
		delete objModel;
		return true;
	}
	return false;
}

ImageHandle Raylib_LoadImage(const char* filepath)
{
	Image2D* image = ImageIO::LoadImage2DFromFile(filepath);
	g_images.push_back(image);
	return (ImageHandle)image;
}

// -----------------------------------------------------------------------
// Scene

CameraHandle Raylib_CreateCamera()
{
	Camera* camera = new Camera;
	g_cameras.push_back(camera);
	return (CameraHandle)camera;
}

void Raylib_CameraSetPosition(CameraHandle cameraHandle, float x, float y, float z)
{
	Camera* camera = (Camera*)cameraHandle;
	camera->origin = vec3(x, y, z);
	camera->UpdateInternal();
}

void Raylib_CameraSetLookAt(CameraHandle cameraHandle, float tx, float ty, float tz)
{
	Camera* camera = (Camera*)cameraHandle;
	camera->lookAt = vec3(tx, ty, tz);
	camera->UpdateInternal();
}

void Raylib_CameraSetPerspective(CameraHandle cameraHandle, float fovY_degrees, float aspectWH)
{
	Camera* camera = (Camera*)cameraHandle;
	camera->fovY_degrees = fovY_degrees;
	camera->aspectWH = aspectWH;
	camera->UpdateInternal();
}

void Raylib_CameraSetLens(CameraHandle cameraHandle, float aperture, float focalDistance)
{
	Camera* camera = (Camera*)cameraHandle;
	camera->aperture = aperture;
	camera->focalDistance = focalDistance;
	camera->UpdateInternal();
}

void Raylib_CameraSetMotion(CameraHandle cameraHandle, float beginTime, float endTime)
{
	Camera* camera = (Camera*)cameraHandle;
	camera->beginTime = beginTime;
	camera->endTime = endTime;
	camera->UpdateInternal();
}

void Raylib_CameraCopy(CameraHandle srcCamera, CameraHandle dstCamera)
{
	Camera* src = (Camera*)srcCamera;
	Camera* dst = (Camera*)dstCamera;
	*dst = *src;
}

int32_t Raylib_DestroyCamera(CameraHandle cameraHandle)
{
	Camera* camera = (Camera*)cameraHandle;
	if (g_cameras.erase_first(camera))
	{
		delete camera;
		return true;
	}
	return false;
}

ImageHandle Raylib_CreateImage(uint32_t width, uint32_t height)
{
	Image2D* image = new Image2D(width, height, 0x0);
	g_images.push_back(image);
	return (ImageHandle)image;
}

void Raylib_DumpImageData(ImageHandle imageHandle, float* outDest)
{
	Image2D* image = (Image2D*)imageHandle;
	image->DumpFloatRGBs(outDest);
}

int32_t Raylib_DestroyImage(ImageHandle imageHandle)
{
	Image2D* image = (Image2D*)imageHandle;
	if (g_images.erase_first(image))
	{
		delete image;
		return true;
	}
	return false;
}

SceneHandle Raylib_CreateScene()
{
	Scene* scene = new Scene;
	g_scenes.push_back(scene);
	return (SceneHandle)scene;
}

void Raylib_FinalizeScene(SceneHandle scene)
{
	((Scene*)scene)->Finalize();
}

int32_t Raylib_DestroyScene(SceneHandle sceneHandle)
{
	Scene* scene = (Scene*)sceneHandle;
	if (g_scenes.erase_first(scene))
	{
		delete scene;
		return true;
	}
	return false;
}

// -----------------------------------------------------------------------
// Rendering

void Raylib_Render(
	const RendererSettings* settings,
	SceneHandle scene,
	CameraHandle camera,
	ImageHandle outMainImage)
{
	Renderer renderer;
	renderer.RenderScene(settings, (Scene*)scene, (Camera*)camera, (Image2D*)outMainImage);
}

int32_t Raylib_Denoise(
	ImageHandle inMainImage,
	int32_t bMainImageHDR,
	ImageHandle inAlbedoImage,
	ImageHandle inNormalImage,
	ImageHandle outDenoisedImage)
{
	Renderer renderer;
	bool bRet = renderer.DenoiseScene(
		(Image2D*)inMainImage,
		(bool)bMainImageHDR,
		(Image2D*)inAlbedoImage,
		(Image2D*)inNormalImage,
		(Image2D*)outDenoisedImage);
	return (bRet ? 1 : 0);
}

void Raylib_AddSceneElement(SceneHandle scene, SceneElementHandle element)
{
	Hitable* hitable = (Hitable*)element;
	((Scene*)scene)->AddSceneElement(hitable);
}

void Raylib_AddOBJModelToScene(SceneHandle scene, OBJModelHandle objModel)
{
	Hitable* objModelRoot = ((OBJModel*)objModel)->rootObject;
	((Scene*)scene)->AddSceneElement(objModelRoot);
}

void Raylib_SetSkyPanorama(SceneHandle scene, ImageHandle skyImage)
{
	((Scene*)scene)->SetSkyPanorama(skyImage);
}

void Raylib_SetSunIlluminance(SceneHandle scene, float r, float g, float b)
{
	((Scene*)scene)->SetSunIlluminance(vec3(r, g, b));
}

void Raylib_SetSunDirection(SceneHandle scene, float x, float y, float z)
{
	((Scene*)scene)->SetSunDirection(vec3(x, y, z));
}

void Raylib_PostProcess(ImageHandle image)
{
	((Image2D*)image)->PostProcess();
}

int32_t Raylib_IsDenoiserSupported()
{
	return Renderer::IsDenoiserSupported() ? 1 : 0;
}

// -----------------------------------------------------------------------
// Utils

RAYLIB_API const char* Raylib_GetRenderModeString(uint32_t auxMode)
{
	static const char* enumStrs[] = {
		"Default",
		"Albedo",
		"SurfaceNormal",
		"MicrosurfaceNormal",
		"Texcoord",
		"Emission",
		"Reflectance",
	};

	bool bValid = (0 <= auxMode && auxMode < RAYLIB_RENDERMODE_MAX);
	return bValid ? enumStrs[auxMode] : nullptr;
}

int32_t Raylib_WriteImageToDisk(ImageHandle imageHandle, const char* filepath, uint32_t fileTypeRaw)
{
	if (imageHandle == NULL || filepath == nullptr || fileTypeRaw >= EImageFileType::RAYLIB_IMAGEFILETYPE_MAX)
	{
		return 0;
	}

	Image2D* image = (Image2D*)imageHandle;
	EImageFileType fileType = (EImageFileType)fileTypeRaw;

	bool bRet = ImageIO::WriteImage2DToDisk(image, filepath, fileType);
	return bRet ? 1 : 0;
}

void Raylib_FlushLogThread()
{
	Logger::FlushLogThread();
}
