#include "mainMenu.hpp"
#include "fumbo.hpp"

void MainMenu::Init() {}
void MainMenu::Cleanup() {}
void MainMenu::Update() {}
void MainMenu::DrawClean() { ClearBackground({16, 18, 18, 255}); }
void MainMenu::DrawDirty() {
  Fumbo::Graphic2D::DrawText("Hello World!", {100, 100}, {}, 124, WHITE);
}
