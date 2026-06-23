#include "vlcMetadata.hpp"
#include "fumbo.hpp"
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
#include <vlc/vlc.h>

// Instance VLC global pada level modul
static libvlc_instance_t *s_vlc = nullptr;

void VlcMeta::Init() {
  if (s_vlc)
    return;

  std::string appDir = Fumbo::Engine::Instance().GetAppDir();

#ifdef _WIN32
  // Set VLC_PLUGIN_PATH ke direktori plugins lokal secara absolut
  if (!appDir.empty()) {
    std::string pluginsPath = appDir;
    if (pluginsPath.back() != '/' && pluginsPath.back() != '\\')
      pluginsPath += "/";
    pluginsPath += "plugins";

    // Normalisasi pemisah path untuk Windows
    for (char &c : pluginsPath) {
      if (c == '/')
        c = '\\';
    }

    _putenv_s("VLC_PLUGIN_PATH", pluginsPath.c_str());
  }
#endif

  // Gunakan mode quiet tanpa logging ke file
  const char *args[] = {"--quiet"};
  s_vlc = libvlc_new(1, args);

  if (s_vlc)
    Fumbo::Log::Info("[VLC] libvlc initialised successfully");
  else
    Fumbo::Log::Error("[VLC] libvlc_new() returned null — VLC not available");
}

void VlcMeta::Shutdown() {
  if (s_vlc) {
    libvlc_release(s_vlc);
    s_vlc = nullptr;
  }
}

// Decode percent-encoded URI paths (e.g. %20 -> space)
static std::string urlDecode(const std::string &str) {
  std::string result;
  result.reserve(str.length());
  for (size_t i = 0; i < str.length(); ++i) {
    if (str[i] == '%' && i + 2 < str.length()) {
      char hex[3] = {str[i + 1], str[i + 2], '\0'};
      char *end;
      long val = std::strtol(hex, &end, 16);
      if (end == hex + 2) {
        result.push_back(static_cast<char>(val));
        i += 2;
        continue;
      }
    }
    result.push_back(str[i]);
  }
  return result;
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

  Fumbo::Log::Infof("[VLC] GetTrackInfo: '%s'", filePath.c_str());

  if (!s_vlc) {
    Fumbo::Log::Warn("[VLC] s_vlc is null, returning stub track");
    return t;
  }

#ifdef _WIN32
  // On Windows, libvlc_media_new_path expects native backslash paths
  std::string normalizedPath = filePath;
  for (char &c : normalizedPath)
    if (c == '/')
      c = '\\';
#else
  // On non-Windows, libvlc_media_new_path expects forward-slash paths
  std::string normalizedPath = filePath;
  for (char &c : normalizedPath)
    if (c == '\\')
      c = '/';
#endif

  libvlc_media_t *m = libvlc_media_new_path(s_vlc, normalizedPath.c_str());
  if (!m) {
    Fumbo::Log::Errorf("[VLC] libvlc_media_new_path failed for: '%s'",
                       normalizedPath.c_str());
    return t;
  }

  // Urai secara sinkron
  libvlc_media_parse_with_options(
      m,
      (libvlc_media_parse_flag_t)(libvlc_media_parse_local |
                                  libvlc_media_fetch_local),
      5000);

  // Tunggu hingga parsing selesai (maks 5 detik)
  using namespace std::chrono;
  auto deadline = steady_clock::now() + seconds(5);
  while (steady_clock::now() < deadline) {
    libvlc_media_parsed_status_t st = libvlc_media_get_parsed_status(m);
    if (st == libvlc_media_parsed_status_done ||
        st == libvlc_media_parsed_status_failed ||
        st == libvlc_media_parsed_status_timeout) {
      if (st == libvlc_media_parsed_status_failed)
        Fumbo::Log::Warnf("[VLC] Parse failed for: '%s'",
                          normalizedPath.c_str());
      else if (st == libvlc_media_parsed_status_timeout)
        Fumbo::Log::Warnf("[VLC] Parse timed out for: '%s'",
                          normalizedPath.c_str());
      break;
    }
    std::this_thread::sleep_for(milliseconds(100));
  }

  auto getMeta = [&](libvlc_meta_t key) -> std::string {
    char *v = libvlc_media_get_meta(m, key);
    if (!v)
      return {};
    std::string res(v);
    libvlc_free(v);
    return res;
  };

  std::string title = getMeta(libvlc_meta_Title);
  if (!title.empty())
    t.title = title;

  t.artist = getMeta(libvlc_meta_Artist);
  if (t.artist.empty())
    t.artist = getMeta(libvlc_meta_AlbumArtist);

  t.album = getMeta(libvlc_meta_Album);

  std::string artUrl = getMeta(libvlc_meta_ArtworkURL);
  if (!artUrl.empty()) {
    if (artUrl.compare(0, 8, "file:///") == 0) {
#ifdef _WIN32
      t.coverArtPath = urlDecode(artUrl.substr(8));
#else
      t.coverArtPath = urlDecode(artUrl.substr(7));
#endif
    } else if (artUrl.compare(0, 7, "file://") == 0) {
      t.coverArtPath = urlDecode(artUrl.substr(7));
    } else {
      t.coverArtPath = artUrl;
    }
  }

  t.durationMs = libvlc_media_get_duration(m);

  Fumbo::Log::Infof("[VLC] Track parsed — title='%s' artist='%s' "
                    "durationMs=%lld coverArt='%s'",
                    t.title.c_str(), t.artist.c_str(), (long long)t.durationMs,
                    t.coverArtPath.c_str());

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
