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
add_subdirectory(engine/lib/raylib)
add_subdirectory(engine)

# Set fumbo_engine library output directory
set_target_properties(fumbo_engine PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/magfy
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/magfy
)

file(GLOB_RECURSE GAME_SOURCES src/core/*.cpp src/states/*.cpp src/main.cpp)

add_executable(${PROJECT_NAME} ${GAME_SOURCES})

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
  # Copy mpv DLL next to executable
  # We still need to know where it is.
  set(MPV_ROOT ${CMAKE_SOURCE_DIR}/engine/lib/windows/mpv)

  target_include_directories(${PROJECT_NAME}
        PRIVATE
            ${MPV_ROOT}/include
    )

  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${MPV_ROOT}/libmpv-2.dll
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()

if(WIN32)
  target_link_libraries(${PROJECT_NAME}
        PRIVATE winmm
    )
endif()

