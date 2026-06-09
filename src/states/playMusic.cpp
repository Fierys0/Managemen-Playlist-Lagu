#include "playMusic.hpp"
#include "../core/appState.hpp"
#include "../core/core.hpp"
#include "../core/globals.hpp"
#include "../core/vlcMetadata.hpp"
#include "fumbo.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

// Konstanta tata letak layar pemutaran musik
static constexpr float PM_CONTENT_X = 90.0f;
static constexpr float PM_CONTENT_Y = 75.0f;
static constexpr float COVER_SIZE = 280.0f;
static constexpr float COVER_X = 490.0f;
static constexpr float COVER_Y = 110.0f;

// Baris kontrol pemutaran
static constexpr float CTRL_Y = 480.0f;
static constexpr float CTRL_BTN_W = 70.0f;
static constexpr float CTRL_BTN_H = 70.0f;
static constexpr float CTRL_CENTER_X = 640.0f;

// Posisi bilah progres
static constexpr float PROG_Y = 565.0f;
static constexpr float PROG_H = 6.0f;

// Format detik menjadi string menit:detik
static std::string FormatTime(float seconds) {
  int s = (int)seconds;
  int m = s / 60;
  s %= 60;
  char buf[16];
  snprintf(buf, sizeof(buf), "%d:%02d", m, s);
  return buf;
}

void PlayMusic::Init() {
  AppCore::currentScreen = AppCore::Screen::Play;
  auto &state = AppState::Instance();
  auto &audio = Fumbo::Engine::Instance().GetAudioManager();

  // Muat tekstur ikon tombol
  m_playIcon = Fumbo::Assets::LoadTextureThemed("assets/images/playIcon.png",
                                                currentTheme.second2);
  m_pauseIcon = Fumbo::Assets::LoadTextureThemed("assets/images/pauseIcon.png",
                                                 currentTheme.second2);
  m_nextIcon = Fumbo::Assets::LoadTextureThemed("assets/images/nextIcon.png",
                                                currentTheme.second2);

  // Membalik nextIcon secara horizontal untuk membuat prevIcon
  Image img = Fumbo::Assets::LoadImage("assets/images/nextIcon.png");
  if (img.data) {
    ImageFlipHorizontal(&img);
    m_prevIcon = LoadTextureFromImage(img);
    UnloadImage(img);
    // Ubah warna piksel menggunakan fitur engine baru
    Fumbo::Assets::RecolorTexture(m_prevIcon, currentTheme.second2);
  }

  // Tombol lagu sebelumnya
  m_prevBtn =
      Fumbo::UI::Button({CTRL_CENTER_X - 130, CTRL_Y, CTRL_BTN_W, CTRL_BTN_H});
  m_prevBtn.ApplyStyle(btnstyle);
  m_prevBtn.Roundness(0.5f);
  m_prevBtn.SetTexture(m_prevIcon);

  // Tombol putar atau jeda dengan ukuran lebih besar
  m_playPauseBtn = Fumbo::UI::Button({CTRL_CENTER_X - 40, CTRL_Y - 10, 80, 80});
  m_playPauseBtn.ApplyStyle(btnstyle);
  m_playPauseBtn.Roundness(0.5f);
  m_playPauseBtn.SetTexture(state.isPlaying ? m_pauseIcon : m_playIcon);

  // Tombol lagu berikutnya
  m_nextBtn =
      Fumbo::UI::Button({CTRL_CENTER_X + 60, CTRL_Y, CTRL_BTN_W, CTRL_BTN_H});
  m_nextBtn.ApplyStyle(btnstyle);
  m_nextBtn.Roundness(0.5f);
  m_nextBtn.SetTexture(m_nextIcon);

  // Konfigurasi slider progres
  Fumbo::UI::SliderConfig sliderCfg;
  sliderCfg.trackColor = currentTheme.prim2;
  sliderCfg.progressColor = currentTheme.second2;
  sliderCfg.knobColor = WHITE;
  sliderCfg.knobWidth = 14.0f;
  sliderCfg.trackHeight = PROG_H;
  m_progressSlider = Fumbo::UI::Slider(0.0f, 1.0f, 0.0f);
  m_progressSlider.SetStyle(sliderCfg);

  // Konfigurasi slider volume
  Fumbo::UI::SliderConfig volSliderCfg;
  volSliderCfg.trackColor = currentTheme.prim2;
  volSliderCfg.progressColor = currentTheme.second2;
  volSliderCfg.knobColor = WHITE;
  volSliderCfg.knobWidth = 10.0f;
  volSliderCfg.trackHeight = 4.0f;
  m_volumeSlider = Fumbo::UI::Slider(0.0f, 1.0f, audio.GetChannelVolume(0));
  m_volumeSlider.SetStyle(volSliderCfg);

  // Muat lagu yang sedang aktif
  LoadCurrentTrack();
}

void PlayMusic::Cleanup() {
  if (m_coverTex.id != 0) {
    UnloadTexture(m_coverTex);
    m_coverTex = {};
  }
  if (m_playIcon.id != 0) {
    UnloadTexture(m_playIcon);
    m_playIcon = {};
  }
  if (m_pauseIcon.id != 0) {
    UnloadTexture(m_pauseIcon);
    m_pauseIcon = {};
  }
  if (m_nextIcon.id != 0) {
    UnloadTexture(m_nextIcon);
    m_nextIcon = {};
  }
  if (m_prevIcon.id != 0) {
    UnloadTexture(m_prevIcon);
    m_prevIcon = {};
  }
}

void PlayMusic::LoadCurrentTrack() {
  auto &state = AppState::Instance();
  const Track *t = state.CurrentTrack();
  if (!t)
    return;

  // Bebaskan cover sebelumnya
  if (m_coverTex.id != 0) {
    UnloadTexture(m_coverTex);
    m_coverTex = {};
  }

  m_loadedQueueIndex = state.currentQueueIndex;

  // Muat tekstur sampul seni
  m_coverTex = VlcMeta::LoadCoverTexture(t->coverArtPath);

  // Simpan metadata untuk ditampilkan
  m_displayTitle = t->title.empty()
                       ? Lang::Get("Judul Tidak Diketahui", "Unknown Title")
                       : t->title;
  m_displayArtist = t->artist.empty()
                        ? Lang::Get("Artis Tidak Diketahui", "Unknown Artist")
                        : t->artist;
  m_displayAlbum = t->album;
  m_coverAngle = 0.0f;

  // Warna aksen mengikuti tema saat ini
  m_accentColor = currentTheme.second2;
}

void PlayMusic::Update() {
  auto &state = AppState::Instance();
  auto &audio = Fumbo::Engine::Instance().GetAudioManager();

  // Muat ulang jika indeks antrean berubah dari luar
  if (state.currentQueueIndex != m_loadedQueueIndex)
    LoadCurrentTrack();

  // Beralih putar atau jeda
  if (m_playPauseBtn.IsPressed()) {
    state.TogglePlayPause();
  }
  m_playPauseBtn.SetTexture(state.isPlaying ? m_pauseIcon : m_playIcon);

  // Lagu berikutnya
  if (m_nextBtn.IsPressed()) {
    state.NextTrack();
    LoadCurrentTrack();
  }

  // Lagu sebelumnya
  if (m_prevBtn.IsPressed()) {
    state.PrevTrack();
    LoadCurrentTrack();
  }

  // Perbarui jangkauan dan nilai slider
  float totalLength = audio.GetMusicLength(0);
  m_progressSlider.SetRange(0.0f, totalLength > 0.0f ? totalLength : 1.0f);

  float progW = 700.0f;
  float progX = (1280.0f - progW) * 0.5f;

  // Jika mouse tidak menekan layar perbarui nilai slider dari posisi lagu
  if (!IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
    m_progressSlider.SetValue(audio.GetMusicPlayed(0));
  }

  // Perbarui deteksi geser slider dan periksa perpindahan posisi
  if (m_progressSlider.Update({progX, PROG_Y, progW, PROG_H})) {
    audio.SeekMusic(m_progressSlider.GetValue(), 0);
  }

  float volW = 120.0f;
  float volX = CTRL_CENTER_X + 190.0f;
  float volY = CTRL_Y + 26.0f;
  float volH = 8.0f;

  // Perbarui deteksi geser slider volume dan perbarui volume audio
  if (m_volumeSlider.Update({volX, volY, volW, volH})) {
    audio.SetChannelVolume(0, m_volumeSlider.GetValue());
  }

  // Animasi rotasi sampul saat memutar
  if (state.isPlaying)
    m_coverAngle += GetFrameTime() * 15.0f;
}

void PlayMusic::DrawDirty() {
  auto &state = AppState::Instance();
  auto &audio = Fumbo::Engine::Instance().GetAudioManager();

  // Latar belakang utama
  Fumbo::Graphic2D::DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                                  currentTheme.prim1);

  // Gradien di belakang area sampul
  Color topGrad = currentTheme.prim2;
  topGrad.a = 255;
  Fumbo::Graphic2D::DrawRectangleGradientV(
      (int)PM_CONTENT_X, (int)PM_CONTENT_Y - 10,
      GetScreenWidth() - (int)PM_CONTENT_X - 10,
      (int)(COVER_Y + COVER_SIZE + 60), topGrad,
      {currentTheme.prim1.r, currentTheme.prim1.g, currentTheme.prim1.b, 0});

  // Efek cahaya melingkar di sekitar sampul
  float glowR = COVER_SIZE * 0.5f + 18.0f;
  float cx = COVER_X + COVER_SIZE * 0.5f;
  float cy = COVER_Y + COVER_SIZE * 0.5f;
  if (state.isPlaying) {
    float pulse = 0.5f + 0.5f * sinf(GetTime() * 2.0f);
    Color gc = {m_accentColor.r, m_accentColor.g, m_accentColor.b,
                (unsigned char)(60 + (int)(40 * pulse))};
    Fumbo::Graphic2D::DrawCircleV({cx, cy}, glowR + pulse * 8.0f, gc);
  }

  // Bingkai sampul
  Fumbo::Graphic2D::DrawRectangleRounded(
      {COVER_X - 4, COVER_Y - 4, COVER_SIZE + 8, COVER_SIZE + 8}, 0.08f, 8,
      {m_accentColor.r, m_accentColor.g, m_accentColor.b, 60});

  // Gambar sampul atau placeholder
  if (m_coverTex.id != 0) {
    Fumbo::Graphic2D::DrawTexture(m_coverTex, {COVER_X, COVER_Y},
                                  {COVER_SIZE, COVER_SIZE});
  } else {
    Fumbo::Graphic2D::DrawRectangleGradientV(
        (int)COVER_X, (int)COVER_Y, (int)COVER_SIZE, (int)COVER_SIZE,
        currentTheme.second2, currentTheme.prim2);
    Fumbo::Graphic2D::DrawText("♪", {cx - 22, cy - 30}, SpaceB, 60,
                               {150, 160, 200, 200});
  }

  // Informasi lagu
  float infoX = PM_CONTENT_X;
  float infoY = COVER_Y + COVER_SIZE + 20.0f;

  Fumbo::Graphic2D::DrawText(m_displayTitle, {infoX + 10, infoY}, SpaceB, 32,
                             currentTheme.second1);
  Fumbo::Graphic2D::DrawText(m_displayArtist, {infoX + 10, infoY + 40}, SpaceB,
                             22, {160, 165, 180, 255});
  if (!m_displayAlbum.empty())
    Fumbo::Graphic2D::DrawText(m_displayAlbum, {infoX + 10, infoY + 68}, SpaceB,
                               17, {110, 115, 135, 255});

  // Info antrean dan daftar lagu berikutnya
  if (!state.queue.empty()) {
    std::string qInfo = Lang::Get("Lagu ", "Song ") +
                        std::to_string(state.currentQueueIndex + 1) + " " +
                        Lang::Get("dari ", "of ") +
                        std::to_string(state.queue.size());
    Fumbo::Graphic2D::DrawText(qInfo, {COVER_X + COVER_SIZE + 20, COVER_Y},
                               SpaceB, 16, {110, 115, 135, 255});

    // Daftar lagu yang akan diputar berikutnya
    float qy = COVER_Y + 30.0f;
    Fumbo::Graphic2D::DrawText(Lang::Get("Selanjutnya:", "Next Up:"),
                               {COVER_X + COVER_SIZE + 20, qy}, SpaceB, 18,
                               {130, 135, 155, 255});
    qy += 28;
    for (int i = 1;
         i <= 5 && state.currentQueueIndex + i < (int)state.queue.size(); ++i) {
      const auto &nt = state.queue[state.currentQueueIndex + i];
      std::string ntTitle = nt.title.empty() ? nt.filePath : nt.title;
      if (ntTitle.size() > 28)
        ntTitle = ntTitle.substr(0, 25) + "...";
      Color nc =
          (i == 1) ? Color{200, 205, 220, 255} : Color{120, 125, 145, 255};
      Fumbo::Graphic2D::DrawText(ntTitle, {COVER_X + COVER_SIZE + 20, qy},
                                 SpaceB, 15, nc);
      qy += 24;
    }
  }

  // Bilah progres background
  float progW = 700.0f;
  float progX = (1280.0f - progW) * 0.5f;
  Fumbo::Graphic2D::DrawRectangleRounded({progX, PROG_Y, progW, PROG_H}, 1.0f,
                                         4, currentTheme.prim2);

  // Widget slider progres
  m_progressSlider.Draw({progX, PROG_Y, progW, PROG_H});

  // Label waktu main
  std::string playedTimeStr = FormatTime(audio.GetMusicPlayed(0));
  Fumbo::Graphic2D::DrawText(playedTimeStr, {progX - 45, PROG_Y - 6}, SpaceB,
                             14, {110, 115, 135, 255});

  if (state.CurrentTrack() && state.CurrentTrack()->durationMs > 0) {
    int sec = (int)(state.CurrentTrack()->durationMs / 1000);
    int m = sec / 60;
    sec %= 60;
    char dur[16];
    snprintf(dur, sizeof(dur), "%d:%02d", m, sec);
    Fumbo::Graphic2D::DrawText(dur, {progX + progW + 6, PROG_Y - 6}, SpaceB, 14,
                               {110, 115, 135, 255});
  }

  // Tombol kontrol
  m_prevBtn.Draw();
  m_playPauseBtn.Draw();
  m_nextBtn.Draw();

  // Label volume dan slider volume
  Fumbo::Graphic2D::DrawText("VOL", {CTRL_CENTER_X + 150, CTRL_Y + 22}, SpaceB,
                             14, {100, 105, 120, 255});
  m_volumeSlider.Draw({CTRL_CENTER_X + 190.0f, CTRL_Y + 26.0f, 120.0f, 8.0f});
}

void PlayMusic::DrawClean() {}
