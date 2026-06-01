#pragma once
#include <string>
#include <vector>

struct Track {
  std::string filePath{}; // Path absolut ke file audio
  std::string title{};    // Dari metadata VLC (atau nama file sebagai cadangan)
  std::string artist{};   // Dari metadata VLC
  std::string album{};    // Dari metadata VLC
  std::string coverArtPath{}; // Path ke cache sampul seni (atau kosong)
  long long durationMs{0};    // Durasi dalam milidetik dari VLC
};

struct Playlist {
  int id{0};
  std::string name{};
  std::string coverPath{}; // Path gambar sampul playlist (opsional)
  std::string description{};
  std::vector<Track> tracks{};
};
