#pragma once
#include "fumbo.hpp"

class MainMenu : public IGameState {
public:
  void Init() override;
  void Cleanup() override;
  void Update() override;
  void DrawClean() override;
  void DrawDirty() override;

private:
  Fumbo::UI::Button settingBtn;
  Fumbo::UI::Button plusBtn;
  Texture2D settingTex;
};
