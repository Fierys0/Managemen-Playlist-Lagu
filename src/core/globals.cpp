#include "globals.hpp"
#include "fumbo.hpp"

Font SpaceB{};
Fumbo::UI::ButtonStyle btnstyle{};
ColorTheme darkTheme;
ColorTheme lightTheme;
ColorTheme currentTheme;

void LoadGlobalAssets()
{
    SpaceB = Fumbo::Assets::LoadFont(
        "assets/fonts/Space_Grotesk/SpaceGrotesk-Bold.ttf", 124);
    darkTheme = {
        0, "Default Dark", {16, 18, 18, 255}, {}, {220, 220, 220, 127}, {}, {250, 249, 246, 127}};
    lightTheme = {
        1, "Default Light", {250, 249, 246, 255}, {}, {50, 50, 50, 255}, {}, {16, 16, 18, 127}};
    currentTheme = darkTheme;
    btnstyle = {SpaceB, 24, {200, 200, 200, 255}, WHITE, GRAY, {}, 1, 12};
}

void UnloadGlobalAssets() { UnloadFont(SpaceB); }
