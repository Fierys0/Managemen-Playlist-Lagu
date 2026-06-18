#pragma once
#include "fumbo.hpp"
#include <string>
#include <unordered_map>

// =============================================================================
// [HASH MAP] CoverCache — Cache Tekstur Sampul Album menggunakan Hash Map
// =============================================================================
// Menggunakan std::unordered_map (Hash Map) untuk menyimpan tekstur sampul
// album yang sudah dimuat dari disk. Ketika sebuah path gambar diminta,
// cache akan memeriksa apakah tekstur sudah ada di dalam hash map:
// - Jika sudah ada (cache hit): kembalikan tekstur langsung, O(1).
// - Jika belum ada (cache miss): muat dari disk, simpan ke hash map, lalu
//   kembalikan.
// Ini menghindari pemuatan ulang file gambar yang sama berulang kali,
// yang sebelumnya menyebabkan lag pada layar pencarian dan menu utama.
// =============================================================================
class CoverCache {
public:
  // [HASH MAP] Singleton instance — hanya satu cache untuk seluruh aplikasi
  static CoverCache &Instance() {
    static CoverCache instance;
    return instance;
  }

  // [HASH MAP] Get — Ambil tekstur dari cache berdasarkan path file
  // Jika belum ada di hash map, muat dari disk dan simpan ke cache.
  // Kompleksitas rata-rata: O(1) berkat fungsi hash pada std::unordered_map
  Texture2D Get(const std::string &path) {
    if (path.empty())
      return {};

    // [HASH MAP] Cari di hash map menggunakan path sebagai key
    auto it = m_cache.find(path);
    if (it != m_cache.end()) {
      // Cache hit: tekstur sudah ada, kembalikan langsung
      return it->second;
    }

    // Cache miss: muat tekstur dari disk
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

    // [HASH MAP] Simpan tekstur ke hash map dengan path sebagai key
    if (tex.id != 0) {
      m_cache[path] = tex;
    }
    return tex;
  }

  // [HASH MAP] GetPlaceholder — Ambil tekstur placeholder dari cache
  Texture2D GetPlaceholder(const std::string &placeholderPath) {
    return Get(placeholderPath);
  }

  // [HASH MAP] Invalidate — Hapus satu entri dari hash map
  // Digunakan saat gambar sampul diganti oleh pengguna
  void Invalidate(const std::string &path) {
    auto it = m_cache.find(path);
    if (it != m_cache.end()) {
      if (it->second.id != 0)
        UnloadTexture(it->second);
      m_cache.erase(it);
    }
  }

  // [HASH MAP] Clear — Bersihkan seluruh hash map dan bebaskan semua tekstur
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

  // [HASH MAP] Hash map internal: key = path file (string), value = Texture2D
  // std::unordered_map menggunakan fungsi hash untuk memetakan key ke bucket,
  // sehingga operasi pencarian, penyisipan, dan penghapusan berjalan O(1) rata-rata.
  std::unordered_map<std::string, Texture2D> m_cache;
};
