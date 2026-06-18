#pragma once
#include "dataStructures.hpp"
#include "playlistData.hpp"
#include <algorithm>
#include <queue>
#include <stack>
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
          // [CIRCULAR LINKED LIST] Bangun ulang antrean melingkar dari tracks
          playQueue.clear();
          for (const auto &t : p.tracks)
            playQueue.pushBack(t);
          if (playQueue.empty()) {
            isPlaying = false;
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

  // [CIRCULAR LINKED LIST] Antrean pemutaran lagu menggunakan senarai melingkar
  // Menggantikan std::deque<Track> + currentQueueIndex.
  // Node terakhir menunjuk ke node pertama, sehingga navigasi Next/Prev
  // secara otomatis berputar tanpa perlu pengecekan batas indeks.
  CircularLinkedList<Track> playQueue{};

  // [STACK] Riwayat pemutaran lagu menggunakan Stack (LIFO)
  // Setiap kali lagu berpindah ke lagu berikutnya, lagu yang baru saja diputar
  // di-push ke stack ini. Saat tombol Prev ditekan, lagu teratas di-pop
  // untuk kembali ke lagu yang benar-benar baru didengar.
  std::stack<Track> playbackHistory{};

  // [QUEUE] Antrean prioritas "Play Next" menggunakan Queue (FIFO)
  // Pengguna dapat menambahkan lagu ke antrean ini agar diputar setelah lagu
  // saat ini selesai. Lagu di antrean ini diambil terlebih dahulu (FIFO)
  // sebelum berpindah ke lagu berikutnya di playQueue.
  std::queue<Track> customNextQueue{};

  bool isPlaying{false};

  bool HasActivePlaylist() const {
    return activePlaylistId >= 0 && !playQueue.empty();
  }

  // Isi antrean dari playlist dan mulai dari lagu pertama.
  void PlayPlaylist(const Playlist &pl);

  // [CIRCULAR LINKED LIST] Kembalikan lagu saat ini dari senarai melingkar
  const Track *CurrentTrack() const { return playQueue.getCurrent(); }

  void NextTrack();
  void PrevTrack();

  // [QUEUE] Tambahkan lagu ke antrean prioritas "Play Next"
  void AddToNextQueue(const Track &t) { customNextQueue.push(t); }

  // [STACK] Bersihkan riwayat pemutaran
  void ClearPlaybackHistory() {
    while (!playbackHistory.empty())
      playbackHistory.pop();
  }

  // [CIRCULAR LINKED LIST] Hitung posisi current relatif terhadap head
  int GetCurrentQueueIndex() const { return playQueue.getCurrentIndex(); }

  // [CIRCULAR LINKED LIST] Kembalikan ukuran antrean pemutaran
  int GetQueueSize() const { return playQueue.size(); }

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

  // Persistensi data ke dua file terpisah di dalam subfolder user/
  static std::string UserDir();             // Buat & kembalikan folder user/
  static std::string PlaylistsFilePath();   // user/playlists.json
  static std::string PreferencesFilePath(); // user/preferences.json
  void SaveToFile() const;   // Simpan playlist & preferensi ke file masing-masing
  void LoadFromFile();       // Muat playlist & preferensi dari file masing-masing

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
