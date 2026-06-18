#include "trails.hpp"

void MouseTrails::Init() {
  Vector2 mouse = GetMousePosition();
  Vector2 scale = Fumbo::Utils::GetUIScale();
  m_cursorPos = {mouse.x / scale.x, mouse.y / scale.y};
  m_timeElapsed = 0;

  float w = Fumbo::Utils::UI_WIDTH;
  float h = Fumbo::Utils::UI_HEIGHT;

  int count = 0;
  float spacing = 26;
  float hexHeight = spacing * 0.866025; // tinggi heksagon: sqrt(3)/2

  for (float y = -100; y < h + 100; y += hexHeight) {
    bool stagger = ((int)(y / hexHeight) % 2) != 0;
    float startX = stagger ? -100 + spacing * 0.5 : -100;

    for (float x = startX; x < w + 100; x += spacing) {
      if (count >= PARTICLE_COUNT)
        break;

      m_particles[count].homePos = {x, y};
      m_particles[count].pos = {x, y};
      m_particles[count].vel = {0, 0};

      // Gradien warna spasial statis berdasarkan posisi asal
      float factor = (y + x) / (w + h);
      m_particles[count].hueOffset = 240 + factor * 120;
      m_particles[count].hueSpeed = 0;
      m_particles[count].baseSize = 2;

      count++;
    }
  }

  // Isi sisa partikel jika layar sangat lebar (keamanan cadangan)
  for (; count < PARTICLE_COUNT; count++) {
    m_particles[count] = m_particles[0];
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
  // Posisi kursor beku saat mouse keluar dari jendela

  for (int i = 0; i < PARTICLE_COUNT; i++) {
    CursorParticle &p = m_particles[i];

    p.hueOffset += p.hueSpeed * dt;

    // Vektor dari posisi asal partikel ke kursor
    float dx = m_cursorPos.x - p.homePos.x;
    float dy = m_cursorPos.y - p.homePos.y;
    float dist = sqrtf(dx * dx + dy * dy);

    // Tarikan gravitasi: partikel tertarik ke kursor
    // Tarikan kuat di dekat kursor, melemah secara eksponensial dengan jarak
    float pullMax = 70;    // pergeseran maksimum dalam piksel
    float pullDecay = 280; // radius pengaruh
    float pull = pullMax * expf(-dist / pullDecay);

    // Target = posisi asal yang digeser ke arah kursor
    Vector2 target;
    if (dist > 0.5) {
      float invDist = 1 / dist;
      target.x = p.homePos.x + dx * invDist * pull;
      target.y = p.homePos.y + dy * invDist * pull;
    } else {
      target = p.homePos;
    }

    // Pegas menuju posisi target
    float springK = 8;
    p.vel.x += (target.x - p.pos.x) * springK * dt;
    p.vel.y += (target.y - p.pos.y) * springK * dt;
    p.vel.x *= 0.90; // redaman
    p.vel.y *= 0.90;
    p.pos.x += p.vel.x;
    p.pos.y += p.vel.y;
  }
}

void MouseTrails::Draw() {
  for (int i = 0; i < PARTICLE_COUNT; i++) {
    const CursorParticle &p = m_particles[i];

    // Jarak dari posisi partikel saat ini ke kursor
    float dx = p.pos.x - m_cursorPos.x;
    float dy = p.pos.y - m_cursorPos.y;
    float dist = sqrtf(dx * dx + dy * dy);

    // Zona kosong di sekitar kursor
    if (dist < m_blankRadius)
      continue;

    // Morfing ukuran: panjang gelombang besar agar hanya satu puncak yang
    // terlihat
    float wavePhase = dist * 0.018 - m_timeElapsed * 0.5;
    float currentSize = 1.3 + sinf(wavePhase) * 1;

    // Alpha memudar di luar zona kosong agar tepinya tidak tajam
    float fadeRadius = 45;
    float alphaFactor = 1;
    if (dist < m_blankRadius + fadeRadius) {
      alphaFactor = (dist - m_blankRadius) / fadeRadius;
    } else if (dist > 150) {
      // Memudar lebih cepat agar efek tidak terlalu lebar
      alphaFactor = fmaxf(0, 1 - (dist - 150) / 150);
    }

    unsigned char alpha = (unsigned char)(190 * alphaFactor);
    if (alpha < 6)
      continue; // lewati partikel yang tidak terlihat

    // Warna berdasarkan gradien spasial
    Color c = ColorFromHSV(p.hueOffset, 0.85, 0.95);
    c.a = alpha;

    // Jejak gerak saat partikel bergerak cepat
    float speed = sqrtf(p.vel.x * p.vel.x + p.vel.y * p.vel.y);
    if (speed > 1.5 && currentSize > 1) {
      float trailLen = fminf(speed * 0.35, 8);
      float invSpd = 1 / speed;
      Vector2 dir = {p.vel.x * invSpd, p.vel.y * invSpd};
      Vector2 tail = {p.pos.x - dir.x * trailLen, p.pos.y - dir.y * trailLen};
      Fumbo::Graphic2D::DrawLineEx(tail, p.pos, currentSize * 0.7, c);
    } else {
      Fumbo::Graphic2D::DrawCircleV(p.pos, currentSize, c);
    }
  }
}
