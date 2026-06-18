#pragma once
#include "../core/appState.hpp"
#include "../core/playlistData.hpp"
#include "fumbo.hpp"

#include <memory>
#include <string>
#include <vector>


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

  // Async file dialogs
  std::unique_ptr<Fumbo::FileDialog::OpenFileAsync> m_audioDialog;
  std::unique_ptr<Fumbo::FileDialog::OpenFileAsync> m_coverDialog;

  // Scroll for track list
  float m_trackListScrollY{0.0f};

  void OpenAudioPicker();
  void OpenCoverPicker();
};
