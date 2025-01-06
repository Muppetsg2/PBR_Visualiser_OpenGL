#pragma once

#define PBR_VISUALISER_VER_MAJOR 1
#define PBR_VISUALISER_VER_MINOR 3
#define PBR_VISUALISER_VER_PATCH 0

#define PBR_VISUALISER_LAST_UPDATE "07.01.2025"

#define PBR_VISUALISER_TO_STRING_VERSION(major, minor, patch) std::format("{}.{}.{}", major, minor, patch)
#define PBR_VISUALISER_TO_INT_VERSION(major, minor, patch) (major * 10000 + minor * 100 + patch)
#define PBR_VISUALISER_VERSION_INT PBR_VISUALISER_TO_INT_VERSION(PBR_VISUALISER_VER_MAJOR, PBR_VISUALISER_VER_MINOR, PBR_VISUALISER_VER_PATCH)
#define PBR_VISUALISER_VERSION_STR PBR_VISUALISER_TO_STRING_VERSION(PBR_VISUALISER_VER_MAJOR, PBR_VISUALISER_VER_MINOR, PBR_VISUALISER_VER_PATCH)


//    _______                      __         
//   / ___/ /  ___ ____  ___ ____ / /__  ___ _
//  / /__/ _ \/ _ `/ _ \/ _ `/ -_) / _ \/ _ `/
//  \___/_//_/\_,_/_//_/\_, /\__/_/\___/\_, / 
//                     /___/           /___/  
//
// --- 1.3.0 ---
// - changed build configurations naming
// - changed compilation target definitions for build configurations
// - added dumping shaders to one file and encrypting it in CMake (uses openssl for generating key)
// - added ShadersExtractor class for retriving information about encyrpted shaders
// - added Config class which is used to store configuration values. Now is only used to help determine which informations should be printed on console.
// - added constructors for Shader class with geometry shader
// - added static function to Shader class for creating shaders using ShadersExtractor
// - updated the Skybox class to remove the variable shadersPath
// - added screenshot saving to Window build configurations
// - added screen shaders for camera plane rendering
//
// --- 1.2.0 ---
// - added exposure and colorIntensity in Skybox and PBR shader
// - added PlaneOrientation dropdown in Debug ImGui Window
// - fixed a bug with reverse reflection when generating images
// - updated CPM and ImGui
// - updated help and added banner in consol output
// - added two project build configurations
// - parsing passed arguments when starting program in Release configuration
// - fixed bug with left, right sides in position argument
// - reduced the amount of vertex data passed to render the image in the Release configuration
// - added filename definition argument in Release configuration
// - added default black texture
// - added resolution argument in Release configuration
// - fixed bug where program couldn't find resources folder when started from diffrent location
// - added ImGui window for Skybox class
// - added loading diffrent skyboxes when running program in Debug configuration
// - added proper versioning system
// - added small txt file with proposed ways of calling the program
//
// -- 1.0.0 --
// - skybox added
// - PBR shader works
// - static Camera class created
// - added support for dds files
// - Texture2D now have hdr files support
// - added default white texture
// - added 3 shapes definitions (quad, cube, sphere)