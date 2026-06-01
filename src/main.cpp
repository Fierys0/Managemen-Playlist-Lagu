#include "core/core.hpp"
#include "core/globals.hpp"
#include "core/vlcMetadata.hpp"
#include "fumbo.hpp"

#include "states/mainMenu.hpp"
#include <memory>

int main() {
  SetConfigFlags(FLAG_WINDOW_UNDECORATED);
  Fumbo::Engine::Instance().Init(1280, 720, "Magfy", 60);
  Fumbo::Assets::AddAssetPack("data.fpk");

  auto appCore = std::make_shared<AppCore>();
  LoadGlobalAssets();
  VlcMeta::Init();

  Fumbo::Engine::Instance().SharedState(appCore);
  Fumbo::Engine::Instance().Run(std::make_shared<MainMenu>());

  VlcMeta::Shutdown();
  UnloadGlobalAssets();
  Fumbo::Engine::Instance().Quit();

  return 0;
}
