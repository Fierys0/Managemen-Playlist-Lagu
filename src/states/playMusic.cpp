#include "playMusic.hpp"
#include "../core/globals.hpp"
#include "fumbo.hpp"
#include "raylib.h"

void playMusic::Init() {
  prevBtn = Fumbo::UI::Button({450, 500, 100, 100});
  nextBtn = Fumbo::UI::Button({600, 500, 100, 100});
  statusBtn = Fumbo::UI::Button({750, 500, 100, 100});
  prevBtn.ApplyStyle(btnstyle);
  nextBtn.ApplyStyle(btnstyle);
  statusBtn.ApplyStyle(btnstyle);
}
void playMusic::Update() {}
void playMusic::DrawDirty() {
  Fumbo::Graphic2D::DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                                  currentTheme.prim1);
  prevBtn.Draw();
  nextBtn.Draw();
  statusBtn.Draw();
  Fumbo::Graphic2D::DrawText("Previous Song < Current Song > Next Song",
                             {350, 450}, SpaceB, 32, currentTheme.second1);
}
void playMusic::Cleanup() {}
void playMusic::DrawClean() {}
