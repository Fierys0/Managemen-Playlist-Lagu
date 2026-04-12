# Managemen Playlist Lagu

Program untuk memenuhi Ujian Akhir Semester Struktur Data

![last-commit](https://img.shields.io/github/last-commit/Fierys0/Managemen-Playlist-Lagu)
![repo-size](https://img.shields.io/github/repo-size/Fierys0/Managemen-Playlist-Lagu)
![license](https://img.shields.io/github/license/Fierys0/Managemen-Playlist-Lagu)

Cloning project

```
git clone --recursive https://github.com/Fierys0/Managemen-Playlist-Lagu
```

---

## Features

- Create your own Playlist(s)
- UI Based

---

## Build Requirements

### All Platforms

- **CMake ≥ 3.16**
- **C++17 compatible compiler**
  - GCC / Clang (Linux, macOS)
  - MinGW-w64 or MSVC (Windows)

### Linux

Install raylib (system version recommended):

**Pacman**

```bash
sudo pacman -S raylib
```

**APT**

```bash
sudo apt install libraylib-dev
```

Install mpv

**Pacman**

```bash
sudo pacman -S mpv
```

**APT**

```bash
sudo apt install mpv
```

### Windows

Compile using cmake only, no prerequisite

## Building the Project

```bash
mkdir build
cmake -G "Ninja" -S . -B build
cmake --build build
```

---

## System Requirements

- OpenGL >= 3.3
