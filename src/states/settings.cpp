#include "settings.hpp"
#include "../core/appState.hpp"
#include "../core/core.hpp"
#include "../core/globals.hpp"
#include "fumbo.hpp"
#include "mainMenu.hpp"
#include <string>

// Konstanta posisi konten halaman pengaturan
static constexpr float S_CONTENT_X = 90.0f;
static constexpr float S_CONTENT_Y = 80.0f;

void Settings::Init() {
  AppCore::currentScreen = AppCore::Screen::Settings;

  // Tombol ganti tema warna
  m_toggleThemeBtn = Fumbo::UI::Button({S_CONTENT_X, S_CONTENT_Y, 260, 52});
  m_toggleThemeBtn.ApplyStyle(btnstyle);
  m_toggleThemeBtn.Roundness(0.2f);
  m_toggleThemeBtn.AddText(Lang::Get("Ganti Tema", "Change Theme"), SpaceB, 20,
                           currentTheme.second1);

  // Tombol aktifkan/nonaktifkan efek jejak mouse
  bool trailsOn = AppState::Instance().trailsEnabled;
  m_toggleTrailsBtn =
      Fumbo::UI::Button({S_CONTENT_X, S_CONTENT_Y + 70, 260, 52});
  m_toggleTrailsBtn.ApplyStyle(btnstyle);
  m_toggleTrailsBtn.Roundness(0.2f);
  std::string trailLabel =
      Lang::Get("Jejak: ", "Trails: ") +
      (trailsOn ? Lang::Get("AKTIF", "ON") : Lang::Get("MATI", "OFF"));
  m_toggleTrailsBtn.AddText(trailLabel, SpaceB, 20, currentTheme.second1);

  // Tombol ganti bahasa
  m_toggleLangBtn =
      Fumbo::UI::Button({S_CONTENT_X, S_CONTENT_Y + 140, 260, 52});
  m_toggleLangBtn.ApplyStyle(btnstyle);
  m_toggleLangBtn.Roundness(0.2f);
  std::string langLabel = Lang::Get("Bahasa: INDONESIA", "Language: ENGLISH");
  m_toggleLangBtn.AddText(langLabel, SpaceB, 20, currentTheme.second1);

  // Tombol kembali ke menu utama
  m_backBtn = Fumbo::UI::Button({S_CONTENT_X, S_CONTENT_Y + 230, 130, 48});
  m_backBtn.ApplyStyle(btnstyle);
  m_backBtn.Roundness(0.2f);
  m_backBtn.AddText(Lang::Get("< Kembali", "< Back"), SpaceB, 18,
                    currentTheme.second1);
}

void Settings::Cleanup() {}

void Settings::Update() {
  // Ganti tema terang/gelap
  if (m_toggleThemeBtn.IsPressed()) {
    currentTheme = (currentTheme.id == darkTheme.id) ? lightTheme : darkTheme;
    AppState::Instance().themeName =
        (currentTheme.id == darkTheme.id) ? "dark" : "light";
    AppState::Instance().SaveToFile();

    m_toggleThemeBtn.AddText(Lang::Get("Ganti Tema", "Change Theme"), SpaceB,
                             20, currentTheme.second1);
    m_toggleTrailsBtn.AddText(Lang::Get("Jejak: ", "Trails: ") +
                                  (AppState::Instance().trailsEnabled
                                       ? Lang::Get("AKTIF", "ON")
                                       : Lang::Get("MATI", "OFF")),
                              SpaceB, 20, currentTheme.second1);
    m_toggleLangBtn.AddText(Lang::Get("Bahasa: INDONESIA", "Language: ENGLISH"),
                            SpaceB, 20, currentTheme.second1);
    m_backBtn.AddText(Lang::Get("< Kembali", "< Back"), SpaceB, 18,
                      currentTheme.second1);

    Fumbo::Engine::Instance().InvalidateCleanLayer();
  }

  // Aktifkan atau nonaktifkan efek jejak mouse
  if (m_toggleTrailsBtn.IsPressed()) {
    AppState::Instance().trailsEnabled = !AppState::Instance().trailsEnabled;
    AppState::Instance().SaveToFile();
    std::string label =
        Lang::Get("Jejak: ", "Trails: ") + (AppState::Instance().trailsEnabled
                                                ? Lang::Get("AKTIF", "ON")
                                                : Lang::Get("MATI", "OFF"));
    m_toggleTrailsBtn.AddText(label, SpaceB, 20, currentTheme.second1);
  }

  // Ganti bahasa (preferensi lokal)
  if (m_toggleLangBtn.IsPressed()) {
    AppState::Instance().language =
        (AppState::Instance().language == "id") ? "en" : "id";
    AppState::Instance().SaveToFile();

    m_toggleThemeBtn.AddText(Lang::Get("Ganti Tema", "Change Theme"), SpaceB,
                             20, currentTheme.second1);
    m_toggleTrailsBtn.AddText(Lang::Get("Jejak: ", "Trails: ") +
                                  (AppState::Instance().trailsEnabled
                                       ? Lang::Get("AKTIF", "ON")
                                       : Lang::Get("MATI", "OFF")),
                              SpaceB, 20, currentTheme.second1);
    m_toggleLangBtn.AddText(Lang::Get("Bahasa: INDONESIA", "Language: ENGLISH"),
                            SpaceB, 20, currentTheme.second1);
    m_backBtn.AddText(Lang::Get("< Kembali", "< Back"), SpaceB, 18,
                      currentTheme.second1);

    Fumbo::Engine::Instance().InvalidateCleanLayer();
  }

  // Kembali ke menu utama
  if (m_backBtn.IsPressed()) {
    AppCore::currentScreen = AppCore::Screen::Home;
    Fumbo::Instance().ChangeState(std::make_shared<MainMenu>());
  }
}

void Settings::DrawClean() {}

void Settings::DrawDirty() {
  Fumbo::Graphic2D::DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                                  currentTheme.prim1);

  // Judul halaman
  Fumbo::Graphic2D::DrawText(Lang::Get("PENGATURAN", "SETTINGS"), {520, 550},
                             SpaceB, 256,
                             {currentTheme.second1.r, currentTheme.second1.g,
                              currentTheme.second1.b, 127});

  // Label tema saat ini (digeser sedikit ke kanan karena tombol melebar)
  std::string themeLabel =
      Lang::Get("Tema: ", "Theme: ") + (currentTheme.id == darkTheme.id
                                            ? Lang::Get("Gelap", "Dark")
                                            : Lang::Get("Terang", "Light"));
  Fumbo::Graphic2D::DrawText(themeLabel, {S_CONTENT_X + 275, S_CONTENT_Y + 14},
                             SpaceB, 20, currentTheme.second1);

  // Keterangan status efek jejak
  std::string trailsLabel =
      AppState::Instance().trailsEnabled
          ? Lang::Get("Efek partikel jejak mouse aktif",
                      "Mouse trail particle effects enabled")
          : Lang::Get("Efek partikel jejak mouse nonaktif",
                      "Mouse trail particle effects disabled");
  Fumbo::Graphic2D::DrawText(trailsLabel, {S_CONTENT_X + 275, S_CONTENT_Y + 84},
                             SpaceB, 18, {130, 135, 155, 255});

  // Keterangan status pilihan bahasa
  std::string langDesc =
      Lang::Get("Bahasa aplikasi saat ini", "Current application language");
  Fumbo::Graphic2D::DrawText(langDesc, {S_CONTENT_X + 275, S_CONTENT_Y + 154},
                             SpaceB, 18, {130, 135, 155, 255});

  m_toggleThemeBtn.Draw();
  m_toggleTrailsBtn.Draw();
  m_toggleLangBtn.Draw();
  m_backBtn.Draw();
}
