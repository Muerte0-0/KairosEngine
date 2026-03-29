#pragma once

namespace Engine
{
    enum Theme
    {
        None = 0,
        
        // Basic
        Light,
        Dark,
        
        // Catppuccin Themes
        Latte,
        Frappé,
        Macchiato,
        Mocha
    };
    class ImGuiUtils
    {
    public:
        static void SetImGuiStyle(Theme theme);
        
    private:
        static void SetTheme_Dark();
        static void SetTheme_Light();
        static void SetTheme_CatppuccinMocha();
    };
}
