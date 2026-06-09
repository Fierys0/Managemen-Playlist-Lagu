#include "core.hpp"
#include "appState.hpp"
#include "fumbo.hpp"
#include "globals.hpp"
#include "states/mainMenu.hpp"
#include "states/playMusic.hpp"
#include "states/searchScreen.hpp"
#include "states/settings.hpp"
#include <cmath>
#include <memory>

// Definisi statis variabel layar aktif
AppCore::Screen AppCore::currentScreen = AppCore::Screen::Home;

// tata letak sidebar
static constexpr float NAV_X      = 0.0f;
static constexpr float NAV_W      = 70.0f;
static constexpr float NAV_BTN_H  = 70.0f;
static constexpr float NAV_HOME_Y = 80.0f;
static constexpr float NAV_PLAY_Y = 160.0f;
static constexpr float NAV_SEARCH_Y = 240.0f;
static constexpr float NAV_SET_Y  = 640.0f;

void AppCore::Init() {
  // Kontrol window
  exitBtn = Fumbo::UI::Button({1210, 10, 40, 40});
  minimizeBtn = Fumbo::UI::Button({1150, 10, 40, 40});
  maximizeBtn = Fumbo::UI::Button({1150, 10, 40, 40});
  exitBtn.ApplyStyle(btnstyle);
  minimizeBtn.ApplyStyle(btnstyle);
  maximizeBtn.ApplyStyle(btnstyle);
  exitBtn.AddText("X");
  minimizeBtn.AddText("_");

  titleBar = Fumbo::UI::Button({0, 0, 1280, 60});
  titleBar.TextOffsetX(-580);

  // Muat preferensi dan playlist dari disk saat pertama kali dijalankan
  AppState::Instance().LoadFromFile();
  if (AppState::Instance().themeName == "light") {
    currentTheme = lightTheme;
  } else {
    currentTheme = darkTheme;
  }

  // Tombol navigasi di sidebar kiri (dikecilkan menjadi 54x54 agar ikon tidak
  // renggang)
  navHomeBtn     = Fumbo::UI::Button({NAV_X + 8, NAV_HOME_Y   + 8, 54, 54});
  navPlayBtn     = Fumbo::UI::Button({NAV_X + 8, NAV_PLAY_Y   + 8, 54, 54});
  navSearchBtn   = Fumbo::UI::Button({NAV_X + 8, NAV_SEARCH_Y + 8, 54, 54});
  navSettingsBtn = Fumbo::UI::Button({NAV_X + 8, NAV_SET_Y    + 8, 54, 54});

  navHomeBtn.ApplyStyle(btnstyle);
  navPlayBtn.ApplyStyle(btnstyle);
  navSearchBtn.ApplyStyle(btnstyle);
  navSettingsBtn.ApplyStyle(btnstyle);
  navHomeBtn.Roundness(0.25f);
  navPlayBtn.Roundness(0.25f);
  navSearchBtn.Roundness(0.25f);
  navSettingsBtn.Roundness(0.25f);

  // Muat gambar navigasi dari asset pack
  m_navHomeTex     = Fumbo::Assets::LoadTexture("assets/images/homescreen.png");
  m_navPlayTex     = Fumbo::Assets::LoadTexture("assets/images/play.png");
  m_navSettingsTex = Fumbo::Assets::LoadTexture("assets/images/settings.png");
  m_navSearchTex   = Fumbo::Assets::LoadTexture("assets/images/seachicon.png");

  // Hubungkan tekstur gambar ke tombol
  navHomeBtn.SetTexture(m_navHomeTex);
  navPlayBtn.SetTexture(m_navPlayTex);
  navSettingsBtn.SetTexture(m_navSettingsTex);
  navSearchBtn.SetTexture(m_navSearchTex);

  mouseTrails.Init();
  mouseTrails.SetEnabled(AppState::Instance().trailsEnabled);
}

void AppCore::Cleanup() {
  if (m_navHomeTex.id != 0)
    UnloadTexture(m_navHomeTex);
  if (m_navPlayTex.id != 0)
    UnloadTexture(m_navPlayTex);
  if (m_navSettingsTex.id != 0)
    UnloadTexture(m_navSettingsTex);
  if (m_navSearchTex.id != 0)
    UnloadTexture(m_navSearchTex);
}

void AppCore::Update() {
  AppState::Instance().UpdateMusicPlayback();
  mouseTrails.SetEnabled(AppState::Instance().trailsEnabled);
  mouseTrails.Update();

  // Kontrol window
  if (exitBtn.IsPressed())
    Fumbo::Engine::Instance().Quit();
  if (minimizeBtn.IsPressed())
    MinimizeWindow();

  if (titleBar.IsPressed() && !exitBtn.IsHover() && !maximizeBtn.IsHover() &&
      !minimizeBtn.IsHover()) {
    m_isDragging = true;
    m_dragOffset = GetMousePosition();
    m_windowPosCache = GetWindowPosition();
  }
  if (m_isDragging) {
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
      Vector2 mousePos = GetMousePosition();
      Vector2 delta = {mousePos.x - m_dragOffset.x,
                       mousePos.y - m_dragOffset.y};
      if (delta.x != 0 || delta.y != 0) {
        m_windowPosCache.x += delta.x;
        m_windowPosCache.y += delta.y;
        SetWindowPosition((int)m_windowPosCache.x, (int)m_windowPosCache.y);
        SetMousePosition((int)m_dragOffset.x, (int)m_dragOffset.y);
      }
    } else {
      m_isDragging = false;
    }
  }

  // Warna title bar
  titleBar.IdleColor(currentTheme.titlebar);
  titleBar.SetButtonColor(currentTheme.titlebar);
  titleBar.HoveredColor(currentTheme.titlebar);
  titleBar.AddText("Magfy", SpaceB, 24, currentTheme.second1);

  // Warna untuk ikon navigasi menggunakan tema saat ini
  Color iconIdleColor = (currentTheme.id == darkTheme.id) ? Color{160, 165, 175, 255} : Color{100, 105, 115, 255};
  Color iconActiveColor = currentTheme.second2;
  Color iconHoverColor = (currentTheme.id == darkTheme.id) ? WHITE : Color{16, 18, 18, 255};

  // Tombol Home
  if (currentScreen == Screen::Home) {
    navHomeBtn.IdleColor(iconActiveColor);
    navHomeBtn.HoveredColor(iconActiveColor);
  } else {
    navHomeBtn.IdleColor(iconIdleColor);
    navHomeBtn.HoveredColor(iconHoverColor);
  }

  // Tombol Play
  if (currentScreen == Screen::Play) {
    navPlayBtn.IdleColor(iconActiveColor);
    navPlayBtn.HoveredColor(iconActiveColor);
  } else {
    navPlayBtn.IdleColor(iconIdleColor);
    navPlayBtn.HoveredColor(iconHoverColor);
  }

  // Tombol Search
  if (currentScreen == Screen::Search) {
    navSearchBtn.IdleColor(iconActiveColor);
    navSearchBtn.HoveredColor(iconActiveColor);
  } else {
    navSearchBtn.IdleColor(iconIdleColor);
    navSearchBtn.HoveredColor(iconHoverColor);
  }

  // Tombol Settings
  if (currentScreen == Screen::Settings) {
    navSettingsBtn.IdleColor(iconActiveColor);
    navSettingsBtn.HoveredColor(iconActiveColor);
  } else {
    navSettingsBtn.IdleColor(iconIdleColor);
    navSettingsBtn.HoveredColor(iconHoverColor);
  }

  // Navigasi saat tombol ditekan
  if (navHomeBtn.IsPressed() && currentScreen != Screen::Home) {
    currentScreen = Screen::Home;
    Fumbo::Instance().ChangeState(std::make_shared<MainMenu>());
  }

  if (navPlayBtn.IsPressed()) {
    if (AppState::Instance().HasActivePlaylist()) {
      if (currentScreen != Screen::Play) {
        currentScreen = Screen::Play;
        Fumbo::Instance().ChangeState(std::make_shared<PlayMusic>());
      }
    } else {
      // Tampilkan pesan pengingat selama 3 detik
      m_noPlaylistNudgeTimer = 3.0f;
    }
  }

  if (navSearchBtn.IsPressed() && currentScreen != Screen::Search) {
    currentScreen = Screen::Search;
    Fumbo::Instance().ChangeState(std::make_shared<SearchScreen>());
  }

  if (navSettingsBtn.IsPressed() && currentScreen != Screen::Settings) {
    currentScreen = Screen::Settings;
    Fumbo::Instance().ChangeState(std::make_shared<Settings>());
  }

  if (m_noPlaylistNudgeTimer > 0.0f)
    m_noPlaylistNudgeTimer -= GetFrameTime();
}

void AppCore::DrawClean() { ClearBackground({16, 18, 18, 255}); }

void AppCore::DrawDirty() {
  // background navbar
  Fumbo::Graphic2D::DrawRectangle((int)NAV_X, 60, (int)NAV_W,
                                  GetScreenHeight() - 60, currentTheme.second1);

  // Latar belakang tombol navigasi (bulat)
  auto drawBg = [](const Fumbo::UI::Button &btn, bool isActive, float x,
                   float y) {
    Rectangle bgRec = {x + 8, y + 8, 54, 54};
    Color bgCol = isActive ? Color{currentTheme.second2.r, currentTheme.second2.g, currentTheme.second2.b, 60}
                           : (btn.IsHover() ? (currentTheme.id == darkTheme.id ? Color{55, 60, 70, 200} : Color{210, 215, 220, 200})
                                             : (currentTheme.id == darkTheme.id ? Color{30, 33, 40, 150} : Color{225, 228, 230, 150}));
    Fumbo::Graphic2D::DrawRectangleRounded(bgRec, 0.25f, 6, bgCol);
    if (isActive) {
      Fumbo::Graphic2D::DrawRectangleRoundedLinesEx(bgRec, 0.25f, 6, 1.5f,
                                                    Color{currentTheme.second2.r, currentTheme.second2.g, currentTheme.second2.b, 180});
    }
  };

  drawBg(navHomeBtn,     currentScreen == Screen::Home,   NAV_X, NAV_HOME_Y);
  drawBg(navPlayBtn,     currentScreen == Screen::Play,   NAV_X, NAV_PLAY_Y);
  drawBg(navSearchBtn,   currentScreen == Screen::Search, NAV_X, NAV_SEARCH_Y);
  drawBg(navSettingsBtn, currentScreen == Screen::Settings, NAV_X, NAV_SET_Y);

  // Tombol navigasi
  navHomeBtn.Draw();
  navPlayBtn.Draw();
  navSearchBtn.Draw();
  navSettingsBtn.Draw();

  // Garis pemisah antara sidebar dan konten
  Fumbo::Graphic2D::DrawRectangle((int)NAV_W, 60, 2, GetScreenHeight() - 60,
                                  currentTheme.second1);

  // Pesan pengingat "pilih playlist dulu" dengan animasi alpha
  if (m_noPlaylistNudgeTimer > 0.0f) {
    float alpha = fminf(1.0f, m_noPlaylistNudgeTimer);
    unsigned char a = (unsigned char)(alpha * 220);
    Fumbo::Graphic2D::DrawRectangleRounded({NAV_W + 10, NAV_PLAY_Y, 280, 55},
                                           0.3f, 8, {30, 33, 40, a});
    std::string nudgeText = Lang::Get("Pilih playlist terlebih dahulu :)",
                                      "Pick a playlist first :)");
    Fumbo::Graphic2D::DrawText(nudgeText, {NAV_W + 20, NAV_PLAY_Y + 15}, SpaceB,
                               18, {200, 200, 200, a});
  }

  // Title bar dan kontrol window
  titleBar.Draw();
  exitBtn.Draw();
  minimizeBtn.Draw();

  // Jejak mouse (ditampilkan paling belakang)
  if (mouseTrails.IsEnabled())
    mouseTrails.Draw();
}
