#pragma once
#include "fumbo.hpp"
#include <string>

extern Font SpaceB;

typedef struct colortheme {
  int id;
  std::string name{};
  Color prim1{};
  Color prim2{};
  Color second1{};
  Color second2{};
  Color titlebar{};
} ColorTheme;

extern Fumbo::UI::ButtonStyle btnstyle;

extern ColorTheme currentTheme;
extern ColorTheme darkTheme;
extern ColorTheme lightTheme;

void LoadGlobalAssets();
void UnloadGlobalAssets();
