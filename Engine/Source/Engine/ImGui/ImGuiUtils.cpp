#include "kepch.h"
#include "ImGuiUtils.h"

#include "imgui.h"

namespace Engine
{
	static float SRGBToLinear(float c)
	{
		if (c <= 0.04045f)
			return c / 12.92f;
		return powf((c + 0.055f) / 1.055f, 2.4f);
	}

	static ImVec4 RGBAtoIV4(float r, float g, float b, float a = 255)
	{
		float newr = r / 255.0f;
		float newg = g / 255.0f;
		float newb = b / 255.0f;
		float newa = a / 255.0f;
		
		return {SRGBToLinear(newr), SRGBToLinear(newg), SRGBToLinear(newb), SRGBToLinear(newa)};
	}

	static ImVec4 HexToImVec4(const std::string& hex)
	{
		std::string clean = hex;
		if (clean[0] == '#')
			clean = clean.substr(1);

		uint32_t value = std::stoul(clean, nullptr, 16);

		if (clean.length() == 6)
		{
			float r = static_cast<float>((value >> 16) & 0xFF) / 255.0f;
			float g = static_cast<float>((value >> 8)  & 0xFF) / 255.0f;
			float b = static_cast<float>((value)       & 0xFF) / 255.0f;

			return {
				SRGBToLinear(r),
				SRGBToLinear(g),
				SRGBToLinear(b),
				1.0f
			};
		}
		else if (clean.length() == 8)
		{
			float r = ((value >> 24) & 0xFF) / 255.0f;
			float g = ((value >> 16) & 0xFF) / 255.0f;
			float b = ((value >> 8)  & 0xFF) / 255.0f;
			float a = ((value)       & 0xFF) / 255.0f;

			return ImVec4(
				SRGBToLinear(r),
				SRGBToLinear(g),
				SRGBToLinear(b),
				a
			);
		}

		return ImVec4(1, 0, 0, 1);
	}
	
    void ImGuiUtils::SetImGuiStyle(Theme theme)
    {
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;			// Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;			// Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;				// Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;				// Enable Viewports
        io.ConfigFlags |= ImGuiConfigFlags_IsSRGB;						// Enable SRGB for ImGui

        //float fontSize = 18.0f;
        //io.FontDefault = io.Fonts->AddFontFromFileTTF("D:/Dev/Projects/KairosEngine/Editor/Assets/Fonts/OpenSans/OpenSans-Regular.ttf", fontSize);
        //io.Fonts->AddFontFromFileTTF("D:/Dev/Projects/KairosEngine/Editor/Assets/Fonts/OpenSans/OpenSans-Bold.ttf", fontSize);
        
		ImGuiStyle& style = ImGui::GetStyle();
		
		// =========================================================
		// Layout
		// =========================================================
		style.WindowRounding    = 6.0f;
		style.ChildRounding     = 6.0f;
		style.FrameRounding     = 4.0f;
		style.PopupRounding     = 4.0f;
		style.ScrollbarRounding = 9.0f;
		style.GrabRounding      = 4.0f;
		style.TabRounding       = 4.0f;
    
		style.WindowPadding     = ImVec2(8.0f, 8.0f);
		style.FramePadding      = ImVec2(5.0f, 3.0f);
		style.ItemSpacing       = ImVec2(8.0f, 4.0f);
		style.ItemInnerSpacing  = ImVec2(4.0f, 4.0f);
    
		style.IndentSpacing     = 21.0f;
		style.ScrollbarSize     = 14.0f;
		style.GrabMinSize       = 10.0f;
    
		// Borders
		style.WindowBorderSize  = 1.0f;
		style.ChildBorderSize   = 1.0f;
		style.PopupBorderSize   = 1.0f;
		style.FrameBorderSize   = 0.0f;
		style.TabBorderSize     = 0.0f;
		
        switch (theme)
        {
            case Theme::Dark:       SetTheme_Dark(); break;
            case Theme::Light:      SetTheme_Light(); break;
            case Theme::Latte:		SetTheme_CatppuccinLatte(); break; 
            case Theme::Frappé:		SetTheme_CatppuccinFrappé(); break; 
            case Theme::Macchiato:  SetTheme_CatppuccinMacchiato(); break; 
            case Theme::Mocha:      SetTheme_CatppuccinMocha(); break; 
            default:                break;
        }
    }

   void ImGuiUtils::SetTheme_Dark()
	{
	    ImGuiStyle& style = ImGui::GetStyle();
	    ImVec4* colors = style.Colors;
	
	    // =========================================================
	    // 1. Sizing & Spacing (same as light for consistency)
	    // =========================================================
	    style.WindowPadding     = ImVec2(12.0f, 12.0f);
	    style.FramePadding      = ImVec2(6.0f, 4.0f);
	    style.CellPadding       = ImVec2(6.0f, 4.0f);
	    style.ItemSpacing       = ImVec2(8.0f, 6.0f);
	    style.ItemInnerSpacing  = ImVec2(6.0f, 4.0f);
	    style.ScrollbarSize     = 14.0f;
	    style.GrabMinSize       = 12.0f;
	
	    // =========================================================
	    // 2. Borders & Rounding (same rigid structure)
	    // =========================================================
	    style.WindowRounding    = 2.0f;
	    style.ChildRounding     = 2.0f;
	    style.FrameRounding     = 2.0f;
	    style.PopupRounding     = 2.0f;
	    style.ScrollbarRounding = 12.0f;
	    style.GrabRounding      = 2.0f;
	    style.TabRounding       = 2.0f;
	
	    style.WindowBorderSize  = 1.0f;
	    style.ChildBorderSize   = 1.0f;
	    style.PopupBorderSize   = 1.0f;
	    style.FrameBorderSize   = 1.0f;
	    style.TabBorderSize     = 1.0f;
	
	    // =========================================================
	    // 3. Dark Palette (derived from your light theme)
	    // =========================================================
	
	    // Core tones
	    const ImVec4 bg        = ImVec4(0.010f, 0.0105f, 0.011f, 1.00f); // Dark canvas
	    const ImVec4 panel     = ImVec4(0.015f, 0.015f, 0.015f, 1.00f); // Panels
	    const ImVec4 surface   = ImVec4(0.018f, 0.0185f, 0.019f, 1.00f); // Inputs
	    const ImVec4 hover     = ImVec4(0.024f, 0.026f, 0.029f, 1.00f);
	    const ImVec4 active    = ImVec4(0.020f, 0.022f, 0.025f, 1.00f);
	
	    const ImVec4 border    = ImVec4(0.028f, 0.028f, 0.026f, 1.00f);
	
	    // Text
	    const ImVec4 text      = ImVec4(0.86f, 0.87f, 0.88f, 1.00f);
	    const ImVec4 textDim   = ImVec4(0.55f, 0.56f, 0.58f, 1.00f);
	
	    // Accent
	    const ImVec4 accent    = ImVec4(0.026f, 0.052f, 0.085f, 1.00f);
	
	    // =========================================================
	    // Core
	    // =========================================================
	    colors[ImGuiCol_Text]         = text;
	    colors[ImGuiCol_TextDisabled] = textDim;
	
	    colors[ImGuiCol_WindowBg] = bg;
	    colors[ImGuiCol_ChildBg]  = ImVec4(0,0,0,0);
	    colors[ImGuiCol_PopupBg]  = panel;
	
	    // =========================================================
	    // Borders & Separators
	    // =========================================================
	    colors[ImGuiCol_Border]        = border;
	    colors[ImGuiCol_BorderShadow]  = ImVec4(0,0,0,0);
	
	    colors[ImGuiCol_Separator]        = border;
	    colors[ImGuiCol_SeparatorHovered] = accent;
	    colors[ImGuiCol_SeparatorActive]  = accent;
	
	    // =========================================================
	    // Frames (Inputs)
	    // =========================================================
	    colors[ImGuiCol_FrameBg]         = surface;
	    colors[ImGuiCol_FrameBgHovered]  = hover;
	    colors[ImGuiCol_FrameBgActive]   = active;
	
	    // =========================================================
	    // Titles & Menu
	    // =========================================================
	    colors[ImGuiCol_TitleBg]          = panel;
	    colors[ImGuiCol_TitleBgActive]    = surface;
	    colors[ImGuiCol_TitleBgCollapsed] = panel;
	    colors[ImGuiCol_MenuBarBg]        = panel;
	
	    // =========================================================
	    // Scrollbars
	    // =========================================================
	    colors[ImGuiCol_ScrollbarBg]          = bg;
	    colors[ImGuiCol_ScrollbarGrab]        = surface;
	    colors[ImGuiCol_ScrollbarGrabHovered] = hover;
	    colors[ImGuiCol_ScrollbarGrabActive]  = active;
	
	    // =========================================================
	    // Interactables (Blueprint Accent System)
	    // =========================================================
	    colors[ImGuiCol_CheckMark]        = accent;
	    colors[ImGuiCol_SliderGrab]       = accent;
	    colors[ImGuiCol_SliderGrabActive] = accent;
	
	    colors[ImGuiCol_Button]          = ImVec4(accent.x, accent.y, accent.z, 0.10f);
	    colors[ImGuiCol_ButtonHovered]   = ImVec4(accent.x, accent.y, accent.z, 0.25f);
	    colors[ImGuiCol_ButtonActive]    = ImVec4(accent.x, accent.y, accent.z, 0.40f);
	
	    colors[ImGuiCol_Header]          = ImVec4(accent.x, accent.y, accent.z, 0.12f);
	    colors[ImGuiCol_HeaderHovered]   = ImVec4(accent.x, accent.y, accent.z, 0.25f);
	    colors[ImGuiCol_HeaderActive]    = ImVec4(accent.x, accent.y, accent.z, 0.45f);
	
	    // =========================================================
	    // Tables (critical for readability)
	    // =========================================================
	    colors[ImGuiCol_TableHeaderBg]     = panel;
	    colors[ImGuiCol_TableBorderStrong] = border;
	    colors[ImGuiCol_TableBorderLight]  = surface;
	    colors[ImGuiCol_TableRowBgAlt]     = ImVec4(1,1,1,0.03f);
	
	    // =========================================================
	    // Tabs
	    // =========================================================
	    colors[ImGuiCol_Tab]                = panel;
	    colors[ImGuiCol_TabHovered]         = hover;
	    colors[ImGuiCol_TabActive]          = surface;
	    colors[ImGuiCol_TabUnfocused]       = panel;
	    colors[ImGuiCol_TabUnfocusedActive] = surface;
	
	    // =========================================================
	    // Misc
	    // =========================================================
	    colors[ImGuiCol_PlotLines]         = accent;
	    colors[ImGuiCol_PlotHistogram]     = accent;
	    colors[ImGuiCol_TextSelectedBg]    = ImVec4(accent.x, accent.y, accent.z, 0.35f);
	    colors[ImGuiCol_DragDropTarget]    = accent;
	    colors[ImGuiCol_NavHighlight]      = accent;
	
#ifdef IMGUI_HAS_DOCK
	    colors[ImGuiCol_DockingPreview] = ImVec4(accent.x, accent.y, accent.z, 0.40f);
	    colors[ImGuiCol_DockingEmptyBg] = bg;
#endif
}

    void ImGuiUtils::SetTheme_Light()
    {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;
    
        // --- 1. Sizing & Spacing (Clean & Rigid) ---
        style.WindowPadding = ImVec2(12.0f, 12.0f);
        style.FramePadding = ImVec2(6.0f, 4.0f);
        style.CellPadding = ImVec2(6.0f, 4.0f);
        style.ItemSpacing = ImVec2(8.0f, 6.0f);
        style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
        style.ScrollbarSize = 14.0f;
        style.GrabMinSize = 12.0f;
    
        // --- 2. Borders & Rounding (Technical/Drafting feel) ---
        style.WindowRounding = 2.0f;
        style.ChildRounding = 2.0f;
        style.FrameRounding = 2.0f;
        style.PopupRounding = 2.0f;
        style.ScrollbarRounding = 12.0f;
        style.GrabRounding = 2.0f;
        style.TabRounding = 2.0f;
    
        style.WindowBorderSize = 1.0f;
        style.ChildBorderSize = 1.0f;
        style.PopupBorderSize = 1.0f;
        style.FrameBorderSize = 1.0f;
        style.TabBorderSize = 1.0f;
    
        // --- 3. Full Color Palette ---
    
        // Main Text & Background
        colors[ImGuiCol_Text] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f); // Deep Carbon Ink
        colors[ImGuiCol_TextDisabled] = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.96f, 0.96f, 0.94f, 1.00f); // Warm Paper
        colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.03f);
        colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // Clean White Popups
    
        // Borders & Separators
        colors[ImGuiCol_Border] = ImVec4(0.75f, 0.75f, 0.72f, 1.00f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_Separator] = ImVec4(0.80f, 0.80f, 0.78f, 1.00f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.17f, 0.34f, 0.59f, 0.78f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.17f, 0.34f, 0.59f, 1.00f);
    
        // Frames (Inputs, Checkboxes, etc)
        colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.90f, 0.92f, 0.95f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.85f, 0.88f, 0.92f, 1.00f);
    
        // Titles & Menus
        colors[ImGuiCol_TitleBg] = ImVec4(0.92f, 0.92f, 0.90f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.88f, 0.88f, 0.86f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.92f, 0.92f, 0.90f, 0.75f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.92f, 0.92f, 0.90f, 1.00f);
    
        // Scrollbars
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.96f, 0.96f, 0.94f, 1.00f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.78f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.70f, 0.70f, 0.68f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.60f, 0.60f, 0.58f, 1.00f);
    
        // Interactables (Blueprint Blue)
        colors[ImGuiCol_CheckMark] = ImVec4(0.17f, 0.34f, 0.59f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.17f, 0.34f, 0.59f, 0.70f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.17f, 0.34f, 0.59f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(0.17f, 0.34f, 0.59f, 0.08f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.17f, 0.34f, 0.59f, 0.20f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.17f, 0.34f, 0.59f, 0.35f);
    
        // Header (Selection in lists/trees)
        colors[ImGuiCol_Header] = ImVec4(0.17f, 0.34f, 0.59f, 0.12f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.17f, 0.34f, 0.59f, 0.25f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.17f, 0.34f, 0.59f, 0.40f);
    
        // Tables (Crucial for Light Mode)
        colors[ImGuiCol_TableHeaderBg] = ImVec4(0.90f, 0.90f, 0.88f, 1.00f);
        colors[ImGuiCol_TableBorderStrong] = ImVec4(0.75f, 0.75f, 0.72f, 1.00f);
        colors[ImGuiCol_TableBorderLight] = ImVec4(0.85f, 0.85f, 0.82f, 1.00f);
        colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.00f, 0.00f, 0.00f, 0.03f);
    
        // Tabs
        colors[ImGuiCol_Tab] = ImVec4(0.92f, 0.92f, 0.90f, 1.00f);
        colors[ImGuiCol_TabHovered] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_TabActive] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.92f, 0.92f, 0.90f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.96f, 0.96f, 0.94f, 1.00f);
    
        // Misc
        colors[ImGuiCol_PlotLines] = ImVec4(0.17f, 0.34f, 0.59f, 1.00f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.17f, 0.34f, 0.59f, 1.00f);
        colors[ImGuiCol_TextSelectedBg] = ImVec4(0.17f, 0.34f, 0.59f, 0.25f);
        colors[ImGuiCol_DragDropTarget] = ImVec4(0.17f, 0.34f, 0.59f, 0.90f);
        colors[ImGuiCol_NavHighlight] = ImVec4(0.17f, 0.34f, 0.59f, 1.00f);
    
#ifdef IMGUI_HAS_DOCK
        colors[ImGuiCol_DockingPreview] = ImVec4(0.17f, 0.34f, 0.59f, 0.40f);
        colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.96f, 0.96f, 0.94f, 1.00f);
#endif
    }

	void ImGuiUtils::SetTheme_CatppuccinLatte()
	{
        ImVec4* colors = ImGui::GetStyle().Colors;
    
        // =========================================================
        // Catppuccin Latte Palette
        // =========================================================
    
		// --- Base layers ---
		const ImVec4 base      = HexToImVec4("#eff1f5");
		const ImVec4 mantle    = HexToImVec4("#e6e9ef");
		const ImVec4 crust     = HexToImVec4("#dce0e8");
		const ImVec4 surface0  = HexToImVec4("#ccd0da");
		const ImVec4 surface1  = HexToImVec4("#bcc0cc");
		const ImVec4 surface2  = HexToImVec4("#acb0be");

		// --- Overlay / UI chrome ---
		const ImVec4 overlay0  = HexToImVec4("#9ca0b0");
		const ImVec4 overlay1  = HexToImVec4("#8c8fa1");
		const ImVec4 overlay2  = HexToImVec4("#7c7f93");

		// --- Text hierarchy ---
		const ImVec4 text      = HexToImVec4("#4c4f69");
		const ImVec4 subtext0  = HexToImVec4("#6c6f85");
		const ImVec4 subtext1  = HexToImVec4("#5c5f77");

		// --- Accent colors ---
		const ImVec4 rosewater = HexToImVec4("#dc8a78");
		const ImVec4 flamingo  = HexToImVec4("#dd7878");
		const ImVec4 pink      = HexToImVec4("#ea76cb");
		const ImVec4 mauve     = HexToImVec4("#8839ef");
		const ImVec4 red       = HexToImVec4("#d20f39");
		const ImVec4 maroon    = HexToImVec4("#e64553");
		const ImVec4 peach     = HexToImVec4("#fe640b");
		const ImVec4 yellow    = HexToImVec4("#df8e1d");
		const ImVec4 green     = HexToImVec4("#40a02b");
		const ImVec4 teal      = HexToImVec4("#179299");
		const ImVec4 sky       = HexToImVec4("#04a5e5");
		const ImVec4 sapphire  = HexToImVec4("#209fb5");
		const ImVec4 blue      = HexToImVec4("#1e66f5");
		const ImVec4 lavender  = HexToImVec4("#7287fd");
		
        // =========================================================
        // Core UI
        // =========================================================
        colors[ImGuiCol_WindowBg]  = base;
        colors[ImGuiCol_ChildBg]   = mantle;
        colors[ImGuiCol_PopupBg]   = crust;
    
        colors[ImGuiCol_Border]        = surface1;
        colors[ImGuiCol_BorderShadow]  = ImVec4(0,0,0,0);
    
        // =========================================================
        // Text
        // =========================================================
        colors[ImGuiCol_Text]         = text;
        colors[ImGuiCol_TextDisabled] = subtext0;
    
        // =========================================================
        // Interactive Elements (Frame-based widgets)
        // =========================================================
        colors[ImGuiCol_FrameBg]         = surface0;
        colors[ImGuiCol_FrameBgHovered]  = surface1;
        colors[ImGuiCol_FrameBgActive]   = surface2;
    	
    	colors[ImGuiCol_Button]        = ImVec4(blue.x, blue.y, blue.z, 0.10f);
    	colors[ImGuiCol_ButtonHovered] = ImVec4(blue.x, blue.y, blue.z, 0.22f);
    	colors[ImGuiCol_ButtonActive]  = ImVec4(blue.x, blue.y, blue.z, 0.35f);
    
        colors[ImGuiCol_Header]          = surface0;
        colors[ImGuiCol_HeaderHovered]   = surface1;
        colors[ImGuiCol_HeaderActive]    = surface2;
    
        // =========================================================
        // Titles & Menu
        // =========================================================
        colors[ImGuiCol_TitleBg]          = mantle;
        colors[ImGuiCol_TitleBgActive]    = surface0;
        colors[ImGuiCol_TitleBgCollapsed] = mantle;
        colors[ImGuiCol_MenuBarBg]        = mantle;
    
        // =========================================================
        // Scrollbars
        // =========================================================
        colors[ImGuiCol_ScrollbarBg]          = surface0;
        colors[ImGuiCol_ScrollbarGrab]        = surface2;
        colors[ImGuiCol_ScrollbarGrabHovered] = overlay0;
        colors[ImGuiCol_ScrollbarGrabActive]  = overlay2;
    
        // =========================================================
        // Tabs
        // =========================================================
        colors[ImGuiCol_Tab]                = surface0;
        colors[ImGuiCol_TabHovered]         = surface2;
        colors[ImGuiCol_TabActive]          = surface1;
        colors[ImGuiCol_TabUnfocused]       = surface0;
        colors[ImGuiCol_TabUnfocusedActive] = surface1;
    
        // =========================================================
        // Accents / Feedback
        // =========================================================
        colors[ImGuiCol_CheckMark]        = green;
        colors[ImGuiCol_SliderGrab]       = sapphire;
        colors[ImGuiCol_SliderGrabActive] = blue;
    
        colors[ImGuiCol_Separator]        = surface1;
        colors[ImGuiCol_SeparatorHovered] = mauve;
        colors[ImGuiCol_SeparatorActive]  = mauve;
    
        colors[ImGuiCol_ResizeGrip]       = surface2;
        colors[ImGuiCol_ResizeGripHovered]= mauve;
        colors[ImGuiCol_ResizeGripActive] = mauve;
    
        // =========================================================
        // Tables & Misc
        // =========================================================
        colors[ImGuiCol_TableHeaderBg]    = surface0;
        colors[ImGuiCol_TableBorderStrong]= surface1;
        colors[ImGuiCol_TableBorderLight] = surface0;
    
        colors[ImGuiCol_TableRowBg]       = ImVec4(0,0,0,0);
        colors[ImGuiCol_TableRowBgAlt]    = ImVec4(1,1,1,0.06f);
    
        colors[ImGuiCol_TextSelectedBg]   = surface2;
        colors[ImGuiCol_DragDropTarget]   = yellow;
    
        colors[ImGuiCol_NavHighlight]         = blue;
        colors[ImGuiCol_NavWindowingHighlight]= ImVec4(1,1,1,0.7f);
        colors[ImGuiCol_NavWindowingDimBg]    = ImVec4(0.8f,0.8f,0.8f,0.2f);
        colors[ImGuiCol_ModalWindowDimBg]     = ImVec4(0,0,0,0.35f);
        
#ifdef IMGUI_HAS_DOCK
        colors[ImGuiCol_DockingPreview] = ImVec4(0.71f, 0.75f, 1.00f, 0.50f);
        colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.12f, 0.12f, 0.18f, 1.00f);
#endif
	}

	void ImGuiUtils::SetTheme_CatppuccinFrappé()
	{
		ImVec4* colors = ImGui::GetStyle().Colors;
    
        // =========================================================
        // Catppuccin Frappé Palette
        // =========================================================
    
		// --- Base layers ---
		const ImVec4 base      = HexToImVec4("#303446");
		const ImVec4 mantle    = HexToImVec4("#292c3c");
		const ImVec4 crust     = HexToImVec4("#232634");
		const ImVec4 surface0  = HexToImVec4("#414559");
		const ImVec4 surface1  = HexToImVec4("#51576d");
		const ImVec4 surface2  = HexToImVec4("#626880");

		// --- Overlay ---
		const ImVec4 overlay0  = HexToImVec4("#737994");
		const ImVec4 overlay1  = HexToImVec4("#838ba7");
		const ImVec4 overlay2  = HexToImVec4("#949cbb");

		// --- Text ---
		const ImVec4 text      = HexToImVec4("#c6d0f5");
		const ImVec4 subtext0  = HexToImVec4("#a5adce");
		const ImVec4 subtext1  = HexToImVec4("#b5bfe2");

		// --- Accents ---
		const ImVec4 rosewater = HexToImVec4("#f2d5cf");
		const ImVec4 flamingo  = HexToImVec4("#eebebe");
		const ImVec4 pink      = HexToImVec4("#f4b8e4");
		const ImVec4 mauve     = HexToImVec4("#ca9ee6");
		const ImVec4 red       = HexToImVec4("#e78284");
		const ImVec4 maroon    = HexToImVec4("#ea999c");
		const ImVec4 peach     = HexToImVec4("#ef9f76");
		const ImVec4 yellow    = HexToImVec4("#e5c890");
		const ImVec4 green     = HexToImVec4("#a6d189");
		const ImVec4 teal      = HexToImVec4("#81c8be");
		const ImVec4 sky       = HexToImVec4("#99d1db");
		const ImVec4 sapphire  = HexToImVec4("#85c1dc");
		const ImVec4 blue      = HexToImVec4("#8caaee");
		const ImVec4 lavender  = HexToImVec4("#babbf1");
		
        // =========================================================
        // Core UI
        // =========================================================
        colors[ImGuiCol_WindowBg]  = base;
        colors[ImGuiCol_ChildBg]   = mantle;
        colors[ImGuiCol_PopupBg]   = crust;
    
        colors[ImGuiCol_Border]        = surface1;
        colors[ImGuiCol_BorderShadow]  = ImVec4(0,0,0,0);
    
        // =========================================================
        // Text
        // =========================================================
        colors[ImGuiCol_Text]         = text;
        colors[ImGuiCol_TextDisabled] = subtext0;
    
        // =========================================================
        // Interactive Elements (Frame-based widgets)
        // =========================================================
        colors[ImGuiCol_FrameBg]         = surface0;
        colors[ImGuiCol_FrameBgHovered]  = surface1;
        colors[ImGuiCol_FrameBgActive]   = surface2;
    	
    	colors[ImGuiCol_Button]        = ImVec4(blue.x, blue.y, blue.z, 0.10f);
    	colors[ImGuiCol_ButtonHovered] = ImVec4(blue.x, blue.y, blue.z, 0.22f);
    	colors[ImGuiCol_ButtonActive]  = ImVec4(blue.x, blue.y, blue.z, 0.35f);
    
        colors[ImGuiCol_Header]          = surface0;
        colors[ImGuiCol_HeaderHovered]   = surface1;
        colors[ImGuiCol_HeaderActive]    = surface2;
    
        // =========================================================
        // Titles & Menu
        // =========================================================
        colors[ImGuiCol_TitleBg]          = mantle;
        colors[ImGuiCol_TitleBgActive]    = surface0;
        colors[ImGuiCol_TitleBgCollapsed] = mantle;
        colors[ImGuiCol_MenuBarBg]        = mantle;
    
        // =========================================================
        // Scrollbars
        // =========================================================
        colors[ImGuiCol_ScrollbarBg]          = surface0;
        colors[ImGuiCol_ScrollbarGrab]        = surface2;
        colors[ImGuiCol_ScrollbarGrabHovered] = overlay0;
        colors[ImGuiCol_ScrollbarGrabActive]  = overlay2;
    
        // =========================================================
        // Tabs
        // =========================================================
        colors[ImGuiCol_Tab]                = surface0;
        colors[ImGuiCol_TabHovered]         = surface2;
        colors[ImGuiCol_TabActive]          = surface1;
        colors[ImGuiCol_TabUnfocused]       = surface0;
        colors[ImGuiCol_TabUnfocusedActive] = surface1;
    
        // =========================================================
        // Accents / Feedback
        // =========================================================
        colors[ImGuiCol_CheckMark]        = green;
        colors[ImGuiCol_SliderGrab]       = sapphire;
        colors[ImGuiCol_SliderGrabActive] = blue;
    
        colors[ImGuiCol_Separator]        = surface1;
        colors[ImGuiCol_SeparatorHovered] = mauve;
        colors[ImGuiCol_SeparatorActive]  = mauve;
    
        colors[ImGuiCol_ResizeGrip]       = surface2;
        colors[ImGuiCol_ResizeGripHovered]= mauve;
        colors[ImGuiCol_ResizeGripActive] = mauve;
    
        // =========================================================
        // Tables & Misc
        // =========================================================
        colors[ImGuiCol_TableHeaderBg]    = surface0;
        colors[ImGuiCol_TableBorderStrong]= surface1;
        colors[ImGuiCol_TableBorderLight] = surface0;
    
        colors[ImGuiCol_TableRowBg]       = ImVec4(0,0,0,0);
        colors[ImGuiCol_TableRowBgAlt]    = ImVec4(1,1,1,0.06f);
    
        colors[ImGuiCol_TextSelectedBg]   = surface2;
        colors[ImGuiCol_DragDropTarget]   = yellow;
    
        colors[ImGuiCol_NavHighlight]         = blue;
        colors[ImGuiCol_NavWindowingHighlight]= ImVec4(1,1,1,0.7f);
        colors[ImGuiCol_NavWindowingDimBg]    = ImVec4(0.8f,0.8f,0.8f,0.2f);
        colors[ImGuiCol_ModalWindowDimBg]     = ImVec4(0,0,0,0.35f);
        
#ifdef IMGUI_HAS_DOCK
        colors[ImGuiCol_DockingPreview] = ImVec4(0.71f, 0.75f, 1.00f, 0.50f);
        colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.12f, 0.12f, 0.18f, 1.00f);
#endif
	}

	void ImGuiUtils::SetTheme_CatppuccinMacchiato()
	{
		ImVec4* colors = ImGui::GetStyle().Colors;
		
	    // =========================================================
	    // Catppuccin Macchiato Palette
	    // =========================================================
	
		// --- Base layers ---
		const ImVec4 base      = HexToImVec4("#24273a");
		const ImVec4 mantle    = HexToImVec4("#1e2030");
		const ImVec4 crust     = HexToImVec4("#181926");
		const ImVec4 surface0  = HexToImVec4("#363a4f");
		const ImVec4 surface1  = HexToImVec4("#494d64");
		const ImVec4 surface2  = HexToImVec4("#5b6078");

		// --- Overlay ---
		const ImVec4 overlay0  = HexToImVec4("#6e738d");
		const ImVec4 overlay1  = HexToImVec4("#8087a2");
		const ImVec4 overlay2  = HexToImVec4("#939ab7");

		// --- Text ---
		const ImVec4 text      = HexToImVec4("#cad3f5");
		const ImVec4 subtext0  = HexToImVec4("#a5adcb");
		const ImVec4 subtext1  = HexToImVec4("#b8c0e0");

		// --- Accents ---
		const ImVec4 rosewater = HexToImVec4("#f4dbd6");
		const ImVec4 flamingo  = HexToImVec4("#f0c6c6");
		const ImVec4 pink      = HexToImVec4("#f5bde6");
		const ImVec4 mauve     = HexToImVec4("#c6a0f6");
		const ImVec4 red       = HexToImVec4("#ed8796");
		const ImVec4 maroon    = HexToImVec4("#ee99a0");
		const ImVec4 peach     = HexToImVec4("#f5a97f");
		const ImVec4 yellow    = HexToImVec4("#eed49f");
		const ImVec4 green     = HexToImVec4("#a6da95");
		const ImVec4 teal      = HexToImVec4("#8bd5ca");
		const ImVec4 sky       = HexToImVec4("#91d7e3");
		const ImVec4 sapphire  = HexToImVec4("#7dc4e4");
		const ImVec4 blue      = HexToImVec4("#8aadf4");
		const ImVec4 lavender  = HexToImVec4("#b7bdf8");
	
	    // =========================================================
        // Core UI
        // =========================================================
        colors[ImGuiCol_WindowBg]  = base;
        colors[ImGuiCol_ChildBg]   = mantle;
        colors[ImGuiCol_PopupBg]   = crust;
    
        colors[ImGuiCol_Border]        = surface1;
        colors[ImGuiCol_BorderShadow]  = ImVec4(0,0,0,0);
    
        // =========================================================
        // Text
        // =========================================================
        colors[ImGuiCol_Text]         = text;
        colors[ImGuiCol_TextDisabled] = subtext0;
    
        // =========================================================
        // Interactive Elements (Frame-based widgets)
        // =========================================================
        colors[ImGuiCol_FrameBg]         = surface0;
        colors[ImGuiCol_FrameBgHovered]  = surface1;
        colors[ImGuiCol_FrameBgActive]   = surface2;
    	
    	colors[ImGuiCol_Button]        = ImVec4(blue.x, blue.y, blue.z, 0.10f);
    	colors[ImGuiCol_ButtonHovered] = ImVec4(blue.x, blue.y, blue.z, 0.22f);
    	colors[ImGuiCol_ButtonActive]  = ImVec4(blue.x, blue.y, blue.z, 0.35f);
    
        colors[ImGuiCol_Header]          = surface0;
        colors[ImGuiCol_HeaderHovered]   = surface1;
        colors[ImGuiCol_HeaderActive]    = surface2;
    
        // =========================================================
        // Titles & Menu
        // =========================================================
        colors[ImGuiCol_TitleBg]          = mantle;
        colors[ImGuiCol_TitleBgActive]    = surface0;
        colors[ImGuiCol_TitleBgCollapsed] = mantle;
        colors[ImGuiCol_MenuBarBg]        = mantle;
    
        // =========================================================
        // Scrollbars
        // =========================================================
        colors[ImGuiCol_ScrollbarBg]          = surface0;
        colors[ImGuiCol_ScrollbarGrab]        = surface2;
        colors[ImGuiCol_ScrollbarGrabHovered] = overlay0;
        colors[ImGuiCol_ScrollbarGrabActive]  = overlay2;
    
        // =========================================================
        // Tabs
        // =========================================================
        colors[ImGuiCol_Tab]                = surface0;
        colors[ImGuiCol_TabHovered]         = surface2;
        colors[ImGuiCol_TabActive]          = surface1;
        colors[ImGuiCol_TabUnfocused]       = surface0;
        colors[ImGuiCol_TabUnfocusedActive] = surface1;
    
        // =========================================================
        // Accents / Feedback
        // =========================================================
        colors[ImGuiCol_CheckMark]        = green;
        colors[ImGuiCol_SliderGrab]       = sapphire;
        colors[ImGuiCol_SliderGrabActive] = blue;
    
        colors[ImGuiCol_Separator]        = surface1;
        colors[ImGuiCol_SeparatorHovered] = mauve;
        colors[ImGuiCol_SeparatorActive]  = mauve;
    
        colors[ImGuiCol_ResizeGrip]       = surface2;
        colors[ImGuiCol_ResizeGripHovered]= mauve;
        colors[ImGuiCol_ResizeGripActive] = mauve;
    
        // =========================================================
        // Tables & Misc
        // =========================================================
        colors[ImGuiCol_TableHeaderBg]    = surface0;
        colors[ImGuiCol_TableBorderStrong]= surface1;
        colors[ImGuiCol_TableBorderLight] = surface0;
    
        colors[ImGuiCol_TableRowBg]       = ImVec4(0,0,0,0);
        colors[ImGuiCol_TableRowBgAlt]    = ImVec4(1,1,1,0.06f);
    
        colors[ImGuiCol_TextSelectedBg]   = surface2;
        colors[ImGuiCol_DragDropTarget]   = yellow;
    
        colors[ImGuiCol_NavHighlight]         = blue;
        colors[ImGuiCol_NavWindowingHighlight]= ImVec4(1,1,1,0.7f);
        colors[ImGuiCol_NavWindowingDimBg]    = ImVec4(0.8f,0.8f,0.8f,0.2f);
        colors[ImGuiCol_ModalWindowDimBg]     = ImVec4(0,0,0,0.35f);
        
#ifdef IMGUI_HAS_DOCK
        colors[ImGuiCol_DockingPreview] = ImVec4(0.71f, 0.75f, 1.00f, 0.50f);
        colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.12f, 0.12f, 0.18f, 1.00f);
#endif
	}
	
    void ImGuiUtils::SetTheme_CatppuccinMocha()
    {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;
    
        // =========================================================
        // Layout
        // =========================================================
        style.WindowRounding    = 6.0f;
        style.ChildRounding     = 6.0f;
        style.FrameRounding     = 4.0f;
        style.PopupRounding     = 4.0f;
        style.ScrollbarRounding = 9.0f;
        style.GrabRounding      = 4.0f;
        style.TabRounding       = 4.0f;
    
        style.WindowPadding     = ImVec2(8.0f, 8.0f);
        style.FramePadding      = ImVec2(5.0f, 3.0f);
        style.ItemSpacing       = ImVec2(8.0f, 4.0f);
        style.ItemInnerSpacing  = ImVec2(4.0f, 4.0f);
    
        style.IndentSpacing     = 21.0f;
        style.ScrollbarSize     = 14.0f;
        style.GrabMinSize       = 10.0f;
    
        // Borders
        style.WindowBorderSize  = 1.0f;
        style.ChildBorderSize   = 1.0f;
        style.PopupBorderSize   = 1.0f;
        style.FrameBorderSize   = 0.0f;
        style.TabBorderSize     = 0.0f;
    
        // =========================================================
        // Catppuccin Mocha Palette
        // =========================================================
    
        // --- Base layers ---
        const ImVec4 base		= HexToImVec4("#1e1e2e");
        const ImVec4 mantle		= HexToImVec4("#181825");
        const ImVec4 crust		= HexToImVec4("#11111b");
        const ImVec4 surface0	= HexToImVec4("#313244");
        const ImVec4 surface1	= HexToImVec4("#45475a");
        const ImVec4 surface2	= HexToImVec4("#585b70");
    
        // --- Overlay / UI chrome ---
        const ImVec4 overlay0	= HexToImVec4("#6c7086");
        const ImVec4 overlay1	= HexToImVec4("#7f849c");
        const ImVec4 overlay2	= HexToImVec4("#9399b2");
    
        // --- Text hierarchy ---
        const ImVec4 text		= HexToImVec4("#cdd6f4");
        const ImVec4 subtext0	= HexToImVec4("#a6adc8");
        const ImVec4 subtext1	= HexToImVec4("#bac2de");
    
        // --- Accent colors ---
        const ImVec4 rosewater	= HexToImVec4("#f5e0dc");
        const ImVec4 flamingo	= HexToImVec4("#f2cdcd");
        const ImVec4 pink		= HexToImVec4("#f5c2e7");
        const ImVec4 mauve		= HexToImVec4("#cba6f7");
        const ImVec4 red		= HexToImVec4("#f38ba8");
        const ImVec4 maroon		= HexToImVec4("#eba0ac");
        const ImVec4 peach		= HexToImVec4("#fab387");
        const ImVec4 yellow		= HexToImVec4("#f9e2af");
        const ImVec4 green		= HexToImVec4("#a6e3a1");
        const ImVec4 teal		= HexToImVec4("#94e2d5");
        const ImVec4 sky		= HexToImVec4("#89dceb");
        const ImVec4 sapphire	= HexToImVec4("#74c7ec");
        const ImVec4 blue		= HexToImVec4("#89b4fa");
        const ImVec4 lavender	= HexToImVec4("#b4befe");
        
    
        // =========================================================
        // Core UI
        // =========================================================
        colors[ImGuiCol_WindowBg]  = base;
        colors[ImGuiCol_ChildBg]   = mantle;
        colors[ImGuiCol_PopupBg]   = crust;
    
        colors[ImGuiCol_Border]        = surface1;
        colors[ImGuiCol_BorderShadow]  = ImVec4(0,0,0,0);
    
        // =========================================================
        // Text
        // =========================================================
        colors[ImGuiCol_Text]         = text;
        colors[ImGuiCol_TextDisabled] = subtext0;
    
        // =========================================================
        // Interactive Elements (Frame-based widgets)
        // =========================================================
        colors[ImGuiCol_FrameBg]         = surface0;
        colors[ImGuiCol_FrameBgHovered]  = surface1;
        colors[ImGuiCol_FrameBgActive]   = surface2;
    	
    	colors[ImGuiCol_Button]        = ImVec4(blue.x, blue.y, blue.z, 0.10f);
    	colors[ImGuiCol_ButtonHovered] = ImVec4(blue.x, blue.y, blue.z, 0.22f);
    	colors[ImGuiCol_ButtonActive]  = ImVec4(blue.x, blue.y, blue.z, 0.35f);
    
        colors[ImGuiCol_Header]          = surface0;
        colors[ImGuiCol_HeaderHovered]   = surface1;
        colors[ImGuiCol_HeaderActive]    = surface2;
    
        // =========================================================
        // Titles & Menu
        // =========================================================
        colors[ImGuiCol_TitleBg]          = mantle;
        colors[ImGuiCol_TitleBgActive]    = surface0;
        colors[ImGuiCol_TitleBgCollapsed] = mantle;
        colors[ImGuiCol_MenuBarBg]        = mantle;
    
        // =========================================================
        // Scrollbars
        // =========================================================
        colors[ImGuiCol_ScrollbarBg]          = surface0;
        colors[ImGuiCol_ScrollbarGrab]        = surface2;
        colors[ImGuiCol_ScrollbarGrabHovered] = overlay0;
        colors[ImGuiCol_ScrollbarGrabActive]  = overlay2;
    
        // =========================================================
        // Tabs
        // =========================================================
        colors[ImGuiCol_Tab]                = surface0;
        colors[ImGuiCol_TabHovered]         = surface2;
        colors[ImGuiCol_TabActive]          = surface1;
        colors[ImGuiCol_TabUnfocused]       = surface0;
        colors[ImGuiCol_TabUnfocusedActive] = surface1;
    
        // =========================================================
        // Accents / Feedback
        // =========================================================
        colors[ImGuiCol_CheckMark]        = green;
        colors[ImGuiCol_SliderGrab]       = sapphire;
        colors[ImGuiCol_SliderGrabActive] = blue;
    
        colors[ImGuiCol_Separator]        = surface1;
        colors[ImGuiCol_SeparatorHovered] = mauve;
        colors[ImGuiCol_SeparatorActive]  = mauve;
    
        colors[ImGuiCol_ResizeGrip]       = surface2;
        colors[ImGuiCol_ResizeGripHovered]= mauve;
        colors[ImGuiCol_ResizeGripActive] = mauve;
    
        // =========================================================
        // Tables & Misc
        // =========================================================
        colors[ImGuiCol_TableHeaderBg]    = surface0;
        colors[ImGuiCol_TableBorderStrong]= surface1;
        colors[ImGuiCol_TableBorderLight] = surface0;
    
        colors[ImGuiCol_TableRowBg]       = ImVec4(0,0,0,0);
        colors[ImGuiCol_TableRowBgAlt]    = ImVec4(1,1,1,0.06f);
    
        colors[ImGuiCol_TextSelectedBg]   = surface2;
        colors[ImGuiCol_DragDropTarget]   = yellow;
    
        colors[ImGuiCol_NavHighlight]         = blue;
        colors[ImGuiCol_NavWindowingHighlight]= ImVec4(1,1,1,0.7f);
        colors[ImGuiCol_NavWindowingDimBg]    = ImVec4(0.8f,0.8f,0.8f,0.2f);
        colors[ImGuiCol_ModalWindowDimBg]     = ImVec4(0,0,0,0.35f);
        
#ifdef IMGUI_HAS_DOCK
        colors[ImGuiCol_DockingPreview] = ImVec4(0.71f, 0.75f, 1.00f, 0.50f);
        colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.12f, 0.12f, 0.18f, 1.00f);
#endif
    }
}
