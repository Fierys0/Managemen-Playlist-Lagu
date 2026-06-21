#include "playlistCard.hpp"
#include "../core/appState.hpp"
#include "../core/globals.hpp"
#include "fumbo.hpp"
#include <string>

PlaylistCard::PlaylistCard(Rectangle bounds, const Playlist &pl,
                           Texture2D coverTex)
    : m_bounds(bounds), m_image(coverTex), m_title(pl.name),
      // Daftar berantai ganda mengambil jumlah lagu dari DoublyLinkedList size
      m_playlistId(pl.id), m_trackCount(pl.tracks.size()) {
}

void PlaylistCard::Update() {
}

void PlaylistCard::Draw(float offsetX) {
  Rectangle drawBounds = m_bounds;
  drawBounds.x += offsetX;

  // Latar belakang kartu dengan sudut membulat
  Fumbo::Graphic2D::DrawRectangleRounded(drawBounds, 0.08, 8,
                                         currentTheme.prim2);

  // Gambar sampul
  float imgH = drawBounds.height - 70;
  if (m_image.id != 0) {
    Fumbo::Graphic2D::DrawTexture(m_image, {drawBounds.x + 8, drawBounds.y + 8},
                                  {drawBounds.width - 16, imgH - 8});
  } else {
    // Tampilkan gradien sebagai pengganti jika tidak ada sampul
    Color gradEnd = currentTheme.prim2;
    Fumbo::Graphic2D::DrawRectangleGradientV(
        (int)(drawBounds.x + 8), (int)(drawBounds.y + 8),
        (int)(drawBounds.width - 16), (int)(imgH - 8), currentTheme.second2,
        gradEnd);
    Fumbo::Graphic2D::DrawText("Music",
                               {drawBounds.x + drawBounds.width * 0.5f - 20.0f,
                                drawBounds.y + imgH * 0.5f - 20.0f},
                               SpaceB, 48, {150, 160, 200, 200});
  }

  // Nama playlist
  Fumbo::Graphic2D::DrawText(m_title,
                             {drawBounds.x + 10, drawBounds.y + imgH + 12},
                             SpaceB, 22, currentTheme.second1);

  // Jumlah lagu
  std::string sub =
      std::to_string(m_trackCount) + " " + Lang::Get("lagu", "songs");
  Fumbo::Graphic2D::DrawText(sub, {drawBounds.x + 10, drawBounds.y + imgH + 40},
                             SpaceB, 15, {160, 165, 175, 255});

  // Sorotan saat hover (hanya jika tidak sedang beranimasi)
  Rectangle screenDrawBounds = Fumbo::Utils::UISpaceToScreen(drawBounds);

  if (offsetX == 0 &&
      CheckCollisionPointRec(GetMousePosition(), screenDrawBounds)) {
    Fumbo::Graphic2D::DrawRectangleRoundedLines(drawBounds, 0.08, 8,
                                                currentTheme.second2);
  }
}

bool PlaylistCard::IsClicked() const {
  Rectangle screenBounds = Fumbo::Utils::UISpaceToScreen(m_bounds);

  return CheckCollisionPointRec(GetMousePosition(), screenBounds) &&
         IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}
