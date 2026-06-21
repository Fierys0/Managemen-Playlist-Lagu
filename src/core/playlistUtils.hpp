#pragma once
#include "playlistData.hpp"
#include <string>
#include <vector>
#include <list>

// [ALPRO] [NAMESPACE]
// Mengelompokkan semua fungsi utilitas dalam namespace tersendiri untuk kerapian kode
namespace PlaylistUtils {

    // [ALPRO] [STRUCT]
    // Mendefinisikan tipe data bentukan (struktur) untuk menampung statistik lagu
    struct TrackStats {
        std::string artist{};
        int playCount{0};
        long long totalDurationMs{0};
    };

    // [ALPRO] [DEFAULT ARGUMENT & INLINE FUNCTION]
    // Fungsi inline dengan nilai argumen default (prefix)
    inline std::string formatLog(const std::string& message, const std::string& prefix = "[INFO]") {
        return prefix + " " + message;
    }

    // [ALPRO] [REFERENCES & POINTER]
    // Menerima pointer ke struct TrackStats dan reference (const Track&) untuk memproses data lagu
    void updateStats(TrackStats* stats, const Track& track);

    // [ALPRO] [FUNCTION OVERLOADING & FUNCTION TEMPLATE]
    // 1. Function Overloading: Nama fungsi sama, tipe parameter berbeda
    // Menghitung total lagu dalam std::vector
    int countTotalTracks(const std::vector<Track>& tracks);
    // Menghitung total lagu dalam std::list
    int countTotalTracks(const std::list<Track>& tracks);

    // 2. Function Template: Fungsi generik untuk memfilter elemen dalam kontainer STL
    template <typename Container, typename Predicate>
    auto filterTracks(const Container& container, Predicate pred) -> std::vector<typename Container::value_type> {
        std::vector<typename Container::value_type> result;
        // [ALPRO] [ITERATOR]
        // Menggunakan iterator dari kontainer STL untuk menelusuri elemen
        for (auto it = container.begin(); it != container.end(); ++it) {
            if (pred(*it)) {
                result.push_back(*it);
            }
        }
        return result;
    }

    // [ALPRO] [Exception Handling dalam C++]
    // Melakukan validasi track dan berpotensi melempar exception jika data tidak valid
    void validateTrack(const Track& track);

    // [ALPRO] [Standard Template Library (STL): Vector dan List]
    // Menggunakan std::list dan std::vector untuk pemrosesan data
    std::list<Track> convertToSTLList(const Playlist& playlist);

    // [ALPRO] [Sort, find, dan count]
    // Mengurutkan lagu berdasarkan durasi (std::sort)
    void sortTracksByDuration(std::vector<Track>& tracks);
    
    // Mencari lagu berdasarkan judul (std::find_if)
    bool findTrackByTitle(const std::vector<Track>& tracks, const std::string& title, Track& foundTrack);
    
    // Menghitung jumlah lagu dari artis tertentu (std::count_if)
    int countTracksByArtist(const std::vector<Track>& tracks, const std::string& artist);
}
