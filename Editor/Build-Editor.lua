project "Editor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

targetdir (ProjectBinDir)
objdir (ProjectIntDir)

files {
	"Source/**.h",
	"Source/**.cpp",
	"Resources/Shaders/**.slang",
}

externalincludedirs {
	"%{wks.location}/Engine/Source",
	"%{wks.location}/ThirdParty/",
	"%{IncludeDir.GLFW}",
}

links {
	"Engine",
}

filter "system:windows"
	systemversion "latest"

links {
	"Ws2_32.lib",
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