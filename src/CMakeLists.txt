set(PRECOMPILE_HEADERS_FILE ${CMAKE_CURRENT_SOURCE_DIR}/pch.h)

# Add source files
file(GLOB_RECURSE SOURCE_FILES 
	 *.c
	 *.cpp)
	
# Add header files
file(GLOB_RECURSE HEADER_FILES 
	 *.h
	 *.hpp)
	 
if (CMAKE_BUILD_TYPE STREQUAL "Release")
	list(FILTER SOURCE_FILES EXCLUDE REGEX ".*(imgui_impl\/).*")
	list(FILTER HEADER_FILES EXCLUDE REGEX ".*(imgui_impl\/).*")
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
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE GLFW_INCLUDE_NONE)
target_compile_definitions(${PROJECT_NAME} PRIVATE LIBRARY_SUFFIX="")
target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG="$<IF:$<CONFIG:Debug>,1,0>")

if(CONSOLE_ENABLED)
	target_compile_definitions(${PROJECT_NAME} PRIVATE CONSOLE_ENABLED)
endif()

target_precompile_headers(${PROJECT_NAME} PUBLIC ${PRECOMPILE_HEADERS_FILE})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
												  ${glad_SOURCE_DIR}
												  ${stb_image_SOURCE_DIR}
												  ${stb_image_write_SOURCE_DIR})

if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
	 target_link_libraries(${PROJECT_NAME} PUBLIC imgui)
	 target_include_directories(${PROJECT_NAME} PUBLIC ${imgui_SOURCE_DIR})
else()
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

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
				   COMMAND ${CMAKE_COMMAND} -E create_symlink 
				   ${CMAKE_SOURCE_DIR}/res 
				   ${CMAKE_CURRENT_BINARY_DIR}/res)