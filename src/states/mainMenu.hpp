#pragma once
#include "fumbo.hpp"
#include "playlistCard.hpp"
#include <vector>

class MainMenu : public IGameState {
public:
    void Init() override;
    void Cleanup() override;
    void Update() override;
    void DrawClean() override;
    void DrawDirty() override;
    
private:
    Texture2D settingTex;
    Fumbo::UI::Button settingBtn;
    Fumbo::UI::Button plusBtn;
    
    std::vector<PlaylistCard> playlistCards;
};