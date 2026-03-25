project "ShaderCompiler"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

targetdir (ToolsBinDir)
objdir (ToolsIntDir)

files {
	"Source/**.h",
	"Source/**.cpp",
}

externalincludedirs {
	
}

filter "configurations:Debug"
        defines { "DEBUG", "_DEBUG", "KE_DEBUG" }
        symbols "On"
        runtime "Debug"
        optimize "Off"

filter "configurations:Release"
    defines { "NDEBUG", "KE_RELEASE" }
	linktimeoptimization "On"
    symbols "On"
    runtime "Release"
    optimize "Speed"
	

filter "configurations:Dist"
    defines { "NDEBUG", "KE_DIST" }
	linktimeoptimization "On"
    symbols "Off"
    runtime "Release"
    optimize "Speed"