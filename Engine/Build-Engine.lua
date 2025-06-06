project "Engine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

targetdir (ProjectBinDir)
objdir (ProjectIntDir)

pchheader "kepch.h"
pchsource "Source/kepch.cpp"

defines
{
	"VK_NO_PROTOTYPES",
	"_CRT_SECURE_NO_WARNINGS",
	"GLFW_INCLUDE_NONE",
	"SLANG_STATIC",
}

files
{
	"Source/**.h",
	"Source/**.cpp",
	"%{IncludeDir.Volk}/volk.c",
}

includedirs
{
	"Source",
	"%{IncludeDir.Volk}",
	"%{IncludeDir.Vulkan}",
	"%{IncludeDir.spdlog}",
	"%{IncludeDir.GLFW}",
	"%{IncludeDir.stb_image}",
	"%{IncludeDir.ImGui}",
	"%{IncludeDir.Glad}",
	"%{IncludeDir.Glm}",
	"%{IncludeDir.Slang}",
}

links
{
	"%{Library.GLFW}",
	"%{Library.Slang}",
	"ImGui",
	"Glad",
}

filter "files:**.c"
    flags { "NoPCH" }  -- Skip PCH for C files
	compileas "C"
	
--filter "files:vendor/ImGuizmo/**.cpp"
	--flags { "NoPCH" }

filter "system:windows"
	systemversion "latest"

links
{
	"Ws2_32.lib",
	"dxgi",
	"gdi32",
}

filter "system:linux"
	systemversion "latest"

links
{
	"X11",
}

filter "configurations:Debug"
	defines 
	{
		"KE_DEBUG",
		"VK_ENABLE_VALIDATION_LAYERS"
	}
	runtime "Debug"
	symbols "on"

links
{
	"%{Library.ShaderC_Combined_Debug}"
}

filter "configurations:Release"
	defines "KE_RELEASE"
	runtime "Release"
	optimize "on"

links
{
	"%{Library.ShaderC_Combined_Release}"
}

filter "configurations:Dist"
	defines "KE_DIST"
	runtime "Release"
	optimize "on"