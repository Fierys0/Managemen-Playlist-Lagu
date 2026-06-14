#include "searchScreen.hpp"
#include "../core/appState.hpp"
#include "../core/core.hpp"
#include "../core/coverCache.hpp"
#include "../core/globals.hpp"
#include "fumbo.hpp"
#include "mainMenu.hpp"
#include "playMusic.hpp"

#include <algorithm>
#include <cctype>
#include <memory>
#include <string>

// Tata letak layar pencarian.
static constexpr float SS_SIDEBAR_W = 72.0f;
static constexpr float SS_TITLEBAR_H = 60.0f;
static constexpr float SS_PADDING = 24.0f;

// Dimensi kotak pencarian
static constexpr float SS_SEARCH_X = SS_SIDEBAR_W + SS_PADDING;
static constexpr float SS_SEARCH_Y = SS_TITLEBAR_H + SS_PADDING;
static constexpr float SS_SEARCH_W = 1280.0f - SS_SIDEBAR_W - SS_PADDING * 2.0f;
static constexpr float SS_SEARCH_H = 56.0f;

// Area hasil pencarian
static constexpr float SS_RESULTS_X = SS_SIDEBAR_W + SS_PADDING;
static constexpr float SS_RESULTS_Y = SS_SEARCH_Y + SS_SEARCH_H + 20.0f;
static constexpr float SS_RESULTS_W =
    1280.0f - SS_SIDEBAR_W - SS_PADDING * 2.0f;
static constexpr float SS_RESULTS_H = 720.0f - SS_RESULTS_Y - SS_PADDING;

// Ukuran tiap baris hasil
static constexpr float SS_ROW_H = 76.0f;
static constexpr float SS_COVER_SZ = 56.0f;
static constexpr float SS_ROW_GAP = 4.0f;

// Fungsi bantu mengubah string ke huruf kecil untuk pencocokan tanpa
// memperhatikan kapitalisasi.
static std::string toLower(const std::string &s) {
  std::string out = s;
  for (auto &c : out)
    c = (char)std::tolower((unsigned char)c);
  return out;
}

void SearchScreen::Init() {
  AppCore::currentScreen = AppCore::Screen::Search;

  // Konfigurasi kotak teks
  m_tbConfig.cornerRoundness = 0.4f;
  m_tbConfig.padding = {16, 10};
  m_tbConfig.backgroundColor = {currentTheme.prim2.r, currentTheme.prim2.g,
                                currentTheme.prim2.b, 230};
  m_tbConfig.outlineColor = {60, 65, 80, 255};
  m_tbConfig.focusedOutlineColor = currentTheme.second2;
  m_tbConfig.textColor =
      currentTheme.id == darkTheme.id ? WHITE : Color{20, 20, 30, 255};
  m_tbConfig.cursorColor = currentTheme.second2;

  m_searchBox = Fumbo::UI::Textbox(
      {SS_SEARCH_X, SS_SEARCH_Y, SS_SEARCH_W - 70.0f, SS_SEARCH_H}, SpaceB, 26);
  m_searchBox.SetStyle(m_tbConfig);
  m_searchBox.SetText("");

  // Tombol hapus (×) di sebelah kanan kotak pencarian
  m_clearBtn = Fumbo::UI::Button(
      {SS_SEARCH_X + SS_SEARCH_W - 62.0f, SS_SEARCH_Y, 56.0f, SS_SEARCH_H});
  m_clearBtn.ApplyStyle(btnstyle);
  m_clearBtn.Roundness(0.4f);
  m_clearBtn.AddText("X", SpaceB, 30, {200, 80, 80, 255});

  RebuildResults();
}

void SearchScreen::Cleanup() {
  // [HASH MAP] Tidak perlu UnloadTexture satu per satu karena
  // masa hidup tekstur dikelola oleh CoverCache (hash map).
  // Cukup bersihkan vektor referensi lokal.
  m_resultCovers.clear();
}

void SearchScreen::RebuildResults() {
  // [HASH MAP] Bersihkan referensi lokal tanpa menghapus tekstur dari cache
  m_resultCovers.clear();
  m_filteredIndices.clear();
  m_scrollY = 0.0f;

  const std::string query = toLower(m_searchBox.GetText());
  const auto &playlists = AppState::Instance().playlists;

  for (int i = 0; i < (int)playlists.size(); ++i) {
    const auto &pl = playlists[i];
    // Tampilkan semua playlist jika kueri kosong, atau yang namanya cocok
    if (query.empty() || toLower(pl.name).find(query) != std::string::npos) {
      m_filteredIndices.push_back(i);

      // [HASH MAP] Ambil tekstur sampul dari CoverCache (hash map)
      // Alih-alih memuat file gambar dari disk setiap kali,
      // CoverCache.Get() akan mengembalikan tekstur dari hash map jika
      // sudah pernah dimuat sebelumnya (cache hit, O(1)).
      // Jika belum ada (cache miss), baru dimuat dari disk dan disimpan.
      Texture2D tex{};
      bool loaded = false;

      // Coba dari coverPath playlist
      if (!pl.coverPath.empty()) {
        tex = CoverCache::Instance().Get(pl.coverPath);
        if (tex.id != 0)
          loaded = true;
      }

      // [HASH MAP] Coba dari coverArtPath lagu pertama yang punya cover
      if (!loaded) {
        // [LINKED LIST] Iterasi menggunakan range-for pada DoublyLinkedList
        for (const auto &t : pl.tracks) {
          if (!t.coverArtPath.empty()) {
            tex = CoverCache::Instance().Get(t.coverArtPath);
            if (tex.id != 0) {
              loaded = true;
              break;
            }
          }
        }
      }

      // [HASH MAP] Gunakan placeholder dari cache jika tidak ada cover
      if (!loaded)
        tex = CoverCache::Instance().Get("assets/images/placeholder.png");

      m_resultCovers.push_back(tex);
    }
  }

  m_lastQuery = m_searchBox.GetText();
}

void SearchScreen::Update() {
  m_searchBox.Update();

  // Tombol clear
  if (m_clearBtn.IsPressed()) {
    m_searchBox.SetText("");
    RebuildResults();
  }

  // Bangun ulang hasil jika teks berubah
  if (m_searchBox.GetText() != m_lastQuery) {
    RebuildResults();
  }

  // Gulir daftar hasil
  m_scrollY -= GetMouseWheelMove() * 36.0f;
  float totalRowH = (float)m_filteredIndices.size() * (SS_ROW_H + SS_ROW_GAP);
  float maxScroll = std::max(0.0f, totalRowH - SS_RESULTS_H);
  m_scrollY = std::max(0.0f, std::min(m_scrollY, maxScroll));

  // Deteksi klik pada baris hasil
  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    Vector2 mouse = GetMousePosition();
    Vector2 scale = Fumbo::Utils::GetUIScale();
    Vector2 mouseUI = {mouse.x / scale.x, mouse.y / scale.y};

    float y = SS_RESULTS_Y - m_scrollY;
    for (int ri = 0; ri < (int)m_filteredIndices.size(); ++ri) {
      float rowTop = y + ri * (SS_ROW_H + SS_ROW_GAP);
      float rowBot = rowTop + SS_ROW_H;

      // Klip baris yang di luar area tampilan
      if (rowBot < SS_RESULTS_Y || rowTop > SS_RESULTS_Y + SS_RESULTS_H)
        continue;

      Rectangle rowRec = {SS_RESULTS_X, rowTop, SS_RESULTS_W, SS_ROW_H};
      if (CheckCollisionPointRec(mouseUI, rowRec)) {
        int plIdx = m_filteredIndices[ri];
        const auto &pl = AppState::Instance().playlists[plIdx];
        if (AppState::Instance().activePlaylistId != pl.id)
          AppState::Instance().PlayPlaylist(pl);
        AppCore::currentScreen = AppCore::Screen::Play;
        Fumbo::Instance().ChangeState(std::make_shared<PlayMusic>());
        return;
      }
    }
  }
}

void SearchScreen::DrawClean() {}

void SearchScreen::DrawDirty() {
  // Latar belakang
  Fumbo::Graphic2D::DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                                  currentTheme.prim1);

  // Teks judul besar yang tampil samar di latar belakang sebagai dekorasi.
  Fumbo::Graphic2D::DrawText(Lang::Get("CARI", "SEARCH"), {380, 540}, SpaceB,
                             256,
                             {currentTheme.second1.r, currentTheme.second1.g,
                              currentTheme.second1.b, 20});

  // Label kecil di atas kotak pencarian.
  Fumbo::Graphic2D::DrawText(Lang::Get("Cari Playlist", "Search Playlists"),
                             {SS_SEARCH_X, SS_TITLEBAR_H + 6.0f}, SpaceB, 16,
                             {currentTheme.second1.r, currentTheme.second1.g,
                              currentTheme.second1.b, 160});

  // Gambar kotak pencarian beserta bayangan di belakangnya.
  // Bayangan halus di belakang kotak
  Fumbo::Graphic2D::DrawRectangleRounded(
      {SS_SEARCH_X - 2, SS_SEARCH_Y - 2, SS_SEARCH_W - 66.0f, SS_SEARCH_H + 4},
      0.4f, 8,
      {currentTheme.second2.r, currentTheme.second2.g, currentTheme.second2.b,
       30});

  m_searchBox.Draw();
  m_clearBtn.Draw();

  // Placeholder text jika kotak kosong
  if (m_searchBox.GetText().empty()) {
    Fumbo::Graphic2D::DrawText(
        Lang::Get("Ketik nama playlist...", "Type a playlist name..."),
        {SS_SEARCH_X + 18.0f, SS_SEARCH_Y + 16.0f}, SpaceB, 22,
        {currentTheme.second1.r, currentTheme.second1.g, currentTheme.second1.b,
         55});
  }

  // Teks kecil yang menampilkan jumlah playlist yang ditemukan.
  std::string countStr;
  if (m_filteredIndices.empty()) {
    countStr = Lang::Get("Tidak ada hasil", "No results found");
  } else {
    countStr = std::to_string(m_filteredIndices.size()) + " " +
               Lang::Get("playlist ditemukan", "playlists found");
  }
  Fumbo::Graphic2D::DrawText(countStr, {SS_RESULTS_X, SS_RESULTS_Y - 18.0f},
                             SpaceB, 14, {130, 135, 155, 200});

  // Render setiap baris hasil, lewati baris yang berada di luar area tampil.

  float y = SS_RESULTS_Y - m_scrollY;

  for (int ri = 0; ri < (int)m_filteredIndices.size(); ++ri) {
    float rowTop = y + ri * (SS_ROW_H + SS_ROW_GAP);
    float rowBot = rowTop + SS_ROW_H;

    // Lewati baris yang sepenuhnya di luar area tampil
    if (rowBot < SS_RESULTS_Y || rowTop > SS_RESULTS_Y + SS_RESULTS_H)
      continue;

    int plIdx = m_filteredIndices[ri];
    const auto &pl = AppState::Instance().playlists[plIdx];

    // Gambar latar belakang baris dengan warna bergantian dan efek hover.
    bool isHovered = false;
    {
      Vector2 mouse = GetMousePosition();
      Vector2 scale = Fumbo::Utils::GetUIScale();
      Vector2 mouseUI = {mouse.x / scale.x, mouse.y / scale.y};
      Rectangle rowRec = {SS_RESULTS_X, rowTop, SS_RESULTS_W, SS_ROW_H};
      isHovered = CheckCollisionPointRec(mouseUI, rowRec) &&
                  rowTop >= SS_RESULTS_Y &&
                  rowBot <= SS_RESULTS_Y + SS_RESULTS_H;
    }

    Color rowBg;
    if (isHovered) {
      rowBg = {currentTheme.second2.r, currentTheme.second2.g,
               currentTheme.second2.b, 40};
    } else if (ri % 2 == 0) {
      rowBg = currentTheme.id == darkTheme.id ? Color{30, 33, 42, 200}
                                              : Color{235, 238, 242, 200};
    } else {
      rowBg = currentTheme.id == darkTheme.id ? Color{25, 28, 37, 200}
                                              : Color{242, 245, 248, 200};
    }
    Fumbo::Graphic2D::DrawRectangleRounded(
        {SS_RESULTS_X, rowTop, SS_RESULTS_W, SS_ROW_H}, 0.18f, 8, rowBg);

    // Garis kiri berwarna aksen saat hover atau playlist aktif
    bool isActive = AppState::Instance().activePlaylistId == pl.id;
    if (isHovered || isActive) {
      Fumbo::Graphic2D::DrawRectangle(
          (int)SS_RESULTS_X, (int)rowTop, 4, (int)SS_ROW_H,
          {currentTheme.second2.r, currentTheme.second2.g,
           currentTheme.second2.b, (unsigned char)(isActive ? 255 : 180)});
    }

    // Gambar thumbnail sampul di sisi kiri baris.
    float coverX = SS_RESULTS_X + 12.0f;
    float coverY = rowTop + (SS_ROW_H - SS_COVER_SZ) / 2.0f;
    if (ri < (int)m_resultCovers.size() && m_resultCovers[ri].id != 0) {
      const Texture2D &tex = m_resultCovers[ri];
      // Gambar dengan mempertahankan rasio aspek
      float scale = SS_COVER_SZ / (float)std::max(tex.width, tex.height);
      float dw = tex.width * scale;
      float dh = tex.height * scale;
      Fumbo::Graphic2D::DrawTexturePro(
          tex, {0, 0, (float)tex.width, (float)tex.height},
          {coverX + (SS_COVER_SZ - dw) / 2.0f,
           coverY + (SS_COVER_SZ - dh) / 2.0f, dw, dh},
          {0, 0}, 0.0f, WHITE);
    }

    // Gambar nama playlist dan batas panjang teks agar tidak melebihi lebar
    // baris.
    float textX = coverX + SS_COVER_SZ + 16.0f;
    std::string dispName = pl.name;
    if (dispName.size() > 45)
      dispName = dispName.substr(0, 42) + "...";
    Fumbo::Graphic2D::DrawText(dispName, {textX, rowTop + 14.0f}, SpaceB, 22,
                               isHovered ? WHITE : currentTheme.second1);

    // [LINKED LIST] Gambar jumlah lagu menggunakan DoublyLinkedList::size()
    std::string trackCount =
        std::to_string(pl.tracks.size()) + " " + Lang::Get("lagu", "tracks");
    Fumbo::Graphic2D::DrawText(trackCount, {textX, rowTop + 44.0f}, SpaceB, 15,
                               {130, 135, 155, 200});

    // Tampilkan indikator teks kecil jika playlist ini sedang diputar.
    if (isActive) {
      Fumbo::Graphic2D::DrawText(
          Lang::Get("▶ Diputar", "▶ Playing"),
          {SS_RESULTS_X + SS_RESULTS_W - 130.0f, rowTop + 26.0f}, SpaceB, 15,
          {currentTheme.second2.r, currentTheme.second2.g,
           currentTheme.second2.b, 230});
    }
  }

  // Tampilkan pesan jika pencarian tidak menghasilkan kecocokan apapun.
  if (m_filteredIndices.empty() && !m_searchBox.GetText().empty()) {
    Fumbo::Graphic2D::DrawText(
        Lang::Get("Tidak ada playlist yang cocok", "No matching playlists"),
        {SS_RESULTS_X + SS_RESULTS_W / 2.0f - 180.0f,
         SS_RESULTS_Y + SS_RESULTS_H / 2.0f - 20.0f},
        SpaceB, 22, {100, 105, 120, 200});
  }

  // Pesan jika belum ada playlist sama sekali
  if (AppState::Instance().playlists.empty()) {
    Fumbo::Graphic2D::DrawText(
        Lang::Get("Belum ada playlist. Buat satu di menu Home.",
                  "No playlists yet. Create one from the Home screen."),
        {SS_RESULTS_X + 10.0f, SS_RESULTS_Y + 20.0f}, SpaceB, 18,
        {100, 105, 120, 200});
  }
}
