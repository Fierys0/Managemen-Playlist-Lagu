#pragma once
#include "fumbo.hpp"

struct CursorParticle {
  Vector2 homePos;
  Vector2 pos;
  Vector2 vel;
  float hueOffset;
  float hueSpeed;
  float baseSize;
};

class AppCore : public IGameState {
public:
  void Init() override;
  void Cleanup() override;
  void Update() override;
  void DrawClean() override;
  void DrawDirty() override;

private:
  static constexpr int PARTICLE_COUNT = 3000;
  CursorParticle m_particles[PARTICLE_COUNT];
  Vector2 m_cursorPos = {640.0f, 360.0f};
  float m_timeElapsed = 0.0f;
  float m_blankRadius = 25.0f;
  Fumbo::UI::Button exitBtn;
  Fumbo::UI::Button maximizeBtn;
  Fumbo::UI::Button minimizeBtn;
};
