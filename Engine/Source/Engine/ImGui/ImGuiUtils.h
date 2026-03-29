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
        
        // Catppuccin Themes
        static void SetTheme_CatppuccinLatte();
        static void SetTheme_CatppuccinFrappé();
        static void SetTheme_CatppuccinMacchiato();
        static void SetTheme_CatppuccinMocha();
    };
}
