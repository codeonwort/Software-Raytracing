# Overview

Implement CPU raytracer.

**Project structure**

```
raylib/  : Contains raytracing implementation. Built as a DLL.
src/     : CUI application that performs basic Read-Eval-Print loop.
           Select a demo scene, adjust rendering options, and execute raytracer.
gui-app/ : GUI version of the CUI app. Written in C# for practice.
```

**How to build**

1. Run `Setup.ps1`.
2. Build at the top level using CMake. This should build `raylib` and `src`.
3. Build `gui-app`. Copy `FreeImage.dll`, `OpenImageDenoise.dll`, `raylib.dll`,
   and `tbb12.dll` from `raylib` output to `gui-app` output directory.

# Environment

**Raylib and CUI app**

* Language   : C++11
* IDE        : Visual Studio 2022
* Build tool : CMake
* OS         : Windows 10 or newer

**GUI app**

* Language   : C# 11 (.NET SDK 7)
* IDE        : Visual Studio 2022

# Sample images

Most of OBJ models are downloaded from Morgan McGuire's [Computer Graphics Archive](https://casual-effects.com/data)

Denoised images are generated by [OpenImageDenoise](https://github.com/OpenImageDenoise/oidn) integration

## Famous demo scenes

![SanMiguel](https://user-images.githubusercontent.com/11644393/214966860-0871ca68-8469-4ae1-a36f-61c1048014f7.jpg)
San Miguel

![CornellBox](https://user-images.githubusercontent.com/11644393/206691703-35e7985a-a5bc-4e74-8ed9-0ce6cf4543f4.jpg)
Cornell Box

![BreakfastRoom](https://user-images.githubusercontent.com/11644393/206691960-0d3def29-b561-4185-9815-e621bcb5c183.jpg)
Breakfast Room

![BreakfastRoomDebug](https://user-images.githubusercontent.com/11644393/206693424-017dec2d-35b8-4d21-a4fd-33d791c8fcea.jpg)
Breakfast Room, debug mode = VertexNormal

![DabrovicSponza](https://user-images.githubusercontent.com/11644393/207809338-75d54b4b-7756-4bfd-8945-539b79f3fe49.jpg)
Dabrovic Sponza

![FireplaceRoom](https://user-images.githubusercontent.com/11644393/207712336-349be0a9-54d1-4ebf-8c7b-c730eede4d8b.jpg)
Fireplace Room

![LivingRoom](https://user-images.githubusercontent.com/11644393/208139547-f662edf3-52a7-420f-9c68-85c975c22fda.jpg)
Living Room

## Random scenes

![preview3](https://user-images.githubusercontent.com/11644393/51801447-49746080-2281-11e9-9d56-2954ab4039c1.jpg)
![preview5](https://user-images.githubusercontent.com/11644393/153760159-e1e8b09c-00b9-4ca9-97c9-9e8421bbecd8.jpg)
![preview6](https://user-images.githubusercontent.com/11644393/186878829-f7ce3927-c30e-4686-abec-94df6e6a5ccb.jpg)

![guiapp](https://user-images.githubusercontent.com/11644393/210278173-a86309b6-df5d-4bed-b4ec-37719b3b44c0.jpg)
