#pragma once
#include "fumbo.hpp"
#include "raylib.h"
#include "trails.hpp"

class AppCore : public IGameState {
public:
  void Init() override;
  void Cleanup() override;
  void Update() override;
  void DrawClean() override;
  void DrawDirty() override;

private:
  Vector2 m_dragOffset = {0, 0};
  Vector2 m_windowPosCache = {0, 0};
  Fumbo::UI::Button exitBtn;
  Fumbo::UI::Button maximizeBtn;
  Fumbo::UI::Button minimizeBtn;
  Fumbo::UI::Button titleBar;
  bool m_isDragging = false;
  MouseTrails mouseTrails;
};
