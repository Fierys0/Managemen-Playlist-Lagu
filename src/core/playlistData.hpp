#pragma once
#include <string>

typedef struct Track {
  std::string MusicLocation{};
  int duration;
} Track;

typedef struct Playlist {
  int id;
  std::string PlaylistName{};
  std::string Cover{};
  Track TrackList[50];
  std::string Description{};
} Playlist;
