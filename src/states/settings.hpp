#pragma once
#include "fumbo.hpp"

class Settings : public IGameState {
public:
  void Init() override;
  void Cleanup() override;
  void Update() override;
  void DrawClean() override;
  void DrawDirty() override;

private:
  Fumbo::UI::Button toggleTheme;
  Fumbo::UI::Button backBtn;
};
