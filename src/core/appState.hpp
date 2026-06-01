#pragma once
#include "playlistData.hpp"
#include <algorithm>
#include <deque>
#include <string>
#include <vector>

// Singleton yang menyimpan semua status aplikasi yang digunakan bersama antar
// layar.
class AppState {
public:
  static AppState &Instance() {
    static AppState s;
    return s;
  }

  // Daftar playlist
  std::vector<Playlist> playlists{};
  int nextPlaylistId{1};

  void AddPlaylist(Playlist pl) {
    pl.id = nextPlaylistId++;
    playlists.push_back(std::move(pl));
    SaveToFile();
  }

  void UpdatePlaylist(int id, Playlist pl) {
    for (auto &p : playlists) {
      if (p.id == id) {
        p.name = pl.name;
        p.coverPath = pl.coverPath;
        p.tracks = pl.tracks;

        // Jika playlist yang diupdate sedang aktif diputar, perbarui antrean
        if (activePlaylistId == id) {
          queue.clear();
          for (const auto &t : p.tracks)
            queue.push_back(t);
          if (queue.empty()) {
            currentQueueIndex = -1;
            isPlaying = false;
          } else if (currentQueueIndex >= (int)queue.size()) {
            currentQueueIndex = 0;
          }
        }
        break;
      }
    }
    SaveToFile();
  }

  void RemovePlaylist(int id) {
    playlists.erase(
        std::remove_if(playlists.begin(), playlists.end(),
                       [id](const Playlist &p) { return p.id == id; }),
        playlists.end());
    SaveToFile();
  }

  // Status pemutaran aktif
  int activePlaylistId{-1}; // -1 berarti tidak ada
  std::deque<Track> queue{};
  int currentQueueIndex{-1};
  bool isPlaying{false};

  bool HasActivePlaylist() const {
    return activePlaylistId >= 0 && !queue.empty();
  }

  // Isi antrean dari playlist dan mulai dari lagu pertama.
  void PlayPlaylist(const Playlist &pl);

  // Kembalikan lagu saat ini atau nullptr.
  const Track *CurrentTrack() const {
    if (currentQueueIndex < 0 || currentQueueIndex >= (int)queue.size())
      return nullptr;
    return &queue[currentQueueIndex];
  }

  void NextTrack();
  void PrevTrack();

  // Playback state variables
  std::string loadedTrackPath{""};
  bool isAudioLoaded{false};

  // Playback control methods
  void UpdateMusicPlayback();
  void PlayCurrentTrack();
  void StopMusic();
  void TogglePlayPause();

  // Pengaturan
  bool trailsEnabled{true};
  std::string language{"id"};
  std::string themeName{"dark"};

  // Persistensi data ke file
  static std::string SaveFilePath();
  void SaveToFile() const;
  void LoadFromFile();

private:
  AppState() = default;

  bool m_isFadingOutTransition{false};
  Track m_nextTrackToPlay{};
};

namespace Lang {
inline std::string Get(const std::string &idKey, const std::string &enKey) {
  return (AppState::Instance().language == "en") ? enKey : idKey;
}
} // namespace Lang
