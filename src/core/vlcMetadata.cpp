#include "vlcMetadata.hpp"
#include "fumbo.hpp"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <string>
#include <thread>
#include <vlc/vlc.h>

// Instance VLC global pada level modul
static libvlc_instance_t *s_vlc = nullptr;

void VlcMeta::Init() {
  if (s_vlc)
    return;
  // Sembunyikan output VLC dengan menonaktifkan video
  const char *args[] = {"--quiet", "--no-video"};
  s_vlc = libvlc_new(2, args);
}

void VlcMeta::Shutdown() {
  if (s_vlc) {
    libvlc_release(s_vlc);
    s_vlc = nullptr;
  }
}

// Ambil nama file tanpa ekstensi dari path lengkap
static std::string stemFromPath(const std::string &path) {
  size_t slash = path.find_last_of("/\\");
  std::string name =
      (slash == std::string::npos) ? path : path.substr(slash + 1);
  size_t dot = name.rfind('.');
  if (dot != std::string::npos)
    name = name.substr(0, dot);
  return name;
}

Track VlcMeta::GetTrackInfo(const std::string &filePath) {
  Track t;
  t.filePath = filePath;
  t.title = stemFromPath(filePath); // judul cadangan jika metadata tidak ada

  if (!s_vlc)
    return t;

  libvlc_media_t *m = libvlc_media_new_path(s_vlc, filePath.c_str());
  if (!m)
    return t;

  // Urai secara sinkron: metadata lokal + ambil sampul seni lokal
  libvlc_media_parse_with_options(
      m,
      (libvlc_media_parse_flag_t)(libvlc_media_parse_local |
                                  libvlc_media_fetch_local),
      5000 // batas waktu dalam milidetik
  );

  // Tunggu hingga parsing selesai (maks 5 detik)
  using namespace std::chrono;
  auto deadline = steady_clock::now() + seconds(5);
  while (steady_clock::now() < deadline) {
    libvlc_media_parsed_status_t st = libvlc_media_get_parsed_status(m);
    if (st == libvlc_media_parsed_status_done ||
        st == libvlc_media_parsed_status_failed ||
        st == libvlc_media_parsed_status_timeout) {
      break;
    }
    std::this_thread::sleep_for(milliseconds(50));
  }

  // Fungsi bantu untuk mengambil satu metadata
  auto getMeta = [&](libvlc_meta_t key) -> std::string {
    const char *v = libvlc_media_get_meta(m, key);
    return v ? std::string(v) : std::string{};
  };

  std::string title = getMeta(libvlc_meta_Title);
  if (!title.empty())
    t.title = title;

  t.artist = getMeta(libvlc_meta_Artist);
  if (t.artist.empty())
    t.artist = getMeta(libvlc_meta_AlbumArtist);

  t.album = getMeta(libvlc_meta_Album);

  // URL sampul seni: VLC mengembalikan URI file:// yang menunjuk ke cache
  // thumbnail-nya
  std::string artUrl = getMeta(libvlc_meta_ArtworkURL);
  if (!artUrl.empty()) {
    // Hapus awalan "file://" untuk mendapatkan path absolut sistem file
    if (artUrl.substr(0, 7) == "file://") {
      t.coverArtPath = artUrl.substr(7);
    } else {
      t.coverArtPath = artUrl;
    }
  }

  t.durationMs = libvlc_media_get_duration(m);

  libvlc_media_release(m);
  return t;
}

Texture2D VlcMeta::LoadCoverTexture(const std::string &coverArtPath) {
  if (!coverArtPath.empty()) {
    // Periksa apakah file ada sebelum mencoba memuatnya
    FILE *f = fopen(coverArtPath.c_str(), "rb");
    if (f) {
      fclose(f);
      // Muat langsung via raylib (melewati asset pack karena ini path cache
      // sistem)
      Image img = LoadImage(coverArtPath.c_str());
      if (img.data) {
        Texture2D tex = LoadTextureFromImage(img);
        UnloadImage(img);
        if (tex.id != 0)
          return tex;
      }
    }
  }

  // Cadangan: gunakan placeholder dari asset pack
  return Fumbo::Assets::LoadTexture("assets/images/placeholder.png");
}
