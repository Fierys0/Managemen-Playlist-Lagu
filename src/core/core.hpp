#pragma once
#include "appState.hpp"
#include "fumbo.hpp"
#include "trails.hpp"

class AppCore : public IGameState {
public:
  void Init() override;
  void Cleanup() override;
  void Update() override;
  void DrawClean() override;
  void DrawDirty() override;

  // Layar mana yang sedang ditampilkan (digunakan untuk menyoroti tombol
  // navigasi aktif)
  enum class Screen { Home, Play, Settings, Search };
  static Screen currentScreen;

private:
  Vector2 m_dragOffset = {0, 0};
  Vector2 m_windowPosCache = {0, 0};

  // Kontrol title bar dan jendela
  Fumbo::UI::Button exitBtn;
  Fumbo::UI::Button maximizeBtn;
  Fumbo::UI::Button minimizeBtn;
  Fumbo::UI::Button titleBar;
  bool m_isDragging = false;

  // Tombol navigasi sidebar kiri
  Fumbo::UI::Button navHomeBtn;
  Fumbo::UI::Button navPlayBtn;
  Fumbo::UI::Button navSettingsBtn;
  Fumbo::UI::Button navSearchBtn;

  // Tekstur ikon navigasi
  Texture2D m_navHomeTex{};
  Texture2D m_navPlayTex{};
  Texture2D m_navSettingsTex{};
  Texture2D m_navSearchTex{};

  // Pengingat "pilih playlist dulu" saat Play ditekan tanpa playlist aktif
  float m_noPlaylistNudgeTimer = 0.0f;

  MouseTrails mouseTrails;
};
