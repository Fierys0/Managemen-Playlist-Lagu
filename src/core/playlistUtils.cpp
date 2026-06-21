#include "playlistUtils.hpp"
#include "fumbo.hpp"
#include <algorithm>
#include <stdexcept>

namespace PlaylistUtils {

    // [ALPRO] [REFERENCES & POINTER]
    // Implementasi penerimaan parameter pointer (stats) dan reference (track)
    void updateStats(TrackStats* stats, const Track& track) {
        if (stats == nullptr) {
            // [ALPRO] [Exception Handling dalam C++]
            // Melempar exception std::invalid_argument jika pointer bernilai nullptr
            throw std::invalid_argument("Pointer ke TrackStats tidak boleh NULL!");
        }
        stats->totalDurationMs += track.durationMs;
        stats->playCount++;
    }

    // [ALPRO] [FUNCTION OVERLOADING & FUNCTION TEMPLATE]
    // Overload 1: Menghitung jumlah elemen pada std::vector<Track>
    int countTotalTracks(const std::vector<Track>& tracks) {
        return static_cast<int>(tracks.size());
    }

    // Overload 2: Menghitung jumlah elemen pada std::list<Track>
    int countTotalTracks(const std::list<Track>& tracks) {
        return static_cast<int>(tracks.size());
    }

    // [ALPRO] [Exception Handling dalam C++]
    // Mengecek apakah track valid. Melempar exception jika ada field yang salah.
    void validateTrack(const Track& track) {
        if (track.title.empty()) {
            throw std::runtime_error("Validasi Gagal: Judul lagu kosong!");
        }
        if (track.durationMs < 0) {
            throw std::out_of_range("Validasi Gagal: Durasi lagu tidak boleh negatif!");
        }
    }

    // [ALPRO] [Standard Template Library (STL): Vector dan List]
    // Mengonversi data dari DoublyLinkedList kustom ke kontainer std::list dari STL
    std::list<Track> convertToSTLList(const Playlist& playlist) {
        std::list<Track> stlList;
        for (const auto& t : playlist.tracks) {
            stlList.push_back(t);
        }
        return stlList;
    }

    // [ALPRO] [Sort, find, dan count]
    // Mengurutkan tracks menggunakan std::sort dan iterator STL
    void sortTracksByDuration(std::vector<Track>& tracks) {
        // [ALPRO] [ITERATOR]
        // tracks.begin() dan tracks.end() merupakan iterator dari std::vector
        std::sort(tracks.begin(), tracks.end(), [](const Track& a, const Track& b) {
            return a.durationMs < b.durationMs;
        });
    }

    // [ALPRO] [Sort, find, dan count]
    // Mencari track berdasarkan judul menggunakan std::find_if dan iterator STL
    bool findTrackByTitle(const std::vector<Track>& tracks, const std::string& title, Track& foundTrack) {
        // [ALPRO] [ITERATOR]
        // std::find_if menggunakan iterator untuk melakukan traversal pencarian
        auto it = std::find_if(tracks.begin(), tracks.end(), [&title](const Track& t) {
            return t.title == title;
        });

        if (it != tracks.end()) {
            foundTrack = *it;
            return true;
        }
        return false;
    }

    // [ALPRO] [Sort, find, dan count]
    // Menghitung jumlah track dari artis tertentu menggunakan std::count_if dan iterator STL
    int countTracksByArtist(const std::vector<Track>& tracks, const std::string& artist) {
        // [ALPRO] [ITERATOR]
        // std::count_if menerima iterator untuk menghitung kecocokan elemen
        return static_cast<int>(std::count_if(tracks.begin(), tracks.end(), [&artist](const Track& t) {
            return t.artist == artist;
        }));
    }
}
