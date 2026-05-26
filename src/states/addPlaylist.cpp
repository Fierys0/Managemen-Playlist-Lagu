#include "addPlaylist.hpp"
#include "mainMenu.hpp"
#include "../core/globals.hpp"
#include "fumbo.hpp"

void AddPlaylist::Init()
{
    // init cover playlist button
    coverBtn = Fumbo::UI::Button({275, 260, 200, 200});
    coverBtn.ApplyStyle(btnstyle);
    coverBtn.Roundness(0.2f);

    // config textbox
    config.cornerRoundness = 0.2f;
    config.padding = {10, 10};
    config.backgroundColor = currentTheme.prim1;
    config.outlineColor = currentTheme.prim1;
    config.focusedOutlineColor = SKYBLUE;
    config.textColor = WHITE;
    config.cursorColor = SKYBLUE;

    // init textbox for title playlist
    titlePlaylist = Fumbo::UI::Textbox({505, 260, 500, 100}, SpaceB, 55);
    titlePlaylist.SetStyle(config);
    titlePlaylist.SetText("Playlist #1");

    // m_eurpos = {400, 325, 300, 85};

    // init add new music button
    addMusicBtn = Fumbo::UI::Button({505, 375, 350, 85});
    addMusicBtn.ApplyStyle(btnstyle);
    addMusicBtn.Roundness(0.2f);
    addMusicBtn.AddText("+ Add music to this playlist", SpaceB, 20, currentTheme.second1);

    // init save button
    savePlaylistBtn = Fumbo::UI::Button({865, 418, 70, 42});
    savePlaylistBtn.ApplyStyle(btnstyle);
    savePlaylistBtn.Roundness(0.2f);
    savePlaylistBtn.AddText("SAVE", SpaceB, 25, currentTheme.second1);

    // init back button for back to main menu
    backBtn = Fumbo::UI::Button({50, 600, 50, 50});
    backBtn.ApplyStyle(btnstyle);
    backBtn.AddText("<", SpaceB, 70, currentTheme.second1);
}

void AddPlaylist::Cleanup() {}
void AddPlaylist::Update()
{

    // change state to main menu if backBtn or save is pressed
    if (backBtn.IsPressed() || savePlaylistBtn.IsPressed())
    {
        Fumbo::Instance().ChangeState(std::make_shared<MainMenu>());
    }

    // open file explorer
    if (addMusicBtn.IsPressed())
    {
    }
    // cover button
    if (coverBtn.IsHover() && !m_coverWasHovered)
    {
        coverBtn.AddText("change cover playlist", SpaceB, 17, currentTheme.second1);
        m_coverWasHovered = true;
    }
    else if (!coverBtn.IsHover() && m_coverWasHovered)
    {
        coverBtn.AddText("", SpaceB, 15, currentTheme.second1);
        m_coverWasHovered = false;
    }

    // update textbox
    titlePlaylist.Update();

    /* // semi otomatis resize and reposition
    if (IsKeyDown(KEY_UP))
    {
        if (IsKeyDown(KEY_LEFT_CONTROL))
            m_eurpos.height--;
        else
            m_eurpos.y--;
    }
    else if (IsKeyDown(KEY_DOWN))
    {
        if (IsKeyDown(KEY_LEFT_CONTROL))
            m_eurpos.height++;
        else
            m_eurpos.y++;
    }
    else if (IsKeyDown(KEY_RIGHT))
    {
        if (IsKeyDown(KEY_LEFT_CONTROL))
        {
            m_eurpos.width++;
        }
        else
            m_eurpos.x++;
    }
    else if (IsKeyDown(KEY_LEFT))
    {
        if (IsKeyDown(KEY_LEFT_CONTROL))
            m_eurpos.width--;
        else
            m_eurpos.x--;
    }

    addMusicBtn.SetBounds(m_eurpos);
    coverBtn.AddText(std::to_string((int)m_eurpos.x) + "+" + std::to_string((int)m_eurpos.y) + "\n" + std::to_string((int)m_eurpos.width) + "+" + std::to_string((int)m_eurpos.height), SpaceB, 13, currentTheme.second1); */
}

void AddPlaylist::DrawClean() {}
void AddPlaylist::DrawDirty()
{
    Fumbo::Graphic2D::DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), currentTheme.prim1);
    coverBtn.Draw();
    titlePlaylist.Draw();
    addMusicBtn.Draw();
    savePlaylistBtn.Draw();
    backBtn.Draw();
}