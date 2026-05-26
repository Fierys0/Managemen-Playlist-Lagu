#include "mainMenu.hpp"
#include "addPlaylist.hpp"
#include "../core/globals.hpp"
#include "fumbo.hpp"
#include "raylib.h"
#include "settings.hpp"
#include <memory>

void MainMenu::Init()
{
  settingTex = Fumbo::Assets::LoadTexture("assets/images/settings.png");
  settingBtn = Fumbo::UI::Button({50, 600, 75, 75});
  settingBtn.SetTexture(settingTex);

  plusBtn = Fumbo::UI::Button({640, 360, 75, 75});
  plusBtn.ApplyStyle(btnstyle);
  plusBtn.AddText("+", SpaceB, 80, currentTheme.second1);
}
void MainMenu::Cleanup() { UnloadTexture(settingTex); }
void MainMenu::Update()
{
  if (settingBtn.IsPressed())
  {
    Fumbo::Instance().ChangeState(std::make_shared<Settings>());
  }

  if (plusBtn.IsPressed())
    Fumbo::Instance().ChangeState(std::make_shared<AddPlaylist>());
}
void MainMenu::DrawClean() {}
void MainMenu::DrawDirty()
{
  Fumbo::Graphic2D::DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                                  currentTheme.prim1);
  Fumbo::Graphic2D::DrawText("Hello World!", {100, 100}, SpaceB, 124,
                             currentTheme.second1);
  settingBtn.Draw();
  plusBtn.Draw();
}
