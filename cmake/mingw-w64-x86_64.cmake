# cmake/mingw-w64-x86_64.cmake
# Cross-compilation toolchain: Linux host → Windows x86-64 target via MinGW-w64

set(CMAKE_SYSTEM_NAME    Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

# ── Compiler & tools ──────────────────────────────────────────────────────────
set(MINGW_PREFIX "x86_64-w64-mingw32")

find_program(CMAKE_C_COMPILER   NAMES ${MINGW_PREFIX}-gcc   /usr/sbin/${MINGW_PREFIX}-gcc   REQUIRED)
find_program(CMAKE_CXX_COMPILER NAMES ${MINGW_PREFIX}-g++   /usr/sbin/${MINGW_PREFIX}-g++   REQUIRED)
find_program(CMAKE_RC_COMPILER  NAMES ${MINGW_PREFIX}-windres /usr/sbin/${MINGW_PREFIX}-windres)
find_program(CMAKE_AR           NAMES ${MINGW_PREFIX}-ar    /usr/sbin/${MINGW_PREFIX}-ar    REQUIRED)
find_program(CMAKE_RANLIB       NAMES ${MINGW_PREFIX}-ranlib /usr/sbin/${MINGW_PREFIX}-ranlib REQUIRED)
find_program(CMAKE_STRIP        NAMES ${MINGW_PREFIX}-strip  /usr/sbin/${MINGW_PREFIX}-strip)

# ── Sysroot search settings ───────────────────────────────────────────────────
set(CMAKE_FIND_ROOT_PATH
    /usr/${MINGW_PREFIX}
    /usr/lib/gcc/${MINGW_PREFIX}
)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)   # host programs
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)    # target libraries
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)    # target headers
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# ── Windows version target ────────────────────────────────────────────────────
# Target Windows 7+ (0x0601) — adjust if needed
add_compile_definitions(_WIN32_WINNT=0x0601 WINVER=0x0601)
