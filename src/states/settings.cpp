#include "settings.hpp"
#include "../core/globals.hpp"
#include "fumbo.hpp"
#include "mainMenu.hpp"
#include <string>

void Settings::Init() {
  toggleTheme = Fumbo::UI::Button({100, 100, 200, 50});
  toggleTheme.ApplyStyle(btnstyle);
  toggleTheme.AddText("Toggle Theme", SpaceB, 24, currentTheme.second1);

  backBtn = Fumbo::UI::Button({50, 600, 75, 75});
  backBtn.ApplyStyle(btnstyle);
  backBtn.AddText("<", SpaceB, 80, currentTheme.second1);
}

void Settings::Cleanup() {}

void Settings::Update() {
  if (toggleTheme.IsPressed()) {
    if (currentTheme.id == darkTheme.id)
      currentTheme = lightTheme;
    else
      currentTheme = darkTheme;

    // Refresh button text color to match theme
    toggleTheme.AddText("Toggle Theme", SpaceB, 24, currentTheme.second1);
    backBtn.AddText("<", SpaceB, 80, currentTheme.second1);

    Fumbo::Engine::Instance().InvalidateCleanLayer();
  }

  if (backBtn.IsPressed()) {
    Fumbo::Instance().ChangeState(std::make_shared<MainMenu>());
  }
}

void Settings::DrawClean() {}

void Settings::DrawDirty() {
  Fumbo::Graphic2D::DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                                  currentTheme.prim1);
  toggleTheme.Draw();
  backBtn.Draw();
  std::string themeNow = "Current Theme: " + currentTheme.name;
  Fumbo::Graphic2D::DrawText(themeNow, {320, 113}, SpaceB, 24,
                             currentTheme.second1);
  Fumbo::Graphic2D::DrawText("xxxxxx", {1200, 600}, SpaceB, 15,
                             currentTheme.second1);
}
