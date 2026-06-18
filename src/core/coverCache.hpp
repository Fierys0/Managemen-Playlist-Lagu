#pragma once
#include "fumbo.hpp"
#include <string>
#include <unordered_map>

// =============================================================================
// [HASH MAP] Cache Tekstur Sampul Album menggunakan Hash Map
// =============================================================================
// Menggunakan std::unordered_map atau Tabel Hash untuk menyimpan tekstur sampul
// album yang sudah dimuat dari disk. Ketika sebuah path gambar diminta
// cache akan memeriksa apakah tekstur sudah ada di dalam tabel hash
// Jika sudah ada kembalikan tekstur langsung dengan kompleksitas O 1
// Jika belum ada muat dari disk simpan ke tabel hash lalu kembalikan
// Ini menghindari pemuatan ulang file gambar yang sama secara berulang kali
// yang sebelumnya menyebabkan lag pada layar pencarian dan menu utama
// =============================================================================
class CoverCache {
public:
  // [HASH MAP] Singleton instance hanya satu cache untuk seluruh aplikasi
  static CoverCache &Instance() {
    static CoverCache instance;
    return instance;
  }

  // [HASH MAP] Ambil tekstur dari cache berdasarkan path file
  // Jika belum ada di tabel hash muat dari disk dan simpan ke cache
  // Kompleksitas rata rata O 1 berkat fungsi hash pada std::unordered_map
  Texture2D Get(const std::string &path) {
    if (path.empty())
      return {};

    // [HASH MAP] Cari di tabel hash menggunakan path sebagai kunci
    auto it = m_cache.find(path);
    if (it != m_cache.end()) {
      // Tekstur ditemukan kembalikan langsung
      return it->second;
    }

    // Tekstur tidak ditemukan muat dari disk
    Texture2D tex{};
    FILE *f = fopen(path.c_str(), "rb");
    if (f) {
      fclose(f);
      Image img = LoadImage(path.c_str());
      if (img.data) {
        tex = LoadTextureFromImage(img);
        UnloadImage(img);
      }
    }

    // [HASH MAP] Simpan tekstur ke tabel hash dengan path sebagai kunci
    if (tex.id != 0) {
      m_cache[path] = tex;
    }
    return tex;
  }

  // [HASH MAP] Ambil tekstur placeholder dari cache
  Texture2D GetPlaceholder(const std::string &placeholderPath) {
    return Get(placeholderPath);
  }

  // [HASH MAP] Hapus satu entri dari tabel hash
  // Digunakan saat gambar sampul diganti oleh pengguna
  void Invalidate(const std::string &path) {
    auto it = m_cache.find(path);
    if (it != m_cache.end()) {
      if (it->second.id != 0)
        UnloadTexture(it->second);
      m_cache.erase(it);
    }
  }

  // [HASH MAP] Bersihkan seluruh tabel hash dan bebaskan semua tekstur
  void Clear() {
    for (auto &pair : m_cache) {
      if (pair.second.id != 0)
        UnloadTexture(pair.second);
    }
    m_cache.clear();
  }

  // [HASH MAP] Akses jumlah entri di dalam cache
  size_t Size() const { return m_cache.size(); }

private:
  CoverCache() = default;
  ~CoverCache() { Clear(); }

  // [HASH MAP] internal tabel hash kunci adalah path file string dan nilai adalah Texture2D
  // std::unordered_map menggunakan fungsi hash untuk memetakan kunci ke bucket
  // sehingga operasi pencarian penyisipan dan penghapusan berjalan O 1 rata rata
  std::unordered_map<std::string, Texture2D> m_cache;
};
