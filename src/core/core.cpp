#include "core.hpp"
#include "fumbo.hpp"
#include "globals.hpp"
#include <cmath>

void AppCore::Init() {
  exitBtn = Fumbo::UI::Button({1210, 10, 40, 40});
  minimizeBtn = Fumbo::UI::Button({1090, 10, 40, 40});
  maximizeBtn = Fumbo::UI::Button({1150, 10, 40, 40});
  exitBtn.ApplyStyle(btnstyle);
  minimizeBtn.ApplyStyle(btnstyle);
  maximizeBtn.ApplyStyle(btnstyle);
  exitBtn.AddText("X");
  maximizeBtn.AddText("O");
  minimizeBtn.AddText("_");
  titleBar = Fumbo::UI::Button({0, 0, 1280, 60});
  titleBar.TextOffsetX(-580);

  mouseTrails.Init();
}

void AppCore::Cleanup() {}

void AppCore::Update() {
  mouseTrails.Update();
  if (exitBtn.IsPressed()) {
    Fumbo::Engine::Instance().Quit();
  }
  if (maximizeBtn.IsPressed()) {
    MaximizeWindow();
  }
  if (minimizeBtn.IsPressed()) {
    MinimizeWindow();
  }

  // Window dragging logic
  if (titleBar.IsPressed() && !exitBtn.IsHover() && !maximizeBtn.IsHover() &&
      !minimizeBtn.IsHover()) {
    m_isDragging = true;
    m_dragOffset = GetMousePosition();
    m_windowPosCache = GetWindowPosition();
  }

  if (m_isDragging) {
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
      Vector2 mousePos = GetMousePosition();
      Vector2 delta = {mousePos.x - m_dragOffset.x,
                       mousePos.y - m_dragOffset.y};

      if (delta.x != 0 || delta.y != 0) {
        m_windowPosCache.x += delta.x;
        m_windowPosCache.y += delta.y;

        SetWindowPosition((int)m_windowPosCache.x, (int)m_windowPosCache.y);

        // Lock the mouse position relative to the window to prevent sliding
        SetMousePosition((int)m_dragOffset.x, (int)m_dragOffset.y);
      }
    } else {
      m_isDragging = false;
    }
  }

  titleBar.IdleColor(currentTheme.titlebar);
  titleBar.SetButtonColor(currentTheme.titlebar);
  titleBar.HoveredColor(currentTheme.titlebar);
  titleBar.AddText("Magfy", SpaceB, 24, currentTheme.second1);
}

void AppCore::DrawClean() { ClearBackground({16, 18, 18, 255}); }

void AppCore::DrawDirty() {
  mouseTrails.Draw();
  titleBar.Draw();
  exitBtn.Draw();
  minimizeBtn.Draw();
  maximizeBtn.Draw();
}
