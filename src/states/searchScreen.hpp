#pragma once
#include "../core/appState.hpp"
#include "fumbo.hpp"

#include <string>
#include <vector>

// Layar pencarian playlist: menampilkan kotak teks di bagian atas dan daftar
// playlist yang cocok dengan kueri secara real-time.
class SearchScreen : public IGameState {
public:
  void Init() override;
  void Cleanup() override;
  void Update() override;
  void DrawClean() override;
  void DrawDirty() override;

private:
  // Kotak pencarian
  Fumbo::UI::Textbox m_searchBox;
  Fumbo::UI::TextboxConfig m_tbConfig;

  // Tombol hapus pencarian
  Fumbo::UI::Button m_clearBtn;

  // Indeks playlist yang sesuai dengan kueri
  std::vector<int> m_filteredIndices;

  // Teks terakhir yang digunakan untuk membangun filteredIndices
  std::string m_lastQuery;

  // Gulir hasil
  float m_scrollY{0};

  // [HASH MAP] Tekstur sampul dari cache — hanya menyimpan Texture2D yang didapat
  // dari CoverCache, bukan memuat sendiri dari disk.
  std::vector<Texture2D> m_resultCovers;

  // Bangun ulang m_filteredIndices berdasarkan kueri saat ini
  void RebuildResults();
};
