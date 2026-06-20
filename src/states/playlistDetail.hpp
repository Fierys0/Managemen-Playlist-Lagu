#pragma once
#include "../core/appState.hpp"
#include "../core/playlistData.hpp"
#include "fumbo.hpp"

#include <string>
#include <vector>

class PlaylistDetail : public IGameState {
public:
  PlaylistDetail(int playlistId);
  void Init() override;
  void Cleanup() override;
  void Update() override;
  void DrawClean() override;
  void DrawDirty() override;

private:
  int m_playlistId{-1};

  // Informasi playlist yang disimpan dalam memori
  std::string m_playlistName{};
  std::string m_playlistDesc{};
  std::string m_coverPath{};
  int m_trackCount{0};
  long long m_totalDurationMs{0};
  std::vector<Track> m_tracks;

  // Tekstur sampul dari CoverCache bukan milik objek ini
  Texture2D m_coverTex{};

  // Tombol tombol antarmuka
  Fumbo::UI::Button m_playBtn;
  Fumbo::UI::Button m_shuffleBtn;
  Fumbo::UI::Button m_editBtn;
  Fumbo::UI::Button m_backBtn;

  // Tekstur ikon
  Texture2D m_playIcon{};
  Texture2D m_shuffleIcon{};

  // Pengguliran daftar lagu
  float m_scrollY{0};

  // Status hover
  int m_hoveredTrack{-1};

  // Pointer ke data playlist untuk dikirim ke metode putar AppState
  const Playlist *FindPlaylist() const;
};
