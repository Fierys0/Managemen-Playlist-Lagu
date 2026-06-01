#include "addPlaylist.hpp"
#include "../core/appState.hpp"
#include "../core/globals.hpp"
#include "../core/vlcMetadata.hpp"
#include "fumbo.hpp"
#include "fumbo/external/portable-file-dialogs.h"
#include "mainMenu.hpp"
#include <algorithm>
#include <memory>
#include <string>

// Konstanta tata letak halaman buat playlist
static constexpr float AP_CONTENT_X = 80.0f;
static constexpr float AP_CONTENT_Y = 70.0f;

AddPlaylist::AddPlaylist(int playlistId) : m_playlistId(playlistId) {}

void AddPlaylist::Init() {
  // Tombol sampul playlist
  m_coverBtn = Fumbo::UI::Button({AP_CONTENT_X, AP_CONTENT_Y + 20, 190, 190});
  m_coverBtn.ApplyStyle(btnstyle);
  m_coverBtn.Roundness(0.12f);
  m_coverBtn.AddText(Lang::Get("+ Sampul", "+ Cover"), SpaceB, 18,
                     currentTheme.second1);

  // Konfigurasi textbox
  m_tbConfig.cornerRoundness = 0.15f;
  m_tbConfig.padding = {10, 8};
  m_tbConfig.backgroundColor = {30, 33, 40, 255};
  m_tbConfig.outlineColor = {60, 65, 80, 255};
  m_tbConfig.focusedOutlineColor = {80, 160, 255, 255};
  m_tbConfig.textColor = WHITE;
  m_tbConfig.cursorColor = {80, 160, 255, 255};

  // Textbox nama playlist
  m_titleBox = Fumbo::UI::Textbox(
      {AP_CONTENT_X + 210, AP_CONTENT_Y + 20, 450, 60}, SpaceB, 36);
  m_titleBox.SetStyle(m_tbConfig);
  m_titleBox.SetText(Lang::Get("Playlist Baru", "New Playlist"));

  // Tombol tambah musik
  m_addMusicBtn =
      Fumbo::UI::Button({AP_CONTENT_X + 210, AP_CONTENT_Y + 100, 320, 55});
  m_addMusicBtn.ApplyStyle(btnstyle);
  m_addMusicBtn.Roundness(0.2f);
  m_addMusicBtn.AddText(Lang::Get("+ Tambah File Musik", "+ Add Music Files"),
                        SpaceB, 18, currentTheme.second1);

  // Tombol simpan
  m_saveBtn =
      Fumbo::UI::Button({AP_CONTENT_X + 545, AP_CONTENT_Y + 100, 120, 55});
  m_saveBtn.ApplyStyle(btnstyle);
  m_saveBtn.Roundness(0.2f);
  m_saveBtn.AddText(Lang::Get("SIMPAN", "SAVE"), SpaceB, 20,
                    currentTheme.second1);

  // Tombol kembali ke menu utama
  m_backBtn = Fumbo::UI::Button({AP_CONTENT_X, AP_CONTENT_Y + 220, 190, 44});
  m_backBtn.ApplyStyle(btnstyle);
  m_backBtn.Roundness(0.2f);
  m_backBtn.AddText(Lang::Get("< Kembali", "< Back"), SpaceB, 20,
                    currentTheme.second1);

  // Muat data jika dalam Mode Edit
  if (m_playlistId != -1) {
    Playlist *found = nullptr;
    for (auto &pl : AppState::Instance().playlists) {
      if (pl.id == m_playlistId) {
        found = &pl;
        break;
      }
    }
    if (found) {
      m_titleBox.SetText(found->name);
      m_coverPath = found->coverPath;
      m_tracks = found->tracks;

      if (!m_coverPath.empty()) {
        FILE *f = fopen(m_coverPath.c_str(), "rb");
        if (f) {
          fclose(f);
          Image img = LoadImage(m_coverPath.c_str());
          if (img.data) {
            m_coverTex = LoadTextureFromImage(img);
            UnloadImage(img);
          }
        }
      }
      if (m_coverTex.id != 0) {
        m_coverBtn.SetTexture(m_coverTex);
        m_coverBtn.AddText("", SpaceB, 18, currentTheme.second1);
      }
    }

    // Konfigurasi tombol HAPUS
    m_deleteBtn =
        Fumbo::UI::Button({AP_CONTENT_X + 680, AP_CONTENT_Y + 100, 160, 55});
    m_deleteBtn.ApplyStyle(btnstyle);
    m_deleteBtn.Roundness(0.2f);
    m_deleteBtn.AddText(Lang::Get("HAPUS", "DELETE"), SpaceB, 20, RED);
  }
}

void AddPlaylist::Cleanup() {
  if (m_coverTex.id != 0) {
    UnloadTexture(m_coverTex);
    m_coverTex = {};
  }
}

void AddPlaylist::OpenAudioPicker() {
  pfd::open_file dialog("Pilih file audio", "",
                        {"File Audio (MP3, OGG, WAV, QOA, XM, MOD)",
                         "*.mp3 *.ogg *.wav *.qoa *.xm *.mod"},
                        pfd::opt::multiselect);
  auto files = dialog.result();
  for (const auto &path : files) {
    // Jangan tambahkan duplikat
    bool dup = false;
    for (const auto &t : m_tracks)
      if (t.filePath == path) {
        dup = true;
        break;
      }
    if (!dup) {
      Track t = VlcMeta::GetTrackInfo(path);
      m_tracks.push_back(t);
    }
  }
}

void AddPlaylist::OpenCoverPicker() {
  pfd::open_file dialog(
      "Pilih gambar sampul", "",
      {"File Gambar", "*.png *.jpg *.jpeg *.bmp *.gif", "Semua file", "*"},
      pfd::opt::none);
  auto files = dialog.result();
  if (!files.empty()) {
    m_coverPath = files[0];
    if (m_coverTex.id != 0)
      UnloadTexture(m_coverTex);
    Image img = LoadImage(m_coverPath.c_str());
    if (img.data) {
      m_coverTex = LoadTextureFromImage(img);
      UnloadImage(img);
      m_coverBtn.SetTexture(m_coverTex);
    }
  }
}

void AddPlaylist::Update() {
  m_titleBox.Update();

  // Perilaku tombol sampul saat hover
  if (m_coverBtn.IsHover() && !m_coverHovered) {
    m_coverBtn.AddText(Lang::Get("Ganti sampul", "Change cover"), SpaceB, 15,
                       currentTheme.second1);
    m_coverHovered = true;
  } else if (!m_coverBtn.IsHover() && m_coverHovered) {
    m_coverBtn.AddText(m_coverPath.empty() ? Lang::Get("+ Sampul", "+ Cover")
                                           : "",
                       SpaceB, 18, currentTheme.second1);
    m_coverHovered = false;
  }
  if (m_coverBtn.IsPressed())
    OpenCoverPicker();

  // Buka pemilih file audio
  if (m_addMusicBtn.IsPressed())
    OpenAudioPicker();

  // Perbarui label tombol dengan jumlah lagu yang ditambahkan
  std::string addLabel = Lang::Get("+ Tambah File Musik", "+ Add Music Files");
  if (!m_tracks.empty()) {
    addLabel = Lang::Get("+ Tambah Lagi  (", "+ Add More  (") +
               std::to_string(m_tracks.size()) + " " +
               Lang::Get("ditambahkan)", "added)");
  }
  m_addMusicBtn.AddText(addLabel, SpaceB, 18, currentTheme.second1);

  // Gulir daftar lagu
  m_trackListScrollY -= GetMouseWheelMove() * 30.0f;
  float maxScroll = fmaxf(0.0f, (float)m_tracks.size() * 48.0f - 300.0f);
  m_trackListScrollY = fmaxf(0.0f, fminf(m_trackListScrollY, maxScroll));

  // Tombol hapus playlist (hanya dalam Mode Edit)
  if (m_playlistId != -1) {
    if (m_deleteBtn.IsPressed()) {
      AppState::Instance().RemovePlaylist(m_playlistId);
      if (AppState::Instance().activePlaylistId == m_playlistId) {
        AppState::Instance().activePlaylistId = -1;
        AppState::Instance().queue.clear();
        AppState::Instance().currentQueueIndex = -1;
        AppState::Instance().isPlaying = false;
        Fumbo::Engine::Instance().GetAudioManager().StopMusic(0);
      }
      Fumbo::Instance().ChangeState(std::make_shared<MainMenu>());
      return;
    }
  }

  // Simpan playlist jika tombol SAVE ditekan dan nama tidak kosong
  if (m_saveBtn.IsPressed() && !m_titleBox.GetText().empty()) {
    Playlist pl;
    pl.name = m_titleBox.GetText();
    pl.coverPath = m_coverPath;
    pl.tracks = m_tracks;
    if (m_playlistId != -1) {
      AppState::Instance().UpdatePlaylist(m_playlistId, std::move(pl));
    } else {
      AppState::Instance().AddPlaylist(std::move(pl));
    }
    Fumbo::Instance().ChangeState(std::make_shared<MainMenu>());
  }

  if (m_backBtn.IsPressed()) {
    Fumbo::Instance().ChangeState(std::make_shared<MainMenu>());
  }

  // Interaksi per baris lagu (Hapus / Pindah)
  float listX = AP_CONTENT_X;
  float listY = AP_CONTENT_Y + 280.0f;
  float listW = 1280.0f - listX - 20.0f;
  float listH = 720.0f - listY - 20.0f;
  float rowH = 46.0f;
  float y = listY - m_trackListScrollY;

  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    Vector2 mouse = GetMousePosition();
    Vector2 scale = Fumbo::Utils::GetUIScale();
    Vector2 mouseUI = {mouse.x / scale.x, mouse.y / scale.y};

    if (mouseUI.x >= listX && mouseUI.x <= listX + listW &&
        mouseUI.y >= listY && mouseUI.y <= listY + listH) {

      for (size_t i = 0; i < m_tracks.size(); ++i) {
        if (y + rowH < listY || y > listY + listH) {
          y += rowH;
          continue;
        }

        // Tombol Naik (▲) di listX + listW - 140
        Rectangle upRec = {listX + listW - 140.0f, y + 8.0f, 30.0f, 30.0f};
        // Tombol Turun (▼) di listX + listW - 100
        Rectangle downRec = {listX + listW - 100.0f, y + 8.0f, 30.0f, 30.0f};
        // Tombol Hapus (X) di listX + listW - 50
        Rectangle delRec = {listX + listW - 50.0f, y + 8.0f, 30.0f, 30.0f};

        if (i > 0 && CheckCollisionPointRec(mouseUI, upRec)) {
          std::swap(m_tracks[i], m_tracks[i - 1]);
          break;
        }
        if (i + 1 < m_tracks.size() &&
            CheckCollisionPointRec(mouseUI, downRec)) {
          std::swap(m_tracks[i], m_tracks[i + 1]);
          break;
        }
        if (CheckCollisionPointRec(mouseUI, delRec)) {
          m_tracks.erase(m_tracks.begin() + i);
          break;
        }

        y += rowH;
      }
    }
  }
}

void AddPlaylist::DrawClean() {}

void AddPlaylist::DrawDirty() {
  // Latar belakang
  Fumbo::Graphic2D::DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                                  currentTheme.prim1);

  // Judul halaman
  std::string pageTitle = (m_playlistId != -1)
                              ? Lang::Get("UBAH PLAYLIST", "EDIT PLAYLIST")
                              : Lang::Get("BUAT PLAYLIST", "CREATE PLAYLIST");
  Fumbo::Graphic2D::DrawText(pageTitle, {320, 570}, SpaceB, 192,
                             {currentTheme.second1.r, currentTheme.second1.g,
                              currentTheme.second1.b, 127});

  m_coverBtn.Draw();

  // Textbox nama playlist
  m_titleBox.Draw();

  // Tombol tambah musik
  m_addMusicBtn.Draw();

  // Tombol simpan dan kembali
  m_saveBtn.Draw();
  m_backBtn.Draw();

  // Tombol hapus playlist (hanya dalam Mode Edit)
  if (m_playlistId != -1) {
    m_deleteBtn.Draw();
  }

  // Daftar lagu yang sudah ditambahkan
  float listX = AP_CONTENT_X;
  float listY = AP_CONTENT_Y + 280.0f;
  float listW = 1280.0f - listX - 20.0f;
  float listH = 720.0f - listY - 20.0f;

  // Header daftar lagu
  std::string headerText =
      m_tracks.empty()
          ? Lang::Get("Belum ada lagu ditambahkan", "No tracks added yet")
          : Lang::Get("Daftar Lagu:", "Track List:");
  Fumbo::Graphic2D::DrawText(headerText, {listX, listY}, SpaceB, 16,
                             {130, 135, 150, 255});

  float rowH = 46.0f;
  float y = listY - m_trackListScrollY;

  for (size_t i = 0; i < m_tracks.size(); ++i) {
    if (y + rowH < listY || y > listY + listH) {
      y += rowH;
      continue;
    }

    const auto &t = m_tracks[i];

    // Warna baris bergantian
    Color rowBg =
        (i % 2 == 0) ? Color{30, 33, 40, 200} : Color{25, 28, 35, 200};
    Fumbo::Graphic2D::DrawRectangleRounded({listX, y, listW, rowH - 2}, 0.1f, 6,
                                           rowBg);

    // Nomor urut
    std::string trackNum = std::to_string(i + 1) + ".";
    Fumbo::Graphic2D::DrawText(trackNum, {listX + 8, y + 12}, SpaceB, 16,
                               {100, 110, 130, 255});

    // Judul lagu
    std::string dispTitle = t.title.empty() ? t.filePath : t.title;
    if (dispTitle.size() > 40)
      dispTitle = dispTitle.substr(0, 37) + "...";
    Fumbo::Graphic2D::DrawText(dispTitle, {listX + 44, y + 12}, SpaceB, 18,
                               WHITE);

    // Nama artis
    if (!t.artist.empty()) {
      std::string dispArtist = t.artist;
      if (dispArtist.size() > 30)
        dispArtist = dispArtist.substr(0, 27) + "...";
      Fumbo::Graphic2D::DrawText(dispArtist, {listX + 500, y + 12}, SpaceB, 16,
                                 {150, 155, 170, 255});
    }

    // Durasi lagu (digeser ke kiri untuk memberi ruang bagi tombol aksi)
    if (t.durationMs > 0) {
      int sec = (int)(t.durationMs / 1000);
      int min = sec / 60;
      sec %= 60;
      char dur[16];
      snprintf(dur, sizeof(dur), "%d:%02d", min, sec);
      Fumbo::Graphic2D::DrawText(dur, {listX + listW - 190, y + 12}, SpaceB, 16,
                                 {130, 135, 150, 255});
    }

    // Tombol Naik (▲)
    Rectangle upRec = {listX + listW - 140.0f, y + 8.0f, 30.0f, 30.0f};
    if (i > 0) {
      bool hoverUp = CheckCollisionPointRec(GetMousePosition(), upRec);
      Fumbo::Graphic2D::DrawRectangleRounded(upRec, 0.2f, 4,
                                             hoverUp ? Color{80, 160, 255, 100}
                                                     : Color{50, 53, 60, 100});
      Fumbo::Graphic2D::DrawText("▲", {upRec.x + 8, upRec.y + 7}, SpaceB, 14,
                                 WHITE);
    }

    // Tombol Turun (▼)
    Rectangle downRec = {listX + listW - 100.0f, y + 8.0f, 30.0f, 30.0f};
    if (i + 1 < m_tracks.size()) {
      bool hoverDown = CheckCollisionPointRec(GetMousePosition(), downRec);
      Fumbo::Graphic2D::DrawRectangleRounded(
          downRec, 0.2f, 4,
          hoverDown ? Color{80, 160, 255, 100} : Color{50, 53, 60, 100});
      Fumbo::Graphic2D::DrawText("▼", {downRec.x + 8, downRec.y + 7}, SpaceB,
                                 14, WHITE);
    }

    // Tombol Hapus (X)
    Rectangle delRec = {listX + listW - 50.0f, y + 8.0f, 30.0f, 30.0f};
    bool hoverDel = CheckCollisionPointRec(GetMousePosition(), delRec);
    Fumbo::Graphic2D::DrawRectangleRounded(delRec, 0.2f, 4,
                                           hoverDel ? Color{230, 80, 80, 200}
                                                    : Color{80, 40, 40, 100});
    Fumbo::Graphic2D::DrawText("X", {delRec.x + 9, delRec.y + 7}, SpaceB, 14,
                               WHITE);

    y += rowH;
  }
}
