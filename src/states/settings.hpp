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
  Fumbo::UI::Button m_toggleThemeBtn;
  Fumbo::UI::Button m_toggleTrailsBtn;
  Fumbo::UI::Button m_toggleLangBtn;
  Fumbo::UI::Button m_backBtn;
};
