#pragma once
#include "../core/playlistData.hpp"
#include "fumbo.hpp"
#include <string>

class PlaylistCard {
public:
  PlaylistCard() = default;
  PlaylistCard(Rectangle bounds, const Playlist &pl, Texture2D coverTex);

  // Make PlaylistCard move-only because Fumbo::UI::Button has deleted copy operations
  PlaylistCard(const PlaylistCard&) = delete;
  PlaylistCard& operator=(const PlaylistCard&) = delete;
  PlaylistCard(PlaylistCard&&) noexcept = default;
  PlaylistCard& operator=(PlaylistCard&&) noexcept = default;

  void Update();
  void Draw(float offsetX = 0.0f);
  bool IsClicked() const;
  bool IsEditClicked() const;

  int GetPlaylistId() const { return m_playlistId; }

private:
  Rectangle m_bounds{};
  Texture2D m_image{};
  std::string m_title{};
  int m_playlistId{-1};
  int m_trackCount{0};
  Fumbo::UI::Button m_editBtn{};
  int m_lastThemeId{-1};
};
