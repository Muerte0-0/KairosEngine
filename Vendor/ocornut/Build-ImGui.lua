project "ImGui"
kind "StaticLib"
language "C++"
cppdialect "C++20"
staticruntime "off"

targetdir (ThirdPartyBinDir)
objdir (ThirdPartyIntDir)

files
{
	"ImGui/*.cpp",
	"ImGui/*.h",
	"ImGui/backends/imgui_impl_glfw.cpp",
	"ImGui/backends/imgui_impl_dx11.cpp",
	"ImGui/backends/imgui_impl_dx12.cpp",
	"ImGui/backends/imgui_impl_vulkan.cpp",
}

includedirs
{
	"ImGui",
	"%{IncludeDir.GLFW}",
	"%{IncludeDir.Vulkan}",
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