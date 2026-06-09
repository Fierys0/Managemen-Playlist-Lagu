#include "appState.hpp"
#include "fumbo.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

// Fungsi bantu JSON: escape string untuk output JSON.
static std::string jsonEsc(const std::string &s) {
  std::string out;
  out.reserve(s.size() + 4);
  for (char c : s) {
    if (c == '"')
      out += "\\\"";
    else if (c == '\\')
      out += "\\\\";
    else if (c == '\n')
      out += "\\n";
    else if (c == '\r')
      out += "\\r";
    else if (c == '\t')
      out += "\\t";
    else
      out += c;
  }
  return out;
}

// Fungsi bantu JSON: unescape nilai string JSON (mendukung \", \\, \n, \t).
static std::string jsonUesc(const std::string &s) {
  std::string out;
  out.reserve(s.size());
  for (size_t i = 0; i < s.size(); ++i) {
    if (s[i] == '\\' && i + 1 < s.size()) {
      ++i;
      if (s[i] == '"')
        out += '"';
      else if (s[i] == '\\')
        out += '\\';
      else if (s[i] == 'n')
        out += '\n';
      else if (s[i] == 'r')
        out += '\r';
      else if (s[i] == 't')
        out += '\t';
      else {
        out += '\\';
        out += s[i];
      }
    } else {
      out += s[i];
    }
  }
  return out;
}

// Ambil nilai string bertanda kutip dari teks JSON mulai dari pos.
// Menggeser pos melewati kutip penutup. Mengembalikan "" jika gagal.
static std::string parseJsonString(const std::string &json, size_t &pos) {
  while (pos < json.size() && json[pos] != '"')
    ++pos;
  if (pos >= json.size())
    return {};
  ++pos; // lewati kutip pembuka
  std::string val;
  while (pos < json.size() && json[pos] != '"') {
    if (json[pos] == '\\' && pos + 1 < json.size()) {
      ++pos;
      if (json[pos] == '"')
        val += '"';
      else if (json[pos] == '\\')
        val += '\\';
      else if (json[pos] == 'n')
        val += '\n';
      else if (json[pos] == 'r')
        val += '\r';
      else if (json[pos] == 't')
        val += '\t';
      else {
        val += '\\';
        val += json[pos];
      }
    } else {
      val += json[pos];
    }
    ++pos;
  }
  ++pos; // lewati kutip penutup
  return val;
}

static long long parseJsonNumber(const std::string &json, size_t &pos) {
  while (pos < json.size() &&
         (json[pos] == ' ' || json[pos] == ':' || json[pos] == '\n'))
    ++pos;
  long long v = 0;
  bool neg = false;
  if (pos < json.size() && json[pos] == '-') {
    neg = true;
    ++pos;
  }
  while (pos < json.size() && json[pos] >= '0' && json[pos] <= '9') {
    v = v * 10 + (json[pos] - '0');
    ++pos;
  }
  return neg ? -v : v;
}

static bool parseJsonBool(const std::string &json, size_t &pos) {
  while (pos < json.size() && json[pos] != 't' && json[pos] != 'f')
    ++pos;
  bool v = (json[pos] == 't');
  while (pos < json.size() && json[pos] != ',' && json[pos] != '}' &&
         json[pos] != '\n')
    ++pos;
  return v;
}

// Path helpers untuk lokasi file data pengguna.
// Kembalikan path folder user/ (membuat jika belum ada).
std::string AppState::UserDir() {
  std::string base = Fumbo::Engine::Instance().GetAppDir();
  if (base.empty())
    base = ".";
  if (base.back() != '/' && base.back() != '\\')
    base += '/';
  std::string ud = base + "user";
  try {
    fs::create_directories(ud);
  } catch (...) {
    // Gagal membuat folder, save dan load akan gagal secara halus.
  }
  return ud + '/';
}

std::string AppState::PlaylistsFilePath() {
  return UserDir() + "playlists.json";
}

std::string AppState::PreferencesFilePath() {
  return UserDir() + "preferences.json";
}

// Simpan preferensi ke user/preferences.json dan data playlist ke user/playlists.json.
void AppState::SaveToFile() const {
  // Tulis pengaturan pengguna ke file preferensi.
  {
    std::ofstream pf(PreferencesFilePath());
    if (pf) {
      pf << "{\n";
      pf << "  \"trailsEnabled\": " << (trailsEnabled ? "true" : "false") << ",\n";
      pf << "  \"language\": \"" << jsonEsc(language) << "\",\n";
      pf << "  \"themeName\": \"" << jsonEsc(themeName) << "\"\n";
      pf << "}\n";
    }
  }

  // Tulis data playlist ke file playlist.
  {
    std::ofstream f(PlaylistsFilePath());
    if (!f)
      return;

    f << "{\n";
    f << "  \"nextPlaylistId\": " << nextPlaylistId << ",\n";
    f << "  \"playlists\": [\n";

    for (size_t pi = 0; pi < playlists.size(); ++pi) {
      const auto &pl = playlists[pi];
      f << "    {\n";
      f << "      \"id\": " << pl.id << ",\n";
      f << "      \"name\": \"" << jsonEsc(pl.name) << "\",\n";
      f << "      \"coverPath\": \"" << jsonEsc(pl.coverPath) << "\",\n";
      f << "      \"description\": \"" << jsonEsc(pl.description) << "\",\n";
      f << "      \"tracks\": [\n";
      for (size_t ti = 0; ti < pl.tracks.size(); ++ti) {
        const auto &t = pl.tracks[ti];
        f << "        {\n";
        f << "          \"filePath\": \"" << jsonEsc(t.filePath) << "\",\n";
        f << "          \"title\": \"" << jsonEsc(t.title) << "\",\n";
        f << "          \"artist\": \"" << jsonEsc(t.artist) << "\",\n";
        f << "          \"album\": \"" << jsonEsc(t.album) << "\",\n";
        f << "          \"coverArtPath\": \"" << jsonEsc(t.coverArtPath) << "\",\n";
        f << "          \"durationMs\": " << t.durationMs << "\n";
        f << "        }";
        if (ti + 1 < pl.tracks.size())
          f << ",";
        f << "\n";
      }
      f << "      ]\n";
      f << "    }";
      if (pi + 1 < playlists.size())
        f << ",";
      f << "\n";
    }
    f << "  ]\n";
    f << "}\n";
  }
}

// Muat preferensi dari user/preferences.json lalu data playlist dari user/playlists.json.
void AppState::LoadFromFile() {
  // Baca file preferensi pengguna. Jika tidak ada, nilai default dari deklarasi struct tetap digunakan.
  {
    std::ifstream pf(PreferencesFilePath());
    if (pf) {
      std::string json((std::istreambuf_iterator<char>(pf)),
                       std::istreambuf_iterator<char>());
      size_t pos = 0;

      auto tePos = json.find("\"trailsEnabled\"");
      if (tePos != std::string::npos) {
        pos = tePos + 15;
        trailsEnabled = parseJsonBool(json, pos);
      }

      auto langPos = json.find("\"language\"");
      if (langPos != std::string::npos) {
        pos = langPos + 10;
        language = parseJsonString(json, pos);
      } else {
        language = "id";
      }

      auto themePos = json.find("\"themeName\"");
      if (themePos != std::string::npos) {
        pos = themePos + 11;
        themeName = parseJsonString(json, pos);
      } else {
        themeName = "dark";
      }
    }
  }

  // Baca file playlist, bersihkan data lama sebelum memuat.
  playlists.clear();
  nextPlaylistId = 1;

  std::ifstream f(PlaylistsFilePath());
  if (!f)
    return;

  std::string json((std::istreambuf_iterator<char>(f)),
                   std::istreambuf_iterator<char>());
  size_t pos = 0;

  // Baca nextPlaylistId
  auto nidPos = json.find("\"nextPlaylistId\"");
  if (nidPos != std::string::npos) {
    pos = nidPos + 16;
    while (pos < json.size() && (json[pos] < '0' || json[pos] > '9'))
      ++pos;
    nextPlaylistId = (int)parseJsonNumber(json, pos);
  }

  // Baca array playlists
  auto arrPos = json.find("\"playlists\"");
  if (arrPos == std::string::npos)
    return;
  pos = arrPos + 11;

  // Cari tanda buka [
  while (pos < json.size() && json[pos] != '[')
    ++pos;
  ++pos;

  while (pos < json.size()) {
    while (pos < json.size() && json[pos] != '{' && json[pos] != ']')
      ++pos;
    if (pos >= json.size() || json[pos] == ']')
      break;
    ++pos; // lewati {

    Playlist pl;
    while (pos < json.size() && json[pos] != '}') {
      while (pos < json.size() && json[pos] != '"' && json[pos] != '}')
        ++pos;
      if (pos >= json.size() || json[pos] == '}')
        break;
      std::string key = parseJsonString(json, pos);

      if (key == "id") {
        while (pos < json.size() && (json[pos] < '0' || json[pos] > '9') &&
               json[pos] != '-')
          ++pos;
        pl.id = (int)parseJsonNumber(json, pos);
      } else if (key == "name") {
        pl.name = parseJsonString(json, pos);
      } else if (key == "coverPath") {
        pl.coverPath = parseJsonString(json, pos);
      } else if (key == "description") {
        pl.description = parseJsonString(json, pos);
      } else if (key == "tracks") {
        while (pos < json.size() && json[pos] != '[')
          ++pos;
        ++pos; // lewati [
        while (pos < json.size()) {
          while (pos < json.size() && json[pos] != '{' && json[pos] != ']')
            ++pos;
          if (pos >= json.size() || json[pos] == ']') {
            ++pos;
            break;
          }
          ++pos; // lewati {
          Track t;
          while (pos < json.size() && json[pos] != '}') {
            while (pos < json.size() && json[pos] != '"' && json[pos] != '}')
              ++pos;
            if (pos >= json.size() || json[pos] == '}')
              break;
            std::string tk = parseJsonString(json, pos);
            if (tk == "filePath")
              t.filePath = parseJsonString(json, pos);
            else if (tk == "title")
              t.title = parseJsonString(json, pos);
            else if (tk == "artist")
              t.artist = parseJsonString(json, pos);
            else if (tk == "album")
              t.album = parseJsonString(json, pos);
            else if (tk == "coverArtPath")
              t.coverArtPath = parseJsonString(json, pos);
            else if (tk == "durationMs") {
              while (pos < json.size() &&
                     (json[pos] < '0' || json[pos] > '9') && json[pos] != '-')
                ++pos;
              t.durationMs = parseJsonNumber(json, pos);
            } else {
              // Lewati nilai yang tidak dikenal
              while (pos < json.size() && json[pos] != ',' && json[pos] != '}')
                ++pos;
            }
          }
          if (pos < json.size() && json[pos] == '}')
            ++pos;
          pl.tracks.push_back(t);
        }
      } else {
        // Lewati nilai yang tidak dikenal
        while (pos < json.size() && json[pos] != ',' && json[pos] != '}' &&
               json[pos] != '\n')
          ++pos;
      }
    }
    if (pos < json.size() && json[pos] == '}')
      ++pos;
    playlists.push_back(std::move(pl));
  }
}

void AppState::PlayPlaylist(const Playlist &pl) {
  activePlaylistId = pl.id;
  queue.clear();
  for (const auto &t : pl.tracks)
    queue.push_back(t);
  currentQueueIndex = 0;
  isPlaying = true;
  PlayCurrentTrack();
}

void AppState::NextTrack() {
  if (!queue.empty()) {
    currentQueueIndex = (currentQueueIndex + 1) % (int)queue.size();
    PlayCurrentTrack();
  }
}

void AppState::PrevTrack() {
  if (!queue.empty()) {
    currentQueueIndex =
        (currentQueueIndex - 1 + (int)queue.size()) % (int)queue.size();
    PlayCurrentTrack();
  }
}

void AppState::UpdateMusicPlayback() {
  auto &audio = Fumbo::Engine::Instance().GetAudioManager();

  // Memproses transisi fade out jika ada perpindahan lagu atau playlist
  if (m_isFadingOutTransition) {
    if (!audio.IsMusicPlaying(0)) {
      audio.UnloadAudio("global_track");
      isAudioLoaded = false;
      m_isFadingOutTransition = false;

      // Mainkan lagu berikutnya yang disimpan
      std::string trackId = "global_track";
      audio.LoadAudio(trackId, m_nextTrackToPlay.filePath, Fumbo::Audio::AudioType::MUSIC);
      loadedTrackPath = m_nextTrackToPlay.filePath;
      isAudioLoaded = true;
      audio.PlayMusic(trackId, 0, false);
      isPlaying = true;
    }
    return;
  }

  if (isPlaying && isAudioLoaded) {
    float len = audio.GetMusicLength(0);
    float pos = audio.GetMusicPlayed(0);
    if (len > 0.0f && pos >= len - 0.1f) {
      NextTrack();
    }
  }
}

void AppState::PlayCurrentTrack() {
  const Track *t = CurrentTrack();
  if (!t) {
    StopMusic();
    return;
  }

  auto &audio = Fumbo::Engine::Instance().GetAudioManager();
  std::string trackId = "global_track";

  // Jika lagu yang diminta berbeda dengan yang sedang dimuat
  if (isAudioLoaded && loadedTrackPath != t->filePath) {
    // Jika musik sedang diputar lakukan fade out transisi
    if (isPlaying) {
      m_nextTrackToPlay = *t;
      m_isFadingOutTransition = true;
      audio.StopMusicFade(0, 0.5f);
      loadedTrackPath = ""; // Mengosongkan path agar tidak dianggap terhambat
      return;
    } else {
      // Jika dijeda langsung stop dan bebaskan
      audio.StopMusic(0);
      audio.UnloadAudio(trackId);
      isAudioLoaded = false;
    }
  }

  if (!isAudioLoaded) {
    audio.LoadAudio(trackId, t->filePath, Fumbo::Audio::AudioType::MUSIC);
    loadedTrackPath = t->filePath;
    isAudioLoaded = true;
  }

  audio.PlayMusic(trackId, 0, false);
  isPlaying = true;
}

void AppState::StopMusic() {
  auto &audio = Fumbo::Engine::Instance().GetAudioManager();
  audio.StopMusic(0);
  isPlaying = false;
}

void AppState::TogglePlayPause() {
  auto &audio = Fumbo::Engine::Instance().GetAudioManager();
  if (isPlaying) {
    audio.PauseMusic(0);
    isPlaying = false;
  } else {
    if (isAudioLoaded) {
      audio.ResumeMusic(0);
      isPlaying = true;
    } else {
      PlayCurrentTrack();
    }
  }
}
