#pragma once
#include "fumbo.hpp"
#include <string>

extern Font SpaceB;

// [ALPRO] [STRUCT]
// Menggunakan struct ColorTheme untuk definisi skema warna UI
typedef struct colortheme
{
  int id;
  std::string name{};
  Color prim1{};
  Color prim2{};
  Color second1{};
  Color second2{};
  Color titlebar{};
} ColorTheme;

extern Fumbo::UI::ButtonStyle btnstyle;
extern Fumbo::UI::ButtonStyle rectangleBtnStyle;

extern ColorTheme currentTheme;
extern ColorTheme darkTheme;
extern ColorTheme lightTheme;

void LoadGlobalAssets();
void UnloadGlobalAssets();
