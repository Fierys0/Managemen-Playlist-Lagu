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

class MouseTrails {
public:
  void Init();
  void Update();
  void Draw();

  void SetEnabled(bool enabled) { m_enabled = enabled; }
  bool IsEnabled() const { return m_enabled; }

private:
  static constexpr int PARTICLE_COUNT = 3000;
  CursorParticle m_particles[PARTICLE_COUNT];
  Vector2 m_cursorPos = {640.0f, 360.0f};
  float m_timeElapsed = 0.0f;
  float m_blankRadius = 25.0f;
  bool m_enabled = true;
};
