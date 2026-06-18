#include "playlistDetail.hpp"
#include "../core/appState.hpp"
#include "../core/core.hpp"
#include "../core/coverCache.hpp"
#include "../core/globals.hpp"
#include "addPlaylist.hpp"
#include "fumbo.hpp"
#include "mainMenu.hpp"
#include "playMusic.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <memory>
#include <string>

// Konstanta tata letak
static constexpr float PD_CONTENT_X = 80;
static constexpr float PD_CONTENT_Y = 60;

// Sampul
static constexpr float COVER_SZ = 200;
static constexpr float COVER_X = PD_CONTENT_X;
static constexpr float COVER_Y = PD_CONTENT_Y + 20;

// Area informasi di sebelah kanan sampul
static constexpr float INFO_X = COVER_X + COVER_SZ + 30;
static constexpr float INFO_Y = COVER_Y + 10;

// Tombol
static constexpr float BTN_Y = COVER_Y + COVER_SZ + 25;

// Daftar lagu
static constexpr float LIST_Y = BTN_Y + 90;
static constexpr float LIST_X = PD_CONTENT_X;
static constexpr float ROW_H = 56;

// Fungsi pembantu
static std::string FormatDuration(long long ms) {
  int totalSec = (int)(ms / 1000);
  int h = totalSec / 3600;
  int m = (totalSec % 3600) / 60;
  int s = totalSec % 60;
  char buf[32];
  if (h > 0)
    snprintf(buf, sizeof(buf), "%dh %dm", h, m);
  else
    snprintf(buf, sizeof(buf), "%d:%02d", m, s);
  return buf;
}

static std::string FormatTrackDuration(long long ms) {
  int sec = (int)(ms / 1000);
  int m = sec / 60;
  sec %= 60;
  char buf[16];
  snprintf(buf, sizeof(buf), "%d:%02d", m, sec);
  return buf;
}

// Konstruktor
PlaylistDetail::PlaylistDetail(int playlistId) : m_playlistId(playlistId) {}

const Playlist *PlaylistDetail::FindPlaylist() const {
  for (const auto &pl : AppState::Instance().playlists) {
    if (pl.id == m_playlistId)
      return &pl;
  }
  return nullptr;
}

// Inisialisasi
void PlaylistDetail::Init() {
  AppCore::currentScreen = AppCore::Screen::Home;

  const Playlist *pl = FindPlaylist();
  if (!pl) {
    Fumbo::Instance().ChangeState(std::make_shared<MainMenu>());
    return;
  }

  // Simpan data playlist ke memori lokal
  m_playlistName = pl->name;
  m_playlistDesc = pl->description;
  m_coverPath = pl->coverPath;
  m_tracks = pl->tracks.toVector();
  m_trackCount = (int)m_tracks.size();
  m_totalDurationMs = 0;
  for (const auto &t : m_tracks)
    m_totalDurationMs += t.durationMs;

  // Muat tekstur sampul dari CoverCache
  m_coverTex = {};
  if (!m_coverPath.empty()) {
    m_coverTex = CoverCache::Instance().Get(m_coverPath);
  }
  if (m_coverTex.id == 0) {
    // Coba gunakan gambar sampul dari lagu pertama
    for (const auto &t : m_tracks) {
      if (!t.coverArtPath.empty()) {
        m_coverTex = CoverCache::Instance().Get(t.coverArtPath);
        if (m_coverTex.id != 0)
          break;
      }
    }
  }
  if (m_coverTex.id == 0)
    m_coverTex = CoverCache::Instance().Get("assets/images/placeholder.png");

  // Muat tekstur ikon
  m_playIcon = Fumbo::Assets::LoadTextureThemed("assets/images/playIcon.png",
                                                currentTheme.second2);
  m_shuffleIcon = Fumbo::Assets::LoadTextureThemed(
      "assets/images/shuffleicon.png", currentTheme.second1);

  // Tombol putar lingkaran dengan warna aksen
  m_playBtn = Fumbo::UI::Button({INFO_X, BTN_Y, 56, 56});
  m_playBtn.ApplyStyle(btnstyle);
  m_playBtn.Roundness(1);
  m_playBtn.SetButtonColor(currentTheme.second2);
  m_playBtn.SetTexture(m_playIcon);

  // Tombol acak
  m_shuffleBtn = Fumbo::UI::Button({INFO_X + 72, BTN_Y, 56, 56});
  m_shuffleBtn.ApplyStyle(btnstyle);
  m_shuffleBtn.Roundness(1);
  m_shuffleBtn.SetButtonColor(currentTheme.prim2);
  m_shuffleBtn.SetTexture(m_shuffleIcon);

  // Tombol ubah
  m_editBtn = Fumbo::UI::Button({INFO_X + 144, BTN_Y + 6, 80, 44});
  m_editBtn.ApplyStyle(btnstyle);
  m_editBtn.Roundness(0.4);
  m_editBtn.AddText(Lang::Get("Ubah", "Edit"), SpaceB, 16,
                    currentTheme.second1);

  // Tombol kembali
  m_backBtn = Fumbo::UI::Button({PD_CONTENT_X, 300, 200, 50});
  m_backBtn.ApplyStyle(btnstyle);
  m_backBtn.Roundness(0.3);
  m_backBtn.AddText(Lang::Get("< Kembali", "< Back"), SpaceB, 16,
                    currentTheme.second1);

  m_scrollY = 0;
  m_hoveredTrack = -1;
}

// Pembersihan
void PlaylistDetail::Cleanup() {
  // CoverCache mengelola tekstur sehingga tidak perlu dibebaskan di sini
  if (m_playIcon.id != 0) {
    UnloadTexture(m_playIcon);
    m_playIcon = {};
  }
  if (m_shuffleIcon.id != 0) {
    UnloadTexture(m_shuffleIcon);
    m_shuffleIcon = {};
  }
}

// Pembaruan
void PlaylistDetail::Update() {
  const Playlist *pl = FindPlaylist();
  if (!pl) {
    Fumbo::Instance().ChangeState(std::make_shared<MainMenu>());
    return;
  }

  // Tombol kembali
  if (m_backBtn.IsPressed()) {
    Fumbo::Instance().ChangeState(std::make_shared<MainMenu>());
    return;
  }

  // Putar semua
  if (m_playBtn.IsPressed()) {
    AppState::Instance().PlayPlaylist(*pl);
    AppCore::currentScreen = AppCore::Screen::Play;
    Fumbo::Instance().ChangeState(std::make_shared<PlayMusic>());
    return;
  }

  // Acak
  if (m_shuffleBtn.IsPressed()) {
    AppState::Instance().PlayPlaylistShuffled(*pl);
    AppCore::currentScreen = AppCore::Screen::Play;
    Fumbo::Instance().ChangeState(std::make_shared<PlayMusic>());
    return;
  }

  // Ubah
  if (m_editBtn.IsPressed()) {
    Fumbo::Instance().ChangeState(std::make_shared<AddPlaylist>(m_playlistId));
    return;
  }

  // Pengguliran daftar lagu
  float listH = 720 - LIST_Y - 20;
  float maxScroll = fmaxf(0, (float)m_trackCount * ROW_H - listH);
  m_scrollY -= GetMouseWheelMove() * 35;
  m_scrollY = fmaxf(0, fminf(m_scrollY, maxScroll));

  // Deteksi hover lagu dan tombol putar per lagu
  Vector2 mouse = GetMousePosition();
  Vector2 scale = Fumbo::Utils::GetUIScale();
  Vector2 mouseUI = {mouse.x / scale.x, mouse.y / scale.y};

  float listW = 1280 - LIST_X - 20;
  m_hoveredTrack = -1;

  if (mouseUI.x >= LIST_X && mouseUI.x <= LIST_X + listW &&
      mouseUI.y >= LIST_Y && mouseUI.y <= LIST_Y + (720 - LIST_Y - 20)) {
    float y = LIST_Y - m_scrollY;
    for (int i = 0; i < m_trackCount; ++i) {
      if (y + ROW_H > LIST_Y && y < LIST_Y + (720 - LIST_Y - 20)) {
        if (mouseUI.y >= y && mouseUI.y < y + ROW_H) {
          m_hoveredTrack = i;

          // Periksa klik pada area tombol putar di sisi kanan baris
          float playBtnX = LIST_X + listW - 50;
          Rectangle playBtnRec = {playBtnX, y + 10, 32, 32};
          if (CheckCollisionPointRec(mouseUI, playBtnRec) &&
              IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            AppState::Instance().PlayPlaylistFromTrack(*pl, i);
            AppCore::currentScreen = AppCore::Screen::Play;
            Fumbo::Instance().ChangeState(std::make_shared<PlayMusic>());
            return;
          }
          break;
        }
      }
      y += ROW_H;
    }
  }
}

// Menggambar bersih
void PlaylistDetail::DrawClean() {}

// Menggambar kotor
void PlaylistDetail::DrawDirty() {
  float screenW = 1280;
  float listW = screenW - LIST_X - 20;
  float listH = 720 - LIST_Y - 20;

  // Latar belakang
  Fumbo::Graphic2D::DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                                  currentTheme.prim1);

  // Gradien atas di belakang area sampul
  Color topGrad = currentTheme.prim2;
  topGrad.a = 200;
  Fumbo::Graphic2D::DrawRectangleGradientV(
      0, 0, GetScreenWidth(), (int)(LIST_Y + 10), topGrad,
      {currentTheme.prim1.r, currentTheme.prim1.g, currentTheme.prim1.b, 0});

  // Gambar sampul
  if (m_coverTex.id != 0) {
    // Bayangan di belakang sampul
    Fumbo::Graphic2D::DrawRectangleRounded(
        {COVER_X - 3, COVER_Y - 3, COVER_SZ + 6, COVER_SZ + 6}, 0.06, 8,
        {0, 0, 0, 80});
    Fumbo::Graphic2D::DrawTexture(m_coverTex, {COVER_X, COVER_Y},
                                  {COVER_SZ, COVER_SZ});
  } else {
    Fumbo::Graphic2D::DrawRectangleRounded(
        {COVER_X, COVER_Y, COVER_SZ, COVER_SZ}, 0.06, 8, currentTheme.prim2);
    Fumbo::Graphic2D::DrawText(
        "♪", {COVER_X + COVER_SZ * 0.5 - 22, COVER_Y + COVER_SZ * 0.5 - 30},
        SpaceB, 60, {150, 160, 200, 200});
  }

  // Nama playlist besar
  Fumbo::Graphic2D::DrawText(m_playlistName, {INFO_X, INFO_Y}, SpaceB, 38,
                             currentTheme.second1);

  // Deskripsi playlist
  if (!m_playlistDesc.empty()) {
    Fumbo::Graphic2D::DrawText(m_playlistDesc, {INFO_X, INFO_Y + 48}, SpaceB,
                               18, {200, 205, 220, 255});
  }

  // Informasi jumlah lagu dan durasi
  std::string infoStr =
      std::to_string(m_trackCount) + " " + Lang::Get("lagu", "song(s)");
  if (m_totalDurationMs > 0) {
    infoStr += "  -  " + FormatDuration(m_totalDurationMs);
  }
  float infoYOffset = m_playlistDesc.empty() ? 48 : 85;
  Fumbo::Graphic2D::DrawText(infoStr, {INFO_X, INFO_Y + 100 + infoYOffset},
                             SpaceB, 16, {140, 145, 160, 255});

  // Tombol kembali
  m_backBtn.Draw();

  // Tombol putar acak dan ubah
  m_playBtn.Draw();
  m_shuffleBtn.Draw();
  m_editBtn.Draw();

  // Header daftar lagu
  float headerY = LIST_Y - 24;
  Fumbo::Graphic2D::DrawText("#", {LIST_X + 12, headerY}, SpaceB, 14,
                             {110, 115, 130, 255});
  Fumbo::Graphic2D::DrawText(Lang::Get("JUDUL", "TITLE"),
                             {LIST_X + 50, headerY}, SpaceB, 14,
                             {110, 115, 130, 255});
  Fumbo::Graphic2D::DrawText(Lang::Get("ARTIS", "ARTIST"),
                             {LIST_X + listW * 0.55, headerY}, SpaceB, 14,
                             {110, 115, 130, 255});
  Fumbo::Graphic2D::DrawText(Lang::Get("DURASI", "DURATION"),
                             {LIST_X + listW - 120, headerY}, SpaceB, 14,
                             {110, 115, 130, 255});

  // Garis pemisah
  Fumbo::Graphic2D::DrawRectangle((int)LIST_X, (int)(headerY + 22), (int)listW,
                                  1, {60, 65, 80, 100});

  // Daftar lagu dengan kliping scissor
  Rectangle scissorRec =
      Fumbo::Utils::UISpaceToScreen({LIST_X, LIST_Y, listW, listH});
  BeginScissorMode((int)scissorRec.x, (int)scissorRec.y, (int)scissorRec.width,
                   (int)scissorRec.height);

  float y = LIST_Y - m_scrollY;
  for (int i = 0; i < m_trackCount; ++i) {
    // Lewati baris yang berada di luar area terlihat
    if (y + ROW_H < LIST_Y) {
      y += ROW_H;
      continue;
    }
    if (y > LIST_Y + listH) {
      break;
    }

    const auto &t = m_tracks[i];

    // Latar belakang baris
    Color rowBg;
    if (m_hoveredTrack == i) {
      rowBg = {50, 55, 70, 200};
    } else if (i % 2 == 0) {
      rowBg = {25, 28, 35, 140};
    } else {
      rowBg = {30, 33, 42, 140};
    }

    // Soroti lagu yang sedang diputar
    bool isCurrentlyPlaying = false;
    if (AppState::Instance().activePlaylistId == m_playlistId) {
      const Track *cur = AppState::Instance().CurrentTrack();
      if (cur && cur->filePath == t.filePath) {
        rowBg = {currentTheme.second2.r, currentTheme.second2.g,
                 currentTheme.second2.b, 40};
        isCurrentlyPlaying = true;
      }
    }

    Fumbo::Graphic2D::DrawRectangleRounded({LIST_X, y, listW, ROW_H - 2}, 0.04,
                                           4, rowBg);

    // Nomor lagu
    Color numColor =
        isCurrentlyPlaying ? currentTheme.second2 : Color{100, 110, 130, 255};
    Fumbo::Graphic2D::DrawText(std::to_string(i + 1), {LIST_X + 14, y + 18},
                               SpaceB, 15, numColor);

    // Judul
    std::string dispTitle = t.title.empty() ? t.filePath : t.title;
    if (dispTitle.size() > 45)
      dispTitle = dispTitle.substr(0, 42) + "...";
    Color titleColor =
        isCurrentlyPlaying ? currentTheme.second2 : currentTheme.second1;
    Fumbo::Graphic2D::DrawText(dispTitle, {LIST_X + 50, y + 12}, SpaceB, 17,
                               titleColor);

    // Artis
    if (!t.artist.empty()) {
      std::string dispArtist = t.artist;
      if (dispArtist.size() > 30)
        dispArtist = dispArtist.substr(0, 27) + "...";
      Fumbo::Graphic2D::DrawText(dispArtist, {LIST_X + listW * 0.55, y + 18},
                                 SpaceB, 14, {140, 145, 160, 255});
    }

    // Durasi
    if (t.durationMs > 0) {
      Fumbo::Graphic2D::DrawText(FormatTrackDuration(t.durationMs),
                                 {LIST_X + listW - 115, y + 18}, SpaceB, 14,
                                 {120, 125, 140, 255});
    }

    // Tombol putar per lagu di sisi kanan
    float playBtnX = LIST_X + listW - 50;
    float playBtnY = y + 12;
    float playBtnSz = 32;

    if (m_hoveredTrack == i || isCurrentlyPlaying) {
      // Gambar tombol putar lingkaran kecil
      Color btnBg = isCurrentlyPlaying
                        ? Color{currentTheme.second2.r, currentTheme.second2.g,
                                currentTheme.second2.b, 180}
                        : Color{80, 85, 100, 180};
      Fumbo::Graphic2D::DrawCircleV(
          {playBtnX + playBtnSz / 2, playBtnY + playBtnSz / 2},
          playBtnSz / 2, btnBg);

      // Gambar segitiga putar
      float cx = playBtnX + playBtnSz / 2 + 2;
      float cy = playBtnY + playBtnSz / 2;
      float triH = 10;
      Fumbo::Graphic2D::DrawTriangle({cx - 5, cy - triH / 2},
                                     {cx - 5, cy + triH / 2}, {cx + 5, cy},
                                     WHITE);
    }

    y += ROW_H;
  }

  EndScissorMode();

  // Indikator pengguliran
  if (m_trackCount * ROW_H > listH) {
    float scrollBarH = listH * (listH / (m_trackCount * ROW_H));
    float maxScroll = m_trackCount * ROW_H - listH;
    float scrollBarY = LIST_Y + (m_scrollY / maxScroll) * (listH - scrollBarH);
    Fumbo::Graphic2D::DrawRectangleRounded(
        {LIST_X + listW - 6, scrollBarY, 4, scrollBarH}, 1, 4,
        {80, 85, 100, 120});
  }
}
