#pragma once
#include "../core/appState.hpp"
#include "../core/playlistData.hpp"
#include "fumbo.hpp"

#include <memory>
#include <string>
#include <vector>

// Forward-declare pfd so we don't pull the whole header into every TU
namespace pfd {
class open_file;
}

class AddPlaylist : public IGameState {
public:
  AddPlaylist(int playlistId = -1);
  void Init() override;
  void Cleanup() override;
  void Update() override;
  void DrawClean() override;
  void DrawDirty() override;

private:
  // UI
  Fumbo::UI::Button m_coverBtn;
  Fumbo::UI::Button m_addMusicBtn;
  Fumbo::UI::Button m_saveBtn;
  Fumbo::UI::Button m_backBtn;
  Fumbo::UI::Button m_deleteBtn; // Hanya tampil saat mengedit
  Fumbo::UI::Textbox m_titleBox;
  Fumbo::UI::TextboxConfig m_tbConfig;

  // State
  int m_playlistId{-1};
  std::vector<Track> m_tracks;
  Texture2D m_coverTex{}; // custom cover (from picker)
  std::string m_coverPath{};
  bool m_coverHovered{false};

  // Async file dialog (pfd is blocking but we run it synchronously)
  bool m_dialogPending{false};
  bool m_coverDialogPending{false};

  // Scroll for track list
  float m_trackListScrollY{0.0f};

  void OpenAudioPicker();
  void OpenCoverPicker();
};
