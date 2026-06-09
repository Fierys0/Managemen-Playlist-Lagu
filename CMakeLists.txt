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
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Enable Position Independent Code for all targets (required for shared libraries)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ── libvlc ────────────────────────────────────────────────────────────────
# On Linux use the system libvlc via pkg-config.
# On Windows use the bundled VLC SDK located at src/lib/windows/vlc/sdk/.
if(WIN32)
  set(VLC_WIN_SDK "${CMAKE_SOURCE_DIR}/src/lib/windows/vlc/sdk")
  set(LIBVLC_INCLUDE_DIRS "${VLC_WIN_SDK}/include")
  set(LIBVLC_LIBRARIES    "${VLC_WIN_SDK}/lib/libvlc.lib"
                           "${VLC_WIN_SDK}/lib/libvlccore.lib")
  set(LIBVLC_CFLAGS_OTHER "")
  message(STATUS "Using bundled Windows VLC SDK: ${VLC_WIN_SDK}")
else()
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(LIBVLC REQUIRED libvlc)
endif()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build Raylib as a static library" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
if(LINUX OR WIN32)
  add_subdirectory(engine/lib/raylib)
endif()
add_subdirectory(engine)

set_target_properties(fumbo_engine PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/magfy
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/magfy
)

add_executable(asset_pack EXCLUDE_FROM_ALL engine/tools/pack_assets.cpp)
target_include_directories(asset_pack PRIVATE
    engine
    engine/tools
    engine/lib/raylib/src
    src
)
set_target_properties(asset_pack PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tools
)

set(ASSET_PACK_OUTPUT ${CMAKE_BINARY_DIR}/magfy/data.fpk)
set(ASSET_SRC_DIR ${CMAKE_SOURCE_DIR}/assets)

file(GLOB_RECURSE ASSET_SOURCES "${ASSET_SRC_DIR}/*")

add_custom_command(
    OUTPUT ${ASSET_PACK_OUTPUT}
    COMMAND $<TARGET_FILE:asset_pack> ${ASSET_SRC_DIR} ${ASSET_PACK_OUTPUT}
    DEPENDS asset_pack ${ASSET_SOURCES}
    COMMENT "Packing assets into data.fpk..."
    VERBATIM
)

add_custom_target(pack_assets DEPENDS ${ASSET_PACK_OUTPUT})

# ── Game sources ───────────────────────────────────────────────────────────
file(GLOB_RECURSE GAME_SOURCES
    src/core/*.cpp
    src/states/*.cpp
    src/main.cpp
)

add_executable(${PROJECT_NAME} ${GAME_SOURCES})

add_dependencies(${PROJECT_NAME} pack_assets)

set_target_properties(${PROJECT_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/magfy
)

if(UNIX AND NOT APPLE)
  set_target_properties(${PROJECT_NAME} PROPERTIES
        BUILD_RPATH "$ORIGIN"
        INSTALL_RPATH "$ORIGIN"
    )
endif()

target_include_directories(${PROJECT_NAME}
    PRIVATE
        src
        src/lib
        src/lib/vlcpp
        ${LIBVLC_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        fumbo_engine
        ${LIBVLC_LIBRARIES}
)

target_compile_options(${PROJECT_NAME}
    PRIVATE
        ${LIBVLC_CFLAGS_OTHER}
)

if(WIN32)
  target_link_libraries(${PROJECT_NAME}
        PRIVATE winmm
    )

  # ── Copy VLC DLLs next to the executable after build ──────────────────
  # The DLLs are NOT part of the SDK archive; copy them from your VLC
  # installation (e.g. C:/Program Files/VideoLAN/VLC/) into:
  #   src/lib/windows/vlc/
  # so that the two files below exist.
  set(VLC_DLL_DIR "${CMAKE_SOURCE_DIR}/src/lib/windows/vlc")
  foreach(_dll libvlc.dll libvlccore.dll)
    if(EXISTS "${VLC_DLL_DIR}/${_dll}")
      add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${VLC_DLL_DIR}/${_dll}"
          "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${_dll}"
        COMMENT "Copying ${_dll} to output directory"
      )
    else()
      message(WARNING
        "VLC DLL not found: ${VLC_DLL_DIR}/${_dll}\n"
        "Copy libvlc.dll and libvlccore.dll from your VLC install into "
        "src/lib/windows/vlc/ for the app to run on Windows.")
    endif()
  endforeach()
endif()
