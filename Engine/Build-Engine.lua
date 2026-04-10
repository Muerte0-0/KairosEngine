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
	"VK_NO_PROTOTYPES",
	"SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE",
	"GLFW_INCLUDE_NONE",
	"GLM_FORCE_RADIANS",
	"GLM_FORCE_DEPTH_ZERO_TO_ONE",
	"GLM_ENABLE_EXPERIMENTAL",
}

files {
	"Source/**.h",
	"Source/**.cpp",
}

includedirs {
	"%{IncludeDir.spdlog}",
	"Source",
	"%{IncludeDir.GLFW}",
	"%{IncludeDir.Vulkan}",
	"%{IncludeDir.ImGui}",
	"%{IncludeDir.GLM}",
	"%{IncludeDir.stb_image}",
	"%{IncludeDir.EnTT}",
	"%{IncludeDir.Assimp}",
	"%{IncludeDir.AssimpConfig}",
	"%{IncludeDir.JSON}",
	"%{IncludeDir.mINI}",
}

links {
	"GLFW",
	"ImGui",
	"Assimp",
}

filter "files:**.c"
    enablepch = "OFF"  -- Skip PCH for C files
	compileas "C"
	
filter "files:Vendor/ocornut/ImGuizmo/**.cpp"
	enablepch = "OFF"  -- Skip PCH for ImGuizmo files

filter "system:windows"
	systemversion "latest"
	
defines {
	"WIN32_LEAN_AND_MEAN",
}

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
	"%{VulkanSDK}/lib/vulkan"
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
