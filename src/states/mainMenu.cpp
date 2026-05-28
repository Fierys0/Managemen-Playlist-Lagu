#include "mainMenu.hpp"
#include "../core/globals.hpp"
#include "addPlaylist.hpp"
#include "fumbo.hpp"
#include "playMusic.hpp"
#include "raylib.h"
#include "settings.hpp"
#include <memory>

void MainMenu::Init() {
  settingTex = Fumbo::Assets::LoadTexture("assets/images/settings.png");
  settingBtn = Fumbo::UI::Button({50, 600, 75, 75});
  settingBtn.SetTexture(settingTex);

  
  plusBtn = Fumbo::UI::Button({603, 600, 75, 75});
  plusBtn.ApplyStyle(btnstyle);
  plusBtn.AddText("+", SpaceB, 80, currentTheme.second1);

  Texture2D img1 = Fumbo::Assets::LoadTexture("assets/images/Cover.png");
  playlistCards.emplace_back(Rectangle{460, 160, 360, 424}, img1, "My Playlist");
  
}
void MainMenu::Cleanup() { UnloadTexture(settingTex); }
void MainMenu::Update() {
  if (settingBtn.IsPressed()) {
    Fumbo::Instance().ChangeState(std::make_shared<Settings>());
  }

  if (plusBtn.IsPressed())
    Fumbo::Instance().ChangeState(std::make_shared<AddPlaylist>());
    for (auto& card : playlistCards) {
    card.Update();
    if (card.IsClicked()) {
        // Play this playlist
    }
}
}
void MainMenu::DrawClean() {} 
void MainMenu::DrawDirty() {
  Fumbo::Graphic2D::DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                                  currentTheme.prim1);
  Fumbo::Graphic2D::DrawText("Hello World!", {50000, 100}, SpaceB, 124,
                             currentTheme.second1);                         
  settingBtn.Draw();
  plusBtn.Draw();
  for (auto& card : playlistCards) {
    card.Draw();
}
}
