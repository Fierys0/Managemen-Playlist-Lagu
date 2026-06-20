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
  // Kontrol
  Fumbo::UI::Button m_playPauseBtn;
  Fumbo::UI::Button m_nextBtn;
  Fumbo::UI::Button m_prevBtn;

  // Gambar sampul
  Texture2D m_coverTex{};

  // Slider kemajuan dan volume
  Fumbo::UI::Slider m_progressSlider;
  Fumbo::UI::Slider m_volumeSlider;

  // Tekstur tombol
  Texture2D m_playIcon{};
  Texture2D m_pauseIcon{};
  Texture2D m_nextIcon{};
  Texture2D m_prevIcon{};

  // Senarai berantai melingkar Deteksi perubahan lagu menggunakan pointer ke Track
  // Alih alih menyimpan indeks yang tidak lagi relevan dengan senarai melingkar
  // kita simpan pointer ke Track yang sedang ditampilkan untuk mendeteksi
  // kapan lagu berubah dan perlu dimuat ulang
  const Track *m_loadedTrackPtr{nullptr};

  // Informasi lagu yang disimpan dalam cache untuk ditampilkan
  std::string m_displayTitle{};
  std::string m_displayArtist{};
  std::string m_displayAlbum{};

  void LoadCurrentTrack();

  // Sudut putaran sampul animasi
  float m_coverAngle{0};

  // Aksen warna dari sampul yang dihitung sebelumnya
  Color m_accentColor{80, 160, 255, 255};
};
