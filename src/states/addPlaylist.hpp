#pragma once
#include "fumbo.hpp"

class AddPlaylist : public IGameState
{
public:
    void Init() override;
    void Cleanup() override;
    void Update() override;
    void DrawClean() override;
    void DrawDirty() override;

private:
    Fumbo::UI::Button coverBtn;
    Fumbo::UI::Button addMusicBtn;
    Fumbo::UI::Button backBtn;
    Fumbo::UI::Button savePlaylistBtn;
    Fumbo::UI::Textbox titlePlaylist;
    Fumbo::UI::TextboxConfig config;
    Rectangle m_eurpos;
    bool m_coverWasHovered = false;
};