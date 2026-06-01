#include "playlistCard.hpp"
#include "../core/globals.hpp" 
#include "raylib.h"

PlaylistCard::PlaylistCard(Rectangle bounds, Texture2D img, std::string name) 
    : bounds(bounds), image(img), title(name) {
}

void PlaylistCard::Update() {
}

void PlaylistCard::Draw() {
    Fumbo::Graphic2D::DrawRectangle(bounds.x, bounds.y, bounds.width, bounds.height, DARKGRAY);
    Fumbo::Graphic2D::DrawTexture(image, {bounds.x + 5, bounds.y + 5}, {bounds.width - 10, 350});
    Fumbo::Graphic2D::DrawText(title, {bounds.x + 10, bounds.y + 350}, SpaceB, 48, WHITE);
}

bool PlaylistCard::IsClicked() {
    return CheckCollisionPointRec(GetMousePosition(), bounds) && IsMouseButtonPressed(0);
}