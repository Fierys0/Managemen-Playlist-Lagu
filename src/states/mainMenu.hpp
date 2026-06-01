#pragma once
#include "../core/appState.hpp"
#include "fumbo.hpp"
#include "playlistCard.hpp"
#include <utility> // pair
#include <vector>

class MainMenu : public IGameState {
public:
  void Init() override;
  void Cleanup() override;
  void Update() override;
  void DrawClean() override;
  void DrawDirty() override;

private:
  Fumbo::UI::Button m_plusBtn;
  Fumbo::UI::Button m_leftBtn;
  Fumbo::UI::Button m_rightBtn;

  struct CardEntry {
    PlaylistCard card;
    Texture2D coverTex{};
    int playlistId{-1};
  };
  std::vector<CardEntry> m_cards;

  void RebuildCards();
  int m_lastPlaylistCount{-1};

  // Pagination & Slide transition animation variables
  int m_currentPage{0};
  int m_prevPage{0};
  float m_transitionProgress{1.0f}; // 1.0f means transition is finished/idle
  int m_transitionDirection{0};     // -1 for left (prev page), 1 for right (next page)
};
