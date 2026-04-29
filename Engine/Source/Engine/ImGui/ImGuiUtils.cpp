#include "kepch.h"
#include "ImGuiUtils.h"

#include "imgui.h"
#include "Engine/Utils/PlatformUtils.h"

namespace Engine
{
	// -----------------------------------------------------------------------
	// Color helpers
	// -----------------------------------------------------------------------

	static float SRGBToLinear(float c)
	{
		if (c <= 0.04045f)
			return c / 12.92f;
		return powf((c + 0.055f) / 1.055f, 2.4f);
	}

	static ImVec4 HexToImVec4(const std::string& hex)
	{
		std::string clean = hex;
		if (clean[0] == '#')
			clean = clean.substr(1);

		const uint32_t value = std::stoul(clean, nullptr, 16);

		if (clean.length() == 6)
		{
			const float r = static_cast<float>((value >> 16) & 0xFF) / 255.0f;
			const float g = static_cast<float>((value >>  8) & 0xFF) / 255.0f;
			const float b = static_cast<float>((value)       & 0xFF) / 255.0f;
			return { SRGBToLinear(r), SRGBToLinear(g), SRGBToLinear(b), 1.0f };
		}

		if (clean.length() == 8)
		{
			const float r = static_cast<float>((value >> 24) & 0xFF) / 255.0f;
			const float g = static_cast<float>((value >> 16) & 0xFF) / 255.0f;
			const float b = static_cast<float>((value >>  8) & 0xFF) / 255.0f;
			const float a = static_cast<float>((value)       & 0xFF) / 255.0f;
			return { SRGBToLinear(r), SRGBToLinear(g), SRGBToLinear(b), a };
		}

		return { 1.0f, 0.0f, 0.0f, 1.0f }; // Fallback: bright red — easy to spot.
	}

	// -----------------------------------------------------------------------
	// Font loading
	// -----------------------------------------------------------------------

	namespace
	{
		/**
		 * @brief Resolve the engine's Resources/Fonts directory.
		 *
		 * Walks up from the executable to the workspace root
		 * (the directory containing KairosEngine-Setup.lua) then appends
		 * the well-known relative path.  Returns an empty path on failure
		 * so callers can fall back gracefully to ImGui's built-in font.
		 */
		std::filesystem::path ResolveFontDirectory()
		{
			const auto root = PlatformUtils::ResolveWorkspaceRoot();
			if (!root)
			{
				LOG(LogLevel::Warning, "ImGuiUtils: could not resolve workspace root — using built-in ImGui font.");
				return {};
			}

			return *root / "Engine" / "Resources" / "Fonts";
		}

		void LoadFonts(float fontSize)
		{
			ImGuiIO& io = ImGui::GetIO();
			const std::filesystem::path fontsDir = ResolveFontDirectory();

			if (fontsDir.empty())
				return; // ImGui will use its default font.
			
			const std::filesystem::path regularPath = fontsDir / "JetbrainsMono" / "JetBrainsMonoNerdFontMono-Regular.ttf";
			const std::filesystem::path boldPath    = fontsDir / "OpenSans" / "OpenSans-Bold.ttf";

			if (std::filesystem::exists(regularPath))
				io.FontDefault = io.Fonts->AddFontFromFileTTF(regularPath.string().c_str(), fontSize);
			else
				LOG(LogLevel::Warning, "ImGuiUtils: font not found at '{}'", regularPath.string());

			if (std::filesystem::exists(boldPath))
				io.Fonts->AddFontFromFileTTF(boldPath.string().c_str(), fontSize);
			else
				LOG(LogLevel::Warning, "ImGuiUtils: font not found at '{}'", boldPath.string());
		}
	}

	// -----------------------------------------------------------------------
	// Public API
	// -----------------------------------------------------------------------

	void ImGuiUtils::SetImGuiStyle(Theme theme)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		io.ConfigFlags |= ImGuiConfigFlags_IsSRGB;

		LoadFonts(16.0f);

		ImGuiStyle& style = ImGui::GetStyle();

		// =========================================================
		// Layout
		// =========================================================
		style.WindowRounding    	= 6.0f;		style.ChildRounding  = 6.0f;
		style.FrameRounding     	= 4.0f;		style.PopupRounding  = 4.0f;
		style.ScrollbarRounding 	= 9.0f;		style.GrabRounding   = 4.0f;
		style.TabRounding       	= 4.0f;

		style.WindowPadding			= ImVec2(8.0f, 8.0f);
		style.FramePadding			= ImVec2(4.0f, 4.0f);
		style.ItemSpacing			= ImVec2(4.0f, 4.0f);
		style.ItemInnerSpacing		= ImVec2(4.0f, 4.0f);
		style.IndentSpacing			= 21.0f;
		style.ScrollbarSize			= 14.0f;
		style.GrabMinSize			= 10.0f;

		style.WindowBorderSize		= 1.0f;		style.ChildBorderSize = 1.0f;
		style.PopupBorderSize		= 1.0f;		style.FrameBorderSize = 0.0f;
		style.TabBorderSize			= 0.0f;

		switch (theme)
		{
		case Theme::Dark:       SetTheme_Dark();                break;
		case Theme::Light:      SetTheme_Light();               break;
		case Theme::Latte:      SetTheme_CatppuccinLatte();     break;
		case Theme::Frappé:     SetTheme_CatppuccinFrappé();    break;
		case Theme::Macchiato:  SetTheme_CatppuccinMacchiato(); break;
		case Theme::Mocha:      SetTheme_CatppuccinMocha();     break;
		default:                                                 break;
		}
	}

	// -----------------------------------------------------------------------
	// Themes
	// -----------------------------------------------------------------------

	void ImGuiUtils::SetTheme_Dark()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4*     colors = style.Colors;

		const ImVec4 bg      = ImVec4(0.010f, 0.0105f, 0.011f, 1.00f);
		const ImVec4 panel   = ImVec4(0.015f, 0.015f,  0.015f, 1.00f);
		const ImVec4 surface = ImVec4(0.018f, 0.0185f, 0.019f, 1.00f);
		const ImVec4 hover   = ImVec4(0.024f, 0.026f,  0.029f, 1.00f);
		const ImVec4 active  = ImVec4(0.020f, 0.022f,  0.025f, 1.00f);
		const ImVec4 border  = ImVec4(0.028f, 0.028f,  0.026f, 1.00f);
		const ImVec4 text    = ImVec4(0.86f,  0.87f,   0.88f,  1.00f);
		const ImVec4 textDim = ImVec4(0.55f,  0.56f,   0.58f,  1.00f);
		const ImVec4 accent  = ImVec4(0.026f, 0.052f,  0.085f, 1.00f);

		colors[ImGuiCol_Text]             = text;
		colors[ImGuiCol_TextDisabled]     = textDim;
		colors[ImGuiCol_WindowBg]         = bg;
		colors[ImGuiCol_ChildBg]          = ImVec4(0,0,0,0);
		colors[ImGuiCol_PopupBg]          = panel;
		colors[ImGuiCol_Border]           = border;
		colors[ImGuiCol_BorderShadow]     = ImVec4(0,0,0,0);
		colors[ImGuiCol_Separator]        = border;
		colors[ImGuiCol_SeparatorHovered] = accent;
		colors[ImGuiCol_SeparatorActive]  = accent;
		colors[ImGuiCol_FrameBg]          = surface;
		colors[ImGuiCol_FrameBgHovered]   = hover;
		colors[ImGuiCol_FrameBgActive]    = active;
		colors[ImGuiCol_TitleBg]          = panel;
		colors[ImGuiCol_TitleBgActive]    = surface;
		colors[ImGuiCol_TitleBgCollapsed] = panel;
		colors[ImGuiCol_MenuBarBg]        = panel;
		colors[ImGuiCol_ScrollbarBg]          = bg;
		colors[ImGuiCol_ScrollbarGrab]        = surface;
		colors[ImGuiCol_ScrollbarGrabHovered] = hover;
		colors[ImGuiCol_ScrollbarGrabActive]  = active;
		colors[ImGuiCol_CheckMark]        = accent;
		colors[ImGuiCol_SliderGrab]       = accent;
		colors[ImGuiCol_SliderGrabActive] = accent;
		colors[ImGuiCol_Button]           = ImVec4(accent.x, accent.y, accent.z, 0.10f);
		colors[ImGuiCol_ButtonHovered]    = ImVec4(accent.x, accent.y, accent.z, 0.25f);
		colors[ImGuiCol_ButtonActive]     = ImVec4(accent.x, accent.y, accent.z, 0.40f);
		colors[ImGuiCol_Header]           = ImVec4(accent.x, accent.y, accent.z, 0.12f);
		colors[ImGuiCol_HeaderHovered]    = ImVec4(accent.x, accent.y, accent.z, 0.25f);
		colors[ImGuiCol_HeaderActive]     = ImVec4(accent.x, accent.y, accent.z, 0.45f);
		colors[ImGuiCol_TableHeaderBg]      = panel;
		colors[ImGuiCol_TableBorderStrong]  = border;
		colors[ImGuiCol_TableBorderLight]   = surface;
		colors[ImGuiCol_TableRowBgAlt]      = ImVec4(1,1,1,0.03f);
		colors[ImGuiCol_Tab]                = panel;
		colors[ImGuiCol_TabHovered]         = hover;
		colors[ImGuiCol_TabActive]          = surface;
		colors[ImGuiCol_TabUnfocused]       = panel;
		colors[ImGuiCol_TabUnfocusedActive] = surface;
		colors[ImGuiCol_PlotLines]          = accent;
		colors[ImGuiCol_PlotHistogram]      = accent;
		colors[ImGuiCol_TextSelectedBg]     = ImVec4(accent.x, accent.y, accent.z, 0.35f);
		colors[ImGuiCol_DragDropTarget]     = accent;
		colors[ImGuiCol_NavHighlight]       = accent;
#ifdef IMGUI_HAS_DOCK
		colors[ImGuiCol_DockingPreview] = ImVec4(accent.x, accent.y, accent.z, 0.40f);
		colors[ImGuiCol_DockingEmptyBg] = bg;
#endif
	}

	void ImGuiUtils::SetTheme_Light()
	{
		ImGuiStyle& style  = ImGui::GetStyle();
		ImVec4*     colors = style.Colors;

		colors[ImGuiCol_Text]             = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
		colors[ImGuiCol_TextDisabled]     = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
		colors[ImGuiCol_WindowBg]         = ImVec4(0.96f, 0.96f, 0.94f, 1.00f);
		colors[ImGuiCol_ChildBg]          = ImVec4(0.00f, 0.00f, 0.00f, 0.03f);
		colors[ImGuiCol_PopupBg]          = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_Border]           = ImVec4(0.75f, 0.75f, 0.72f, 1.00f);
		colors[ImGuiCol_BorderShadow]     = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_Separator]        = ImVec4(0.80f, 0.80f, 0.78f, 1.00f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.17f, 0.34f, 0.59f, 0.78f);
		colors[ImGuiCol_SeparatorActive]  = ImVec4(0.17f, 0.34f, 0.59f, 1.00f);
		colors[ImGuiCol_FrameBg]          = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_FrameBgHovered]   = ImVec4(0.90f, 0.92f, 0.95f, 1.00f);
		colors[ImGuiCol_FrameBgActive]    = ImVec4(0.85f, 0.88f, 0.92f, 1.00f);
		colors[ImGuiCol_TitleBg]          = ImVec4(0.92f, 0.92f, 0.90f, 1.00f);
		colors[ImGuiCol_TitleBgActive]    = ImVec4(0.88f, 0.88f, 0.86f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.92f, 0.92f, 0.90f, 0.75f);
		colors[ImGuiCol_MenuBarBg]        = ImVec4(0.92f, 0.92f, 0.90f, 1.00f);
		colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.96f, 0.96f, 0.94f, 1.00f);
		colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.80f, 0.80f, 0.78f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.70f, 0.70f, 0.68f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.60f, 0.60f, 0.58f, 1.00f);
		colors[ImGuiCol_CheckMark]        = ImVec4(0.17f, 0.34f, 0.59f, 1.00f);
		colors[ImGuiCol_SliderGrab]       = ImVec4(0.17f, 0.34f, 0.59f, 0.70f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.17f, 0.34f, 0.59f, 1.00f);
		colors[ImGuiCol_Button]           = ImVec4(0.17f, 0.34f, 0.59f, 0.08f);
		colors[ImGuiCol_ButtonHovered]    = ImVec4(0.17f, 0.34f, 0.59f, 0.20f);
		colors[ImGuiCol_ButtonActive]     = ImVec4(0.17f, 0.34f, 0.59f, 0.35f);
		colors[ImGuiCol_Header]           = ImVec4(0.17f, 0.34f, 0.59f, 0.12f);
		colors[ImGuiCol_HeaderHovered]    = ImVec4(0.17f, 0.34f, 0.59f, 0.25f);
		colors[ImGuiCol_HeaderActive]     = ImVec4(0.17f, 0.34f, 0.59f, 0.40f);
		colors[ImGuiCol_TableHeaderBg]      = ImVec4(0.90f, 0.90f, 0.88f, 1.00f);
		colors[ImGuiCol_TableBorderStrong]  = ImVec4(0.75f, 0.75f, 0.72f, 1.00f);
		colors[ImGuiCol_TableBorderLight]   = ImVec4(0.85f, 0.85f, 0.82f, 1.00f);
		colors[ImGuiCol_TableRowBgAlt]      = ImVec4(0.00f, 0.00f, 0.00f, 0.03f);
		colors[ImGuiCol_Tab]                = ImVec4(0.92f, 0.92f, 0.90f, 1.00f);
		colors[ImGuiCol_TabHovered]         = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TabActive]          = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TabUnfocused]       = ImVec4(0.92f, 0.92f, 0.90f, 1.00f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.96f, 0.96f, 0.94f, 1.00f);
		colors[ImGuiCol_PlotLines]          = ImVec4(0.17f, 0.34f, 0.59f, 1.00f);
		colors[ImGuiCol_PlotHistogram]      = ImVec4(0.17f, 0.34f, 0.59f, 1.00f);
		colors[ImGuiCol_TextSelectedBg]     = ImVec4(0.17f, 0.34f, 0.59f, 0.25f);
		colors[ImGuiCol_DragDropTarget]     = ImVec4(0.17f, 0.34f, 0.59f, 0.90f);
		colors[ImGuiCol_NavHighlight]       = ImVec4(0.17f, 0.34f, 0.59f, 1.00f);
#ifdef IMGUI_HAS_DOCK
		colors[ImGuiCol_DockingPreview] = ImVec4(0.17f, 0.34f, 0.59f, 0.40f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.96f, 0.96f, 0.94f, 1.00f);
#endif
	}

	// -----------------------------------------------------------------------
	// Catppuccin helper macro — all four flavors share the same structure;
	// only the palette values differ.
	// -----------------------------------------------------------------------

#define KE_CATPPUCCIN_APPLY_COLORS()																	\
	colors[ImGuiCol_WindowBg]					= base;   colors[ImGuiCol_ChildBg]  = mantle;			\
	colors[ImGuiCol_PopupBg]					= crust;												\
	colors[ImGuiCol_Border]						= surface1;												\
	colors[ImGuiCol_BorderShadow]				= ImVec4(0,0,0,0);										\
	colors[ImGuiCol_Text]						= text;													\
	colors[ImGuiCol_TextDisabled]				= subtext0;												\
	colors[ImGuiCol_FrameBg]					= surface0;												\
	colors[ImGuiCol_FrameBgHovered]				= surface1;												\
	colors[ImGuiCol_FrameBgActive]				= surface2;												\
	colors[ImGuiCol_Button]						= ImVec4(blue.x, blue.y, blue.z, 0.05f);				\
	colors[ImGuiCol_ButtonHovered]				= ImVec4(blue.x, blue.y, blue.z, 0.07f);				\
	colors[ImGuiCol_ButtonActive]				= ImVec4(blue.x, blue.y, blue.z, 0.1f);					\
	colors[ImGuiCol_Header]						= surface0;												\
	colors[ImGuiCol_HeaderHovered]				= surface1;												\
	colors[ImGuiCol_HeaderActive]				= surface2;												\
	colors[ImGuiCol_TitleBg]					= mantle;												\
	colors[ImGuiCol_TitleBgActive]				= surface0;												\
	colors[ImGuiCol_TitleBgCollapsed]			= mantle;												\
	colors[ImGuiCol_MenuBarBg]					= mantle;												\
	colors[ImGuiCol_ScrollbarBg]				= surface0;												\
	colors[ImGuiCol_ScrollbarGrab]				= surface2;												\
	colors[ImGuiCol_ScrollbarGrabHovered]		= overlay0;												\
	colors[ImGuiCol_ScrollbarGrabActive]		= overlay2;												\
	colors[ImGuiCol_Tab]						= surface0;												\
	colors[ImGuiCol_TabHovered]					= surface2;												\
	colors[ImGuiCol_TabActive]					= surface1;												\
	colors[ImGuiCol_TabUnfocused]				= surface0;												\
	colors[ImGuiCol_TabUnfocusedActive]			= surface1;												\
	colors[ImGuiCol_CheckMark]					= green;												\
	colors[ImGuiCol_SliderGrab]					= sapphire;												\
	colors[ImGuiCol_SliderGrabActive]			= blue;													\
	colors[ImGuiCol_Separator]					= surface1;												\
	colors[ImGuiCol_SeparatorHovered]			= mauve;												\
	colors[ImGuiCol_SeparatorActive]			= mauve;												\
	colors[ImGuiCol_ResizeGrip]					= surface2;												\
	colors[ImGuiCol_ResizeGripHovered]			= mauve;												\
	colors[ImGuiCol_ResizeGripActive]			= mauve;												\
	colors[ImGuiCol_TableHeaderBg]				= surface0;												\
	colors[ImGuiCol_TableBorderStrong]			= surface1;												\
	colors[ImGuiCol_TableBorderLight]			= surface0;												\
	colors[ImGuiCol_TableRowBg]					= ImVec4(0,0,0,0);										\
	colors[ImGuiCol_TableRowBgAlt]				= ImVec4(1,1,1,0.06f);									\
	colors[ImGuiCol_TextSelectedBg]				= surface2;												\
	colors[ImGuiCol_DragDropTarget]				= yellow;												\
	colors[ImGuiCol_NavHighlight]				= blue;													\
	colors[ImGuiCol_NavWindowingHighlight]		= ImVec4(1,1,1,0.7f);									\
	colors[ImGuiCol_NavWindowingDimBg]			= ImVec4(0.8f,0.8f,0.8f,0.2f);							\
	colors[ImGuiCol_ModalWindowDimBg]			= ImVec4(0,0,0,0.35f);									\
	KE_CATPPUCCIN_DOCK_COLORS()

#ifdef IMGUI_HAS_DOCK
#	define KE_CATPPUCCIN_DOCK_COLORS()																	\
		colors[ImGuiCol_DockingPreview]			= ImVec4(0.71f, 0.75f, 1.00f, 0.50f);					\
		colors[ImGuiCol_DockingEmptyBg]			= ImVec4(0.12f, 0.12f, 0.18f, 1.00f);
#else
#	define KE_CATPPUCCIN_DOCK_COLORS()
#endif

	void ImGuiUtils::SetTheme_CatppuccinLatte()
	{
		ImVec4* colors = ImGui::GetStyle().Colors;

		const ImVec4 base      = HexToImVec4("#eff1f5");
		const ImVec4 mantle    = HexToImVec4("#e6e9ef");
		const ImVec4 crust     = HexToImVec4("#dce0e8");
		const ImVec4 surface0  = HexToImVec4("#ccd0da");
		const ImVec4 surface1  = HexToImVec4("#bcc0cc");
		const ImVec4 surface2  = HexToImVec4("#acb0be");
		const ImVec4 overlay0  = HexToImVec4("#9ca0b0");
		const ImVec4 overlay2  = HexToImVec4("#7c7f93");
		const ImVec4 text      = HexToImVec4("#4c4f69");
		const ImVec4 subtext0  = HexToImVec4("#6c6f85");
		const ImVec4 mauve     = HexToImVec4("#8839ef");
		const ImVec4 yellow    = HexToImVec4("#df8e1d");
		const ImVec4 green     = HexToImVec4("#40a02b");
		const ImVec4 sapphire  = HexToImVec4("#209fb5");
		const ImVec4 blue      = HexToImVec4("#1e66f5");

		KE_CATPPUCCIN_APPLY_COLORS()
	}

	void ImGuiUtils::SetTheme_CatppuccinFrappé()
	{
		ImVec4* colors = ImGui::GetStyle().Colors;

		const ImVec4 base      = HexToImVec4("#303446");
		const ImVec4 mantle    = HexToImVec4("#292c3c");
		const ImVec4 crust     = HexToImVec4("#232634");
		const ImVec4 surface0  = HexToImVec4("#414559");
		const ImVec4 surface1  = HexToImVec4("#51576d");
		const ImVec4 surface2  = HexToImVec4("#626880");
		const ImVec4 overlay0  = HexToImVec4("#737994");
		const ImVec4 overlay2  = HexToImVec4("#949cbb");
		const ImVec4 text      = HexToImVec4("#c6d0f5");
		const ImVec4 subtext0  = HexToImVec4("#a5adce");
		const ImVec4 mauve     = HexToImVec4("#ca9ee6");
		const ImVec4 yellow    = HexToImVec4("#e5c890");
		const ImVec4 green     = HexToImVec4("#a6d189");
		const ImVec4 sapphire  = HexToImVec4("#85c1dc");
		const ImVec4 blue      = HexToImVec4("#8caaee");

		KE_CATPPUCCIN_APPLY_COLORS()
	}

	void ImGuiUtils::SetTheme_CatppuccinMacchiato()
	{
		ImVec4* colors = ImGui::GetStyle().Colors;

		const ImVec4 base      = HexToImVec4("#24273a");
		const ImVec4 mantle    = HexToImVec4("#1e2030");
		const ImVec4 crust     = HexToImVec4("#181926");
		const ImVec4 surface0  = HexToImVec4("#363a4f");
		const ImVec4 surface1  = HexToImVec4("#494d64");
		const ImVec4 surface2  = HexToImVec4("#5b6078");
		const ImVec4 overlay0  = HexToImVec4("#6e738d");
		const ImVec4 overlay2  = HexToImVec4("#939ab7");
		const ImVec4 text      = HexToImVec4("#cad3f5");
		const ImVec4 subtext0  = HexToImVec4("#a5adcb");
		const ImVec4 mauve     = HexToImVec4("#c6a0f6");
		const ImVec4 yellow    = HexToImVec4("#eed49f");
		const ImVec4 green     = HexToImVec4("#a6da95");
		const ImVec4 sapphire  = HexToImVec4("#7dc4e4");
		const ImVec4 blue      = HexToImVec4("#8aadf4");

		KE_CATPPUCCIN_APPLY_COLORS()
	}

	void ImGuiUtils::SetTheme_CatppuccinMocha()
	{
		ImGuiStyle& style  = ImGui::GetStyle();
		ImVec4*     colors = style.Colors;

		const ImVec4 base      = HexToImVec4("#1e1e2e");
		const ImVec4 mantle    = HexToImVec4("#181825");
		const ImVec4 crust     = HexToImVec4("#11111b");
		const ImVec4 surface0  = HexToImVec4("#313244");
		const ImVec4 surface1  = HexToImVec4("#45475a");
		const ImVec4 surface2  = HexToImVec4("#585b70");
		const ImVec4 overlay0  = HexToImVec4("#6c7086");
		const ImVec4 overlay2  = HexToImVec4("#9399b2");
		const ImVec4 text      = HexToImVec4("#cdd6f4");
		const ImVec4 subtext0  = HexToImVec4("#a6adc8");
		const ImVec4 mauve     = HexToImVec4("#cba6f7");
		const ImVec4 yellow    = HexToImVec4("#f9e2af");
		const ImVec4 green     = HexToImVec4("#a6e3a1");
		const ImVec4 sapphire  = HexToImVec4("#74c7ec");
		const ImVec4 blue      = HexToImVec4("#89b4fa");

		KE_CATPPUCCIN_APPLY_COLORS()
	}

#undef KE_CATPPUCCIN_APPLY_COLORS
#undef KE_CATPPUCCIN_DOCK_COLORS
}
