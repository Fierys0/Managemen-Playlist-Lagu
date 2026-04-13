#include "trails.hpp"

void MouseTrails::Init() {
  Vector2 mouse = GetMousePosition();
  Vector2 scale = Fumbo::Utils::GetUIScale();
  m_cursorPos = {mouse.x / scale.x, mouse.y / scale.y};
  m_timeElapsed = 0.0f;

  float w = Fumbo::Utils::UI_WIDTH;
  float h = Fumbo::Utils::UI_HEIGHT;

  int count = 0;
  float spacing = 26.0f;
  float hexHeight = spacing * 0.866025f; // sqrt(3)/2

  for (float y = -100.0f; y < h + 100.0f; y += hexHeight) {
    bool stagger = ((int)(y / hexHeight) % 2) != 0;
    float startX = stagger ? -100.0f + spacing * 0.5f : -100.0f;

    for (float x = startX; x < w + 100.0f; x += spacing) {
      if (count >= PARTICLE_COUNT)
        break;

      m_particles[count].homePos = {x, y};
      m_particles[count].pos = {x, y};
      m_particles[count].vel = {0, 0};

      // Static spatial color gradient based on home position
      float factor = (y + x) / (w + h);
      m_particles[count].hueOffset = 240.0f + factor * 120.0f;
      m_particles[count].hueSpeed = 0.0f;
      m_particles[count].baseSize = 2.0f; // structured crisp size

      count++;
    }
  }

  // Fill remaining particles if screen was ultra-wide (safety fallback)
  for (; count < PARTICLE_COUNT; count++) {
    m_particles[count] = m_particles[0]; // just duplicate first to keep safe
  }
}

void MouseTrails::Update() {
  float dt = GetFrameTime();
  m_timeElapsed += dt;

  Vector2 mouse = GetMousePosition();
  int physW = GetScreenWidth();
  int physH = GetScreenHeight();
  bool inBounds =
      (mouse.x >= 0 && mouse.x < physW && mouse.y >= 0 && mouse.y < physH);

  if (inBounds) {
    Vector2 scale = Fumbo::Utils::GetUIScale();
    m_cursorPos = {mouse.x / scale.x, mouse.y / scale.y};
  }
  // Cursor position freezes when mouse leaves window

  for (int i = 0; i < PARTICLE_COUNT; i++) {
    CursorParticle &p = m_particles[i];

    p.hueOffset += p.hueSpeed * dt;

    // Vector from particle's home to cursor
    float dx = m_cursorPos.x - p.homePos.x;
    float dy = m_cursorPos.y - p.homePos.y;
    float dist = sqrtf(dx * dx + dy * dy);

    // Gravity pull: particles are displaced toward cursor
    // Pull is strong near cursor, decays exponentially with distance
    float pullMax = 70.0f;    // max displacement in pixels
    float pullDecay = 280.0f; // radius of influence
    float pull = pullMax * expf(-dist / pullDecay);

    // Target = home position displaced toward cursor
    Vector2 target;
    if (dist > 0.5f) {
      float invDist = 1.0f / dist;
      target.x = p.homePos.x + dx * invDist * pull;
      target.y = p.homePos.y + dy * invDist * pull;
    } else {
      target = p.homePos;
    }

    // Spring toward target position
    float springK = 8.0f;
    p.vel.x += (target.x - p.pos.x) * springK * dt;
    p.vel.y += (target.y - p.pos.y) * springK * dt;
    p.vel.x *= 0.90f; // damping
    p.vel.y *= 0.90f;
    p.pos.x += p.vel.x;
    p.pos.y += p.vel.y;
  }
}

void MouseTrails::Draw() {
  for (int i = 0; i < PARTICLE_COUNT; i++) {
    const CursorParticle &p = m_particles[i];

    // Distance from particle's current position to cursor
    float dx = p.pos.x - m_cursorPos.x;
    float dy = p.pos.y - m_cursorPos.y;
    float dist = sqrtf(dx * dx + dy * dy);

    // Blank zone around cursor based on configurable variable
    if (dist < m_blankRadius)
      continue;

    // Size morphing: massive wavelength so only one wave crest is visible
    float wavePhase = dist * 0.018f - m_timeElapsed * 0.5f;
    float currentSize = 1.3f + sinf(wavePhase) * 1.0f;

    // Alpha fade-in outside the blank radius to prevent sharp popping edge
    float fadeRadius = 45.0f;
    float alphaFactor = 1.0f;
    if (dist < m_blankRadius + fadeRadius) {
      alphaFactor = (dist - m_blankRadius) / fadeRadius;
    } else if (dist > 150.0f) {
      // Fade out much sooner so the effect itself isn't massively wide
      alphaFactor = fmaxf(0.0f, 1.0f - (dist - 150.0f) / 150.0f);
    }

    unsigned char alpha = (unsigned char)(127.5f * alphaFactor);
    if (alpha < 6)
      continue; // skip invisible particles

    // Color based on spatial gradient
    Color c = ColorFromHSV(p.hueOffset, 0.85f, 0.95f);
    c.a = alpha;

    // Motion streak when particle is moving fast
    float speed = sqrtf(p.vel.x * p.vel.x + p.vel.y * p.vel.y);
    if (speed > 1.5f && currentSize > 1.0f) {
      float trailLen = fminf(speed * 0.35f, 8.0f);
      float invSpd = 1.0f / speed;
      Vector2 dir = {p.vel.x * invSpd, p.vel.y * invSpd};
      Vector2 tail = {p.pos.x - dir.x * trailLen, p.pos.y - dir.y * trailLen};
      Fumbo::Graphic2D::DrawLineEx(tail, p.pos, currentSize * 0.7f, c);
    } else {
      Fumbo::Graphic2D::DrawCircleV(p.pos, currentSize, c);
    }
  }
}
