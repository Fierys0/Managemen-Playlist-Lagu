cmake_minimum_required(VERSION 3.16)

project(Magfy VERSION 0.1 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(default_build_type "Release")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING
      "Choose the type of build." FORCE)

  # Set the possible values for cmake-gui drop-down menus
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()



# Enable Position Independent Code for all targets (required for shared libraries)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Raylib - build as static library to combine with fumbo
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build Raylib as a static library" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
if(LINUX)
  add_subdirectory(engine/lib/raylib)
endif()
add_subdirectory(engine)

# --- Asset Packing ---

# Set fumbo_engine library output directory
set_target_properties(fumbo_engine PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/magfy
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/magfy
)

# Compile the asset packager tool from the engine
add_executable(asset_pack EXCLUDE_FROM_ALL engine/tools/pack_assets.cpp)
target_include_directories(asset_pack PRIVATE
    engine
    engine/tools
    engine/lib/raylib/src
    src
)
# Place the tool in a tools directory within build
set_target_properties(asset_pack PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tools
)

# Define the assets output path (near the binary)
set(ASSET_PACK_OUTPUT ${CMAKE_BINARY_DIR}/magfy/data.fpk)
set(ASSET_SRC_DIR ${CMAKE_SOURCE_DIR}/assets)

# Gather all asset files to ensure repack on change
file(GLOB_RECURSE ASSET_SOURCES "${ASSET_SRC_DIR}/*")

# Command to perform the packing
add_custom_command(
    OUTPUT ${ASSET_PACK_OUTPUT}
    COMMAND $<TARGET_FILE:asset_pack> ${ASSET_SRC_DIR} ${ASSET_PACK_OUTPUT}
    DEPENDS asset_pack ${ASSET_SOURCES}
    COMMENT "Packing assets into data.fpk..."
    VERBATIM
)

add_custom_target(pack_assets DEPENDS ${ASSET_PACK_OUTPUT})

file(GLOB_RECURSE GAME_SOURCES src/core/*.cpp src/states/*.cpp src/main.cpp)

add_executable(${PROJECT_NAME} ${GAME_SOURCES})

# Ensure assets are packed before the game executable is built
add_dependencies(${PROJECT_NAME} pack_assets)

# Put executable in "magfy" folder
set_target_properties(${PROJECT_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/magfy
)

# Set RPATH so executable can find fumbo shared library
if(UNIX AND NOT APPLE)
  set_target_properties(${PROJECT_NAME} PROPERTIES
        BUILD_RPATH "$ORIGIN"
        INSTALL_RPATH "$ORIGIN"
    )
endif()

target_include_directories(${PROJECT_NAME}
    PRIVATE
        src
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        fumbo_engine
)

if(WIN32)
  target_link_libraries(${PROJECT_NAME}
        PRIVATE winmm
    )
endif()

