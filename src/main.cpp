#include "core/core.hpp"
#include "fumbo.hpp"
#include "raylib.h"
#include "states/mainMenu.hpp"
#include <memory>

int main() {

  Fumbo::Engine::Instance().Init(1280, 720, "Magfy", 60);
  ToggleBorderlessWindowed();
  auto appCore = std::make_shared<AppCore>();
  Fumbo::Engine::Instance().SharedState(appCore);
  Fumbo::Engine::Instance().Run(std::make_shared<MainMenu>());
  Fumbo::Engine::Instance().Quit();

  return 0;
}
