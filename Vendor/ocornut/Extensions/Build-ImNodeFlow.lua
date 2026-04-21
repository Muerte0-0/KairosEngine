project "ImNodeFlow"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	warnings "off"

targetdir (ThirdPartyBinDir)
objdir (ThirdPartyIntDir)

defines {
	"IMGUI_DEFINE_MATH_OPERATORS",
}

files
{
	"ImNodeFlow/src/ImNodeFlow.cpp",
	"ImNodeFlow/include/ImNodeFlow.h",
	"ImNodeFlow/src/ImNodeFlow.inl",
	"ImNodeFlow/src/imgui_bezier_math.h",
	"ImNodeFlow/src/imgui_bezier_math.inl",
	"ImNodeFlow/src/imgui_extra_math.h",
	"ImNodeFlow/src/imgui_extra_math.inl",
	"ImNodeFlow/src/context_wrapper.h",
}

includedirs
{
	"ImNodeFlow/include",
	"ImNodeFlow/src",
	"%{IncludeDir.ImGui}",
}

filter "system:windows"
	systemversion "latest"
	defines { "_CRT_SECURE_NO_WARNINGS" }

filter "configurations:Debug"
	runtime "Debug"
	symbols "on"

filter "configurations:Release"
	runtime "Release"
	optimize "on"

filter "configurations:Dist"
	runtime "Release"
	optimize "on"
