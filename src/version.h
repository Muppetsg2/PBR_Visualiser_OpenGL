#pragma once

#define PBR_VISUALISER_VER_MAJOR 1
#define PBR_VISUALISER_VER_MINOR 3
#define PBR_VISUALISER_VER_PATCH 8

#define PBR_VISUALISER_LAST_UPDATE "29.03.2025"

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
// --- 1.3.8 ---
// - added fbx 3d model loading using OpenFBX library in window build
// - fixed behaviour when loading 3d model is canceled in window build
// - added sample fbx and obj models to repository
// 
// --- 1.3.6 ---
// - added popup when error occure while loading faces of skybox in window build
// - added popup when error occure when creating skybox from faces in window build
// - updated tinyfiledialogs in window build
// - added freetype for handling fonts in window build
// - add popup when error occure while loading skybox from hdr or data in window build
// - add popup when error occure while loading 3d model in window build
// - added popup when model is succesfully loaded in window build
// - added popup when error occure while loading textures in window build
// - changed CMake adding thirdparty libraries
// 
// --- 1.3.5 ---
// - added MSAA to window build
// - added popup when screenshot is saved in window build
// - added new skybox in console build. Name: golden_bay
// - repaired text in help message in console build
// - added Argument Parser in console build
// - fixed behaviour that occured when was passed map with less channels to slot for map with more channels
// - fixed errors while changing skybox in window build
// - fixed cleaning faces when closing window to load faces as skybox in window build
// - changed file dialogs from ImGui FileDialogs to tinyfiledialogs in window build
// - changed names of Park skybox faces
// - added checking whether a window is open in imgui window drawing functions in window build
// - python file has been added that prepares applications for public release
// - added information on how to move around the scene in window build
// - added README.md
// - added Icon
// - updated file with examples
// - changed imgui look in window build
// - fixed bug with not applying filter on screenshot in window build
//
// --- 1.3.2 ---
// - deleted skybox draw function in console build
// - deleted skybox.frag and skybox.vert from console build
// - fixed bug with changing skybox texture in console build
// - updated spdlog to 1.15.1
// - updated ImGui to 1.91.8-docking version
// - updated cgltf
// - updated tiny_obj_loader
// - deleted chrono lib from console build and iostream lib from window build
// - fixed attribute naming in Config class and TimeManager class
// - fixed function naming in Config class
// - added to shader static method to create shader from file
// - deleted constructor in static classes
// - added filters for camera output (pixeling image, negative, grayscale)
// - deleted _hdr bool from Skybox in console build
// - changed _paths[6] to _path in console build
// - added option to close app using ImGui
// - added information in the menu bar about whether a given window is open
// - added checking whether the models and textures folder exists when opening a dialog box in window build
// - fixed bug with saving screenshot to already created folder in window build
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