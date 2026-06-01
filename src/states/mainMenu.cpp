#include "mainMenu.hpp"
#include "../core/appState.hpp"
#include "../core/core.hpp"
#include "../core/globals.hpp"
#include "../core/vlcMetadata.hpp"
#include "addPlaylist.hpp"
#include "fumbo.hpp"
#include "playMusic.hpp"
#include <memory>

// Area konten dimulai setelah sidebar kiri (70px) dan title bar (60px)
static constexpr float CONTENT_X = 72.0f;
static constexpr float CONTENT_Y = 70.0f;

// Tata letak kartu playlist
static constexpr float CARD_W = 210.0f;
static constexpr float CARD_H = 270.0f;
static constexpr float CARD_GAP = 20.0f;

void MainMenu::Init() {
  m_plusBtn = Fumbo::UI::Button({1200, 650, 56, 56});
  m_plusBtn.ApplyStyle(btnstyle);
  m_plusBtn.Roundness(0.5f);
  m_plusBtn.AddText("+", SpaceB, 48, currentTheme.second1);

  // Initialize page navigation buttons (glassmorphic slide style)
  m_leftBtn = Fumbo::UI::Button({CONTENT_X + 10, 290, 44, 80});
  m_leftBtn.ApplyStyle(btnstyle);
  m_leftBtn.Roundness(0.2f);
  m_leftBtn.AddText("<", SpaceB, 28, WHITE);

  m_rightBtn = Fumbo::UI::Button({1210, 290, 44, 80});
  m_rightBtn.ApplyStyle(btnstyle);
  m_rightBtn.Roundness(0.2f);
  m_rightBtn.AddText(">", SpaceB, 28, WHITE);

  RebuildCards();
  AppCore::currentScreen = AppCore::Screen::Home;
}

void MainMenu::Cleanup() {
  for (auto &e : m_cards)
    if (e.coverTex.id != 0)
      UnloadTexture(e.coverTex);
  m_cards.clear();
}

void MainMenu::RebuildCards() {
  // Hapus tekstur lama sebelum membangun ulang
  for (auto &e : m_cards)
    if (e.coverTex.id != 0)
      UnloadTexture(e.coverTex);
  m_cards.clear();

  auto &playlists = AppState::Instance().playlists;
  m_lastPlaylistCount = (int)playlists.size();

  float startX = CONTENT_X + 70.0f;
  float startY =
      CONTENT_Y + 50.0f; // Shifted down to make room for header title

  for (size_t i = 0; i < playlists.size(); ++i) {
    const auto &pl = playlists[i];
    int localIndex = i % 8;
    int row = localIndex / 4;
    int col = localIndex % 4;

    float cardX = startX + col * (CARD_W + CARD_GAP);
    float cardY = startY + row * (CARD_H + CARD_GAP);

    // Muat tekstur sampul ambil dari coverPath playlist terlebih dahulu lalu
    // dari lagu pertama yang punya cover atau gambar placeholder
    Texture2D tex{};
    bool loaded = false;
    if (!pl.coverPath.empty()) {
      FILE *f = fopen(pl.coverPath.c_str(), "rb");
      if (f) {
        fclose(f);
        Image img = LoadImage(pl.coverPath.c_str());
        if (img.data) {
          tex = LoadTextureFromImage(img);
          UnloadImage(img);
          loaded = true;
        }
      }
    }
    if (!loaded) {
      for (const auto &t : pl.tracks) {
        if (!t.coverArtPath.empty()) {
          FILE *f = fopen(t.coverArtPath.c_str(), "rb");
          if (f) {
            fclose(f);
            Image img = LoadImage(t.coverArtPath.c_str());
            if (img.data) {
              tex = LoadTextureFromImage(img);
              UnloadImage(img);
              loaded = true;
              break;
            }
          }
        }
      }
    }
    if (!loaded)
      tex = Fumbo::Assets::LoadTexture("assets/images/placeholder.png");

    Rectangle bounds{cardX, cardY, CARD_W, CARD_H};
    CardEntry entry;
    entry.coverTex = tex;
    entry.playlistId = pl.id;
    entry.card = PlaylistCard(bounds, pl, tex);
    m_cards.push_back(std::move(entry));
  }
}

void MainMenu::Update() {
  // Bangun ulang kartu jika jumlah playlist berubah
  if ((int)AppState::Instance().playlists.size() != m_lastPlaylistCount) {
    RebuildCards();
    int totalPlaylists = (int)AppState::Instance().playlists.size();
    int maxPages = (totalPlaylists + 7) / 8;
    if (m_currentPage >= maxPages) {
      m_currentPage = (maxPages > 0) ? (maxPages - 1) : 0;
      RebuildCards();
    }
  }

  // Animasi transisi halaman
  if (m_transitionProgress < 1.0f) {
    m_transitionProgress += GetFrameTime() * 2.5f;
    if (m_transitionProgress > 1.0f) {
      m_transitionProgress = 1.0f;
    }
  }

  // Tombol tambah playlist baru
  if (m_plusBtn.IsPressed()) {
    AppCore::currentScreen = AppCore::Screen::Home;
    Fumbo::Instance().ChangeState(std::make_shared<AddPlaylist>());
  }

  int totalPlaylists = (int)AppState::Instance().playlists.size();

  // Navigasi halaman dengan tombol Left/Right
  if (m_transitionProgress >= 1.0f) {
    if (m_currentPage > 0 && m_leftBtn.IsPressed()) {
      m_prevPage = m_currentPage;
      m_currentPage--;
      m_transitionProgress = 0.0f;
      m_transitionDirection = -1;
    }
    if ((m_currentPage + 1) * 8 < totalPlaylists && m_rightBtn.IsPressed()) {
      m_prevPage = m_currentPage;
      m_currentPage++;
      m_transitionProgress = 0.0f;
      m_transitionDirection = 1;
    }
  }

  // Periksa klik pada setiap kartu playlist (hanya jika tidak sedang transisi)
  if (m_transitionProgress >= 1.0f) {
    for (auto &entry : m_cards) {
      int cardIndex = -1;
      for (size_t i = 0; i < m_cards.size(); ++i) {
        if (m_cards[i].playlistId == entry.playlistId) {
          cardIndex = (int)i;
          break;
        }
      }
      if (cardIndex == -1 || cardIndex / 8 != m_currentPage)
        continue;

      entry.card.Update();
      if (entry.card.IsEditClicked()) {
        AppCore::currentScreen = AppCore::Screen::Home;
        Fumbo::Instance().ChangeState(
            std::make_shared<AddPlaylist>(entry.playlistId));
        return;
      }
      if (entry.card.IsClicked()) {
        for (const auto &pl : AppState::Instance().playlists) {
          if (pl.id == entry.playlistId) {
            // Jika playlist yang ditekan berbeda dengan playlist aktif saat ini
            if (AppState::Instance().activePlaylistId != pl.id) {
              AppState::Instance().PlayPlaylist(pl);
            }
            AppCore::currentScreen = AppCore::Screen::Play;
            Fumbo::Instance().ChangeState(std::make_shared<PlayMusic>());
            return;
          }
        }
      }
    }
  }
}

void MainMenu::DrawClean() {}

void MainMenu::DrawDirty() {
  // Latar belakang
  Fumbo::Graphic2D::DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                                  currentTheme.prim1);

  // Header seksi
  Fumbo::Graphic2D::DrawText(Lang::Get("Playlist Kamu", "Your Playlists"),
                             {CONTENT_X + 70.0f, CONTENT_Y + 15.0f}, SpaceB, 28,
                             currentTheme.second1);

  // Tombol buat playlist baru
  m_plusBtn.Draw();

  // Tombol navigasi Left/Right
  int totalPlaylists = (int)AppState::Instance().playlists.size();
  if (m_currentPage > 0) {
    m_leftBtn.Draw();
  }
  if ((m_currentPage + 1) * 8 < totalPlaylists) {
    m_rightBtn.Draw();
  }

  // Kartu playlist dengan efek animasi geser
  float t = m_transitionProgress;
  float ease = t * t * (3.0f - 2.0f * t);

  for (size_t i = 0; i < m_cards.size(); ++i) {
    int p = (int)i / 8;
    if (p == m_currentPage) {
      float offsetX = 0.0f;
      if (m_transitionProgress < 1.0f) {
        offsetX = -m_transitionDirection * 1080.0f * (1.0f - ease);
      }
      m_cards[i].card.Draw(offsetX);
    } else if (p == m_prevPage && m_transitionProgress < 1.0f) {
      float offsetX = -m_transitionDirection * 1080.0f * ease;
      m_cards[i].card.Draw(offsetX);
    }
  }

  // Pesan jika belum ada playlist
  if (m_cards.empty()) {
    Fumbo::Graphic2D::DrawText(
        Lang::Get("Belum ada playlist, tekan  +  untuk membuat",
                  "No playlists yet, press  +  to create one"),
        {CONTENT_X + 70.0f, CONTENT_Y + 100.0f}, SpaceB, 20,
        {120, 125, 140, 255});
  }
}
