#include "core/core.hpp"
#include "core/globals.hpp"
#include "fumbo.hpp"
#include "raylib.h"
#include "states/mainMenu.hpp"
#include <memory>

int main() {

  Fumbo::Engine::Instance().Init(1280, 720, "Magfy", 60);
  SetConfigFlags(FLAG_WINDOW_UNDECORATED);
  auto appCore = std::make_shared<AppCore>();
  LoadGlobalAssets();
  Fumbo::Engine::Instance().SharedState(appCore);
  Fumbo::Engine::Instance().Run(std::make_shared<MainMenu>());
  UnloadGlobalAssets();
  Fumbo::Engine::Instance().Quit();

  return 0;
}
