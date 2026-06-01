#pragma once
#include "../core/appState.hpp"
#include "fumbo.hpp"

#include <string>

class PlayMusic : public IGameState {
public:
  void Init() override;
  void Cleanup() override;
  void Update() override;
  void DrawClean() override;
  void DrawDirty() override;

private:
  // Controls
  Fumbo::UI::Button m_playPauseBtn;
  Fumbo::UI::Button m_nextBtn;
  Fumbo::UI::Button m_prevBtn;

  // Cover art
  Texture2D m_coverTex{};

  // Progress and volume sliders
  Fumbo::UI::Slider m_progressSlider;
  Fumbo::UI::Slider m_volumeSlider;

  // Button textures
  Texture2D m_playIcon{};
  Texture2D m_pauseIcon{};
  Texture2D m_nextIcon{};
  Texture2D m_prevIcon{};

  int m_loadedQueueIndex{-1};

  // Cached track info for display
  std::string m_displayTitle{};
  std::string m_displayArtist{};
  std::string m_displayAlbum{};

  void LoadCurrentTrack();

  // Animated cover spin angle
  float m_coverAngle{0.0f};

  // Color accent from cover (precomputed)
  Color m_accentColor{80, 160, 255, 255};
};
