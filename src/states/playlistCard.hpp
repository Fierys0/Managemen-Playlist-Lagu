#pragma once
#include "../core/playlistData.hpp"
#include "fumbo.hpp"
#include <string>

class PlaylistCard {
public:
  PlaylistCard() = default;
  PlaylistCard(Rectangle bounds, const Playlist &pl, Texture2D coverTex);

  // Jadikan PlaylistCard hanya pindah karena Fumbo::UI::Button menghapus operasi salin
  PlaylistCard(const PlaylistCard&) = delete;
  PlaylistCard& operator=(const PlaylistCard&) = delete;
  PlaylistCard(PlaylistCard&&) noexcept = default;
  PlaylistCard& operator=(PlaylistCard&&) noexcept = default;

  void Update();
  void Draw(float offsetX = 0);
  bool IsClicked() const;

  int GetPlaylistId() const { return m_playlistId; }

private:
  Rectangle m_bounds{};
  Texture2D m_image{};
  std::string m_title{};
  int m_playlistId{-1};
  int m_trackCount{0};
};
