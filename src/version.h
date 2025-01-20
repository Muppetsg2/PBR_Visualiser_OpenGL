#pragma once

#define PBR_VISUALISER_VER_MAJOR 1
#define PBR_VISUALISER_VER_MINOR 3
#define PBR_VISUALISER_VER_PATCH 2

#define PBR_VISUALISER_LAST_UPDATE "20.01.2025"

#define PBR_VISUALISER_TO_STRING_VERSION(major, minor, patch) std::format("{}.{}.{}", major, minor, patch)
#define PBR_VISUALISER_TO_INT_VERSION(major, minor, patch) (major * 10000 + minor * 100 + patch)
#define PBR_VISUALISER_VERSION_INT PBR_VISUALISER_TO_INT_VERSION(PBR_VISUALISER_VER_MAJOR, PBR_VISUALISER_VER_MINOR, PBR_VISUALISER_VER_PATCH)
#define PBR_VISUALISER_VERSION_STR PBR_VISUALISER_TO_STRING_VERSION(PBR_VISUALISER_VER_MAJOR, PBR_VISUALISER_VER_MINOR, PBR_VISUALISER_VER_PATCH)


//   __________  ___  ____ 
//  /_  __/ __ \/ _ \/ __ \
//   / / / /_/ / // / /_/ /
//  /_/  \____/____/\____/ 
//        
//        
// NOTHING


//    _______                      __         
//   / ___/ /  ___ ____  ___ ____ / /__  ___ _
//  / /__/ _ \/ _ `/ _ \/ _ `/ -_) / _ \/ _ `/
//  \___/_//_/\_,_/_//_/\_, /\__/_/\___/\_, / 
//                     /___/           /___/  
// 
// --- 1.3.2 ---
// - deleted skybox draw function in consol build
// - deleted skybox.frag and skybox.vert from consol build
//
// --- 1.3.1 ---
// - added opengl debug info in console verbose mode
// - added opengl error debug info in window no debug build
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
// - fixed bug in Texture2D constructor
// - added deinitialization of Shape class at the end of program in console build
// - the full-screen texture drawing method was used using one triangle. This made it possible to avoid creating additional VBO and EBO
// - added validation of the submitted file name in console build
// - added Interactive mode in console build
// - added button to reset maps to initial state in window build
// - added cgltf library to window build
// - added tinyObjLoader library to window build
// - added button to load own 3d object file to visualiser in window build. Supported formats: .glb, .gltf, .obj. (.glb, .gltf and .obj must contain meshes composed of triangles and at least information about Position, Normal, TexCoords)
// - added information about model filename in window build
// - added two models (watch and lamp) in 3 formats (.obj, .glb and .glft + .bin)
// - added information about skybox file/folder name in window build
// - updated ImGui to 1.91.7-docking version
// - optimized rendering in console mode by not drawing skybox
// - added 3 diffrent display modes for skybox in window build
// - added option to change displayed mipmap level for skybox in window build
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