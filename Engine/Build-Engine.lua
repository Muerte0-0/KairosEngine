project "Engine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

targetdir (ProjectBinDir)
objdir (ProjectIntDir)

pchheader "kepch.h"
pchsource "Source/kepch.cpp"

defines {
	"_CRT_SECURE_NO_WARNINGS",
}

files {
	"Source/**.h",
	"Source/**.cpp",
}

includedirs {
	"Source",
	"%{IncludeDir.GLFW}",
	"%{IncludeDir.Vulkan}",
	"%{IncludeDir.ImGui}",
	"%{IncludeDir.GLM}",
}

links {
	"GLFW",
	"ImGui",
}

filter "files:**.c"
    enablepch = "OFF"  -- Skip PCH for C files
	compileas "C"
	
filter "files:Vendor/ocornut/ImGuizmo/**.cpp"
	enablepch = "OFF"  -- Skip PCH for ImGuizmo files

filter "system:windows"
	systemversion "latest"

links {
	"Ws2_32.lib",
	"dxgi",
	"d3d11",
	"d3d12",
	"d3dcompiler",
	"gdi32",
	"%{Library.Vulkan}",
}

filter "system:linux"
	systemversion "latest"

links {
	"X11",
	"vulkan",
}

filter "configurations:Debug"
	defines "KE_DEBUG"
	runtime "Debug"
	symbols "on"

filter "configurations:Release"
	defines "KE_RELEASE"
	runtime "Release"
	optimize "on"

filter "configurations:Dist"
	defines "KE_DIST"
	runtime "Release"
	optimize "on"