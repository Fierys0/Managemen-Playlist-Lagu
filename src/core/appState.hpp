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

  // [ALPRO] [Standard Template Library (STL): Vector dan List]
  // Menyimpan daftar playlist menggunakan std::vector
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
        p.description = pl.description;
        p.tracks = pl.tracks;

        // Jika playlist yang diupdate sedang aktif diputar, perbarui antrean
        if (activePlaylistId == id) {
          // [CIRCULAR LINKED LIST] Load ulang antrean melingkar dari tracks
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
  int activePlaylistId{-1}; // Nilai minus satu berarti tidak ada

  // [CIRCULAR LINKED LIST] Antrean pemutaran lagu menggunakan senarai melingkar
  // Menggantikan std::deque untuk melacak indeks antrean
  // Node terakhir menunjuk ke node pertama sehingga navigasi berikutnya dan
  // sebelumnya secara otomatis berputar tanpa perlu pengecekan batas indeks
  CircularLinkedList<Track> playQueue{};

  // [STACK] Riwayat pemutaran lagu menggunakan Stack LIFO
  // Setiap kali lagu berpindah ke lagu berikutnya lagu yang baru saja diputar
  // dimasukkan ke tumpukan ini. Saat tombol sebelumnya ditekan lagu teratas
  // dikeluarkan untuk kembali ke lagu yang baru didengar
  std::stack<Track> playbackHistory{};

  // [QUEUE] Antrean prioritas putar nanti menggunakan Queue FIFO
  // Pengguna dapat menambahkan lagu ke antrean ini agar diputar setelah lagu
  // saat ini selesai. Lagu di antrean ini diambil terlebih dahulu
  // sebelum berpindah ke lagu berikutnya di playQueue
  std::queue<Track> customNextQueue{};

  bool isPlaying{false};

  bool HasActivePlaylist() const {
    return activePlaylistId >= 0 && !playQueue.empty();
  }

  // Isi antrean dari playlist dan mulai dari lagu pertama.
  void PlayPlaylist(const Playlist &pl);

  // Isi antrean dari playlist dalam urutan acak dan mulai putar.
  void PlayPlaylistShuffled(const Playlist &pl);

  // Isi antrean dari playlist dan mulai dari lagu ke indeks trackIndex.
  void PlayPlaylistFromTrack(const Playlist &pl, int trackIndex);

  // [ALPRO] [REFERENCES & POINTER]
  // Mengembalikan pointer ke objek Track yang sedang aktif diputar
  const Track *CurrentTrack() const { return playQueue.getCurrent(); }

  void NextTrack();
  void PrevTrack();

  // [QUEUE] Tambahkan lagu ke antrean prioritas putar nanti
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

  // Variabel status pemutaran
  std::string loadedTrackPath{""};
  bool isAudioLoaded{false};

  // Metode kontrol pemutaran
  void UpdateMusicPlayback();
  void PlayCurrentTrack();
  void StopMusic();
  void TogglePlayPause();

  // Pengaturan
  bool trailsEnabled{true};
  std::string language{"id"};
  std::string themeName{"dark"};

  // Persistensi data ke dua file terpisah di dalam subfolder user
  static std::string UserDir();             // Buat dan kembalikan folder user
  static std::string PlaylistsFilePath();   // Jalur ke file playlists.json
  static std::string PreferencesFilePath(); // Jalur ke file preferences.json
  void
  SaveToFile() const;  // Simpan playlist dan preferensi ke file masing masing
  void LoadFromFile(); // Muat playlist dan preferensi dari file masing masing

private:
  AppState() = default;

  bool m_isFadingOutTransition{false};
  Track m_nextTrackToPlay{};
};

// [ALPRO] [NAMESPACE]
// Mendefinisikan namespace Lang untuk fungsi lokalisasi bahasa
namespace Lang {
// [ALPRO] [DEFAULT ARGUMENT & INLINE FUNCTION]
// Fungsi inline Get untuk mereturn terjemahan string berdasarkan bahasa aktif
inline std::string Get(const std::string &idKey, const std::string &enKey) {
  return (AppState::Instance().language == "en") ? enKey : idKey;
}
} // namespace Lang
