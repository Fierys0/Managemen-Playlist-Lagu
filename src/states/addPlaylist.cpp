#include "addPlaylist.hpp"
#include "../core/appState.hpp"
#include "../core/globals.hpp"
#include "../core/vlcMetadata.hpp"
#include "fumbo.hpp"
#include "mainMenu.hpp"
#include <algorithm>
#include <memory>
#include <string>

// Konstanta tata letak halaman buat playlist
static constexpr float AP_CONTENT_X = 80;
static constexpr float AP_CONTENT_Y = 70;

AddPlaylist::AddPlaylist(int playlistId) : m_playlistId(playlistId) {}

void AddPlaylist::Init() {
  // Tombol sampul playlist
  m_coverBtn = Fumbo::UI::Button({AP_CONTENT_X, AP_CONTENT_Y + 20, 190, 190});
  m_coverBtn.ApplyStyle(btnstyle);
  m_coverBtn.Roundness(0.12);
  m_coverBtn.AddText(Lang::Get("+ Sampul", "+ Cover"), SpaceB, 18,
                     currentTheme.second1);

  // Konfigurasi kotak teks
  m_tbConfig.cornerRoundness = 0.15;
  m_tbConfig.padding = {10, 8};
  m_tbConfig.backgroundColor = {30, 33, 40, 255};
  m_tbConfig.outlineColor = {60, 65, 80, 255};
  m_tbConfig.focusedOutlineColor = {80, 160, 255, 255};
  m_tbConfig.textColor = WHITE;
  m_tbConfig.cursorColor = {80, 160, 255, 255};

  // Kotak teks nama playlist
  m_titleBox = Fumbo::UI::Textbox(
      {AP_CONTENT_X + 210, AP_CONTENT_Y + 15, 630, 45}, SpaceB, 24);
  m_titleBox.SetStyle(m_tbConfig);
  m_titleBox.SetText(Lang::Get("Playlist Baru", "New Playlist"));

  // Kotak teks deskripsi playlist
  m_descBox = Fumbo::UI::Textbox(
      {AP_CONTENT_X + 210, AP_CONTENT_Y + 70, 630, 80}, SpaceB, 18);
  m_descBox.SetStyle(m_tbConfig);
  m_descBox.SetMultiline(true);
  m_descBox.SetMaxLines(3);
  m_descBox.SetText(Lang::Get("Deskripsi Playlist", "Playlist Description"));

  // Tombol tambah musik
  m_addMusicBtn =
      Fumbo::UI::Button({AP_CONTENT_X + 210, AP_CONTENT_Y + 160, 320, 45});
  m_addMusicBtn.ApplyStyle(btnstyle);
  m_addMusicBtn.Roundness(0.2);
  m_addMusicBtn.AddText(Lang::Get("+ Tambah File Musik", "+ Add Music Files"),
                        SpaceB, 18, currentTheme.second1);

  // Tombol simpan
  m_saveBtn =
      Fumbo::UI::Button({AP_CONTENT_X + 545, AP_CONTENT_Y + 160, 120, 45});
  m_saveBtn.ApplyStyle(btnstyle);
  m_saveBtn.Roundness(0.2);
  m_saveBtn.AddText(Lang::Get("SIMPAN", "SAVE"), SpaceB, 20,
                    currentTheme.second1);

  // Tombol kembali
  m_backBtn = Fumbo::UI::Button({AP_CONTENT_X, AP_CONTENT_Y + 220, 190, 44});
  m_backBtn.ApplyStyle(btnstyle);
  m_backBtn.Roundness(0.2);
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
      m_descBox.SetText(found->description);
      m_coverPath = found->coverPath;
      // [LINKED LIST] Konversi DoublyLinkedList ke vector untuk diubah di
      // antarmuka pengguna
      m_tracks = found->tracks.toVector();

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
        Fumbo::UI::Button({AP_CONTENT_X + 680, AP_CONTENT_Y + 160, 160, 45});
    m_deleteBtn.ApplyStyle(btnstyle);
    m_deleteBtn.Roundness(0.2);
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
  if (m_audioDialog || m_coverDialog)
    return;

  m_audioDialog = std::make_unique<Fumbo::FileDialog::OpenFileAsync>(
      "Pilih file audio",
      std::vector<std::string>{"File Audio (MP3, OGG, WAV, QOA, XM, MOD)",
                               "*.mp3 *.ogg *.wav *.qoa *.xm *.mod"},
      true);
}

void AddPlaylist::OpenCoverPicker() {
  if (m_audioDialog || m_coverDialog)
    return;

  m_coverDialog = std::make_unique<Fumbo::FileDialog::OpenFileAsync>(
      "Pilih gambar sampul",
      std::vector<std::string>{"File Gambar", "*.png *.jpg *.jpeg *.bmp *.gif",
                               "Semua file", "*"});
}

void AddPlaylist::Update() {
  if (m_coverDialog) {
    if (m_coverDialog->IsReady()) {
      auto results = m_coverDialog->GetResult();
      std::string picked = results.empty() ? "" : results[0];
      if (!picked.empty()) {
        m_coverPath = picked;
        if (m_coverTex.id != 0)
          UnloadTexture(m_coverTex);
        Image img = LoadImage(m_coverPath.c_str());
        if (img.data) {
          m_coverTex = LoadTextureFromImage(img);
          UnloadImage(img);
          m_coverBtn.SetTexture(m_coverTex);
        }
      }
      m_coverDialog.reset();
    }
    return;
  }

  if (m_audioDialog) {
    if (m_audioDialog->IsReady()) {
      auto files = m_audioDialog->GetResult();
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
      m_audioDialog.reset();
    }
    return;
  }

  m_titleBox.Update();
  m_descBox.Update();

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
  m_trackListScrollY -= GetMouseWheelMove() * 30;
  float maxScroll = fmaxf(0, (float)m_tracks.size() * 48 - 300);
  m_trackListScrollY = fmaxf(0, fminf(m_trackListScrollY, maxScroll));

  // Tombol hapus playlist (hanya dalam Mode Edit)
  if (m_playlistId != -1) {
    if (m_deleteBtn.IsPressed()) {
      AppState::Instance().RemovePlaylist(m_playlistId);
      if (AppState::Instance().activePlaylistId == m_playlistId) {
        AppState::Instance().activePlaylistId = -1;
        // Bersihkan antrean pemutaran melingkar
        AppState::Instance().playQueue.clear();
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
    pl.description = m_descBox.GetText();
    pl.coverPath = m_coverPath;
    // [LINKED LIST] Konversi vector kembali ke DoublyLinkedList untuk
    // penyimpanan
    for (auto &t : m_tracks)
      pl.tracks.pushBack(std::move(t));
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

  // Interaksi per baris lagu
  float listX = AP_CONTENT_X;
  float listY = AP_CONTENT_Y + 280;
  float listW = 1280 - listX - 20;
  float listH = 720 - listY - 20;
  float rowH = 46;
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

        // Tombol Naik
        Rectangle upRec = {listX + listW - 140, y + 8, 30, 30};
        // Tombol Turun
        Rectangle downRec = {listX + listW - 100, y + 8, 30, 30};
        // Tombol Hapus
        Rectangle delRec = {listX + listW - 50, y + 8, 30, 30};

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

  // Kotak teks nama playlist
  m_titleBox.Draw();

  // Kotak teks deskripsi playlist
  m_descBox.Draw();

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
  float listY = AP_CONTENT_Y + 280;
  float listW = 1280 - listX - 20;
  float listH = 720 - listY - 20;

  // Header daftar lagu
  std::string headerText =
      m_tracks.empty()
          ? Lang::Get("Belum ada lagu ditambahkan", "No tracks added yet")
          : Lang::Get("Daftar Lagu:", "Track List:");
  Fumbo::Graphic2D::DrawText(headerText, {listX, listY}, SpaceB, 16,
                             {130, 135, 150, 255});

  float rowH = 46;
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
    Fumbo::Graphic2D::DrawRectangleRounded({listX, y, listW, rowH - 2}, 0.1, 6,
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

    // Durasi lagu
    if (t.durationMs > 0) {
      int sec = (int)(t.durationMs / 1000);
      int min = sec / 60;
      sec %= 60;
      char dur[16];
      snprintf(dur, sizeof(dur), "%d:%02d", min, sec);
      Fumbo::Graphic2D::DrawText(dur, {listX + listW - 190, y + 12}, SpaceB, 16,
                                 {130, 135, 150, 255});
    }

    // Tombol Naik
    Rectangle upRec = {listX + listW - 140, y + 8, 30, 30};
    if (i > 0) {
      bool hoverUp = CheckCollisionPointRec(GetMousePosition(), upRec);
      Fumbo::Graphic2D::DrawRectangleRounded(upRec, 0.2, 4,
                                             hoverUp ? Color{80, 160, 255, 100}
                                                     : Color{50, 53, 60, 100});
      Fumbo::Graphic2D::DrawText("Up", {upRec.x + 8, upRec.y + 7}, SpaceB, 14,
                                 WHITE);
    }

    // Tombol Turun
    Rectangle downRec = {listX + listW - 100, y + 8, 30, 30};
    if (i + 1 < m_tracks.size()) {
      bool hoverDown = CheckCollisionPointRec(GetMousePosition(), downRec);
      Fumbo::Graphic2D::DrawRectangleRounded(
          downRec, 0.2, 4,
          hoverDown ? Color{80, 160, 255, 100} : Color{50, 53, 60, 100});
      Fumbo::Graphic2D::DrawText("Dn", {downRec.x + 8, downRec.y + 7}, SpaceB,
                                 14, WHITE);
    }

    // Tombol Hapus
    Rectangle delRec = {listX + listW - 50, y + 8, 30, 30};
    bool hoverDel = CheckCollisionPointRec(GetMousePosition(), delRec);
    Fumbo::Graphic2D::DrawRectangleRounded(delRec, 0.2, 4,
                                           hoverDel ? Color{230, 80, 80, 200}
                                                    : Color{80, 40, 40, 100});
    Fumbo::Graphic2D::DrawText("X", {delRec.x + 9, delRec.y + 7}, SpaceB, 14,
                               WHITE);

    y += rowH;
  }
}
