#pragma once
#include "fumbo.hpp"
#include "raylib.h"

class playMusic : public IGameState {
public:
  void Init() override;
  void Cleanup() override;
  void Update() override;
  void DrawClean() override;
  void DrawDirty() override;

private:
  Fumbo::UI::Button statusBtn;
  Fumbo::UI::Button nextBtn;
  Fumbo::UI::Button prevBtn;
  Texture2D nextTex;
  Texture2D prevTex;
  Texture2D statusTex;
};
