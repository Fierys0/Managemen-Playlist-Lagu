#include "playlistCard.hpp"
#include "../core/appState.hpp"
#include "../core/globals.hpp"
#include "fumbo.hpp"
#include <string>

PlaylistCard::PlaylistCard(Rectangle bounds, const Playlist &pl,
                           Texture2D coverTex)
    : m_bounds(bounds), m_image(coverTex), m_title(pl.name),
      m_playlistId(pl.id), m_trackCount((int)pl.tracks.size()) {}

void PlaylistCard::Update() {}

void PlaylistCard::Draw(float offsetX) {
  Rectangle drawBounds = m_bounds;
  drawBounds.x += offsetX;

  // Latar belakang kartu dengan sudut membulat
  Fumbo::Graphic2D::DrawRectangleRounded(drawBounds, 0.08f, 8,
                                         currentTheme.prim2);

  // Gambar sampul
  float imgH = drawBounds.height - 70.0f;
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
                               {drawBounds.x + drawBounds.width * 0.5f - 20,
                                drawBounds.y + imgH * 0.5f - 20},
                               SpaceB, 48, {150, 160, 200, 200});
  }

  // Nama playlist
  Fumbo::Graphic2D::DrawText(m_title,
                             {drawBounds.x + 10, drawBounds.y + imgH + 6},
                             SpaceB, 22, currentTheme.second1);

  // Jumlah lagu
  std::string sub =
      std::to_string(m_trackCount) + " " + Lang::Get("lagu", "songs");
  Fumbo::Graphic2D::DrawText(sub, {drawBounds.x + 10, drawBounds.y + imgH + 34},
                             SpaceB, 15, {160, 165, 175, 255});

  // Gambar tombol Ubah di sudut kanan bawah kartu
  Rectangle editRec = {drawBounds.x + drawBounds.width - 60,
                       drawBounds.y + drawBounds.height - 38, 50, 26};
  bool hoverEdit =
      (offsetX == 0.0f) && CheckCollisionPointRec(GetMousePosition(), editRec);
  Fumbo::Graphic2D::DrawRectangleRounded(
      editRec, 0.2f, 4, hoverEdit ? currentTheme.second2 : currentTheme.prim1);
  Fumbo::Graphic2D::DrawText(Lang::Get("Ubah", "Edit"),
                             {editRec.x + 8, editRec.y + 6}, SpaceB, 13,
                             currentTheme.second1);

  // Sorotan saat hover (hanya jika tidak sedang beranimasi)
  if (offsetX == 0.0f &&
      CheckCollisionPointRec(GetMousePosition(), drawBounds)) {
    Fumbo::Graphic2D::DrawRectangleRoundedLines(drawBounds, 0.08f, 8,
                                                currentTheme.second2);
  }
}

bool PlaylistCard::IsEditClicked() const {
  Rectangle editRec = {m_bounds.x + m_bounds.width - 60,
                       m_bounds.y + m_bounds.height - 38, 50, 26};
  return CheckCollisionPointRec(GetMousePosition(), editRec) &&
         IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

bool PlaylistCard::IsClicked() const {
  Rectangle editRec = {m_bounds.x + m_bounds.width - 60,
                       m_bounds.y + m_bounds.height - 38, 50, 26};
  if (CheckCollisionPointRec(GetMousePosition(), editRec)) {
    return false;
  }
  return CheckCollisionPointRec(GetMousePosition(), m_bounds) &&
         IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}
