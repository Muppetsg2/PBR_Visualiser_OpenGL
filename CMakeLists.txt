cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(global_settings)
include(utils)

pbr_visualiser_extract_version()

project(PBR_Visualiser VERSION ${PBR_VISUALISER_VERSION})
message(STATUS "Build PBR_Visualiser: ${PBR_VISUALISER_VERSION}")

IF(WIN32)
  message(NOTICE "\n")
  message(NOTICE "********************** IMPORTANT NOTICE ****************************\n")
  message(NOTICE "Make sure you have enabled Developer Mode in your system settings!")
  message(NOTICE "To enable Developer Mode go to Settings -> Update & Security -> For developers -> Developer Mode ON")
  message(NOTICE "\n********************************************************************")
  message(NOTICE "\n")
ENDIF()

set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/bin/${CMAKE_CFG_INTDIR})

# ---- Dependencies ----
add_subdirectory(thirdparty)

# ---- Main project's files ----
add_subdirectory(src)

if(WIN32)
	set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${OUTPUT_DIR}")
endif()