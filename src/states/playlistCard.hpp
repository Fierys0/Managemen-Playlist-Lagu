#pragma once
#include "fumbo.hpp"
#include <string>

class PlaylistCard {
private:
    Rectangle bounds;
    Texture2D image;
    std::string title;
    
public:
    PlaylistCard() = default;
    PlaylistCard(Rectangle bounds, Texture2D img, std::string name);
    
    void Update();
    void Draw();
    bool IsClicked();
    bool DeletePressed();
};