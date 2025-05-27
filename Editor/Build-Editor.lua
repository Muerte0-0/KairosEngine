project "Editor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

targetdir (ProjectBinDir)
objdir (ProjectIntDir)

files
{
	"Source/**.h",
	"Source/**.cpp"
}

externalincludedirs
{
	"%{IncludeDir.spdlog}",
	"%{wks.location}/Engine/Source",
	"%{wks.location}/Engine/Dependencies/Include",
	"%{IncludeDir.Glm}"
}

links
{
	"Engine",

	"%{Library.Vulkan}"
}

filter "system:windows"
	systemversion "latest"
links
{
	"%{Library.WinSock}",
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