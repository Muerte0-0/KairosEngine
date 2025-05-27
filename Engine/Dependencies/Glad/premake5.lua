project "Glad"
kind "StaticLib"
language "C"
staticruntime "on"

targetdir (ThirdPartyBinDir)
objdir (ThirdPartyIntDir)

files
{
	"include/**.h",
	"src/**.c"
}

includedirs
{
	"include"
}

filter "system:windows"
systemversion "latest"

defines
{
	"_CRT_SECURE_NO_WARNINGS"
}

filter "configurations:Debug"
	runtime "Debug"
	symbols "on"

filter "configurations:Release"
	runtime "Release"
	optimize "on"

filter "configurations:Dist"
	runtime "Release"
	optimize "on"
	symbols "off"
