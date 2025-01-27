set(PRECOMPILE_HEADERS_FILE ${CMAKE_CURRENT_SOURCE_DIR}/pch.h)

include(${CMAKE_SOURCE_DIR}/cmake/pack_shaders.cmake)

generate_key(6 SHADERS_KEY)

# Add source files
file(GLOB_RECURSE SOURCE_FILES 
	 *.c
	 *.cpp)
	
# Add header files
file(GLOB_RECURSE HEADER_FILES 
	 *.h
	 *.hpp)
	 
if (NOT WINDOW_APP)
	list(FILTER SOURCE_FILES EXCLUDE REGEX ".*(imgui_impl\/|TimeManager\.cpp|ModelLoader\.cpp).*")
	list(FILTER HEADER_FILES EXCLUDE REGEX ".*(imgui_impl\/|TimeManager\.h|ModelLoader\.h).*")
endif()

# Search for the assets files
file(GLOB_RECURSE ASSETS_FILES 
	 ${CMAKE_SOURCE_DIR}/res/*.*)

set_source_files_properties(${ASSETS_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})						   
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADER_FILES})
source_group(TREE ${CMAKE_SOURCE_DIR}         FILES ${ASSETS_FILES})

# Define the executable
add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES} ${ASSETS_FILES})

if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PUBLIC NOMINMAX)
    target_compile_options(${PROJECT_NAME} PUBLIC "/Zc:preprocessor")
	configure_file(${CMAKE_SOURCE_DIR}/res/build_resources/msvcp140.dll ${CMAKE_CURRENT_BINARY_DIR}/msvcp140.dll COPYONLY)
	configure_file(${CMAKE_SOURCE_DIR}/res/build_resources/vcruntime140.dll ${CMAKE_CURRENT_BINARY_DIR}/vcruntime140.dll COPYONLY)
	configure_file(${CMAKE_SOURCE_DIR}/res/build_resources/vcruntime140_1.dll ${CMAKE_CURRENT_BINARY_DIR}/vcruntime140_1.dll COPYONLY)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE GLFW_INCLUDE_NONE)
target_compile_definitions(${PROJECT_NAME} PRIVATE LIBRARY_SUFFIX="")
target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG="$<IF:$<CONFIG:Debug>,1,0>")
target_compile_definitions(${PROJECT_NAME} PRIVATE S_KEY="${SHADERS_KEY}")

if(WINDOW_APP)
	target_compile_definitions(${PROJECT_NAME} PRIVATE WINDOW_APP)
endif()

target_precompile_headers(${PROJECT_NAME} PUBLIC ${PRECOMPILE_HEADERS_FILE})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
												  ${glad_SOURCE_DIR}
												  ${stb_image_SOURCE_DIR}
												  ${stb_image_write_SOURCE_DIR})

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PUBLIC "-O2")
endif()

target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})
target_link_libraries(${PROJECT_NAME} PUBLIC glad
											 stb_image
											 stb_image_write
											 glfw
											 gli
											 spdlog
											 glm::glm)

if(WINDOW_APP)
	 target_include_directories(${PROJECT_NAME} PUBLIC ${imgui_SOURCE_DIR}
													   ${cgltf_SOURCE_DIR}
													   ${tiny_obj_loader_SOURCE_DIR})
	 target_link_libraries(${PROJECT_NAME} PUBLIC imgui
												  cgltf
												  tiny_obj_loader)
endif()

# Shader Packing
file(GLOB SHADER_FILES ${CMAKE_SOURCE_DIR}/res/shader/*)

if (WINDOW_APP)
	list(FILTER SHADER_FILES EXCLUDE REGEX ".*(basic2\.vert|basic2\.frag).*")
else()
	list(FILTER SHADER_FILES EXCLUDE REGEX ".*(basic\.vert|basic\.frag|screen\.vert|screen\.frag|skybox\.vert|skybox\.frag).*")
endif()

string(REPLACE ";" " " SHADER_FILES_STR "${SHADER_FILES}")

configure_file(${CMAKE_SOURCE_DIR}/res/build_resources/null.txt ${CMAKE_CURRENT_BINARY_DIR}/null.txt COPYONLY)

# Create a separate script to invoke the function
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/cmake_post_build.cmake 
"cmake_policy(SET CMP0011 NEW)\n
include(${CMAKE_SOURCE_DIR}/cmake/pack_shaders.cmake)\n
set(null_path ${CMAKE_CURRENT_BINARY_DIR}/null.txt CACHE INTERNAL \"Path to null character for shaders\")\n
string(REPLACE \" \" \";\" SHADER_LIST \"\${arg3}\")\n
pack_shaders(\"\${arg1}\" \"\${arg2}\" \"\${SHADER_LIST}\")\n"
)

# Function packing shaders to one binary file
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Packing Shaders Started..."
    COMMAND ${CMAKE_COMMAND} -E remove -f "${CMAKE_CURRENT_BINARY_DIR}/shaders.dat" # Delete old file
    COMMAND ${CMAKE_COMMAND} -E touch "${CMAKE_CURRENT_BINARY_DIR}/shaders.dat" # Create new file
    COMMAND ${CMAKE_COMMAND} -Darg1="${SHADERS_KEY}" -Darg2="${CMAKE_CURRENT_BINARY_DIR}/shaders.dat" -Darg3="${SHADER_FILES_STR}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_post_build.cmake"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
				   COMMAND ${CMAKE_COMMAND} -E create_symlink 
				   ${CMAKE_SOURCE_DIR}/res 
				   ${CMAKE_CURRENT_BINARY_DIR}/res)