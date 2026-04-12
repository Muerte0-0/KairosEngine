project "Editor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

targetdir (ProjectBinDir)
objdir (ProjectIntDir)

defines {
	"GLM_ENABLE_EXPERIMENTAL",
}

files {
	"Source/**.h",
	"Source/**.cpp",
	"Resources/Shaders/**.slang",
	"Resources/Shaders/**.spv",
}

externalincludedirs {
	"%{IncludeDir.spdlog}",
	"%{wks.location}/Engine/Source",
	"%{wks.location}/ThirdParty/",
	"%{IncludeDir.GLFW}",
	"%{IncludeDir.ImGui}",
	"%{IncludeDir.ImGuizmo}",
	"%{IncludeDir.GLM}",
	"%{IncludeDir.EnTT}",
}

links {
	"Engine",
	"GLFW",
	"ImGui",
	"Assimp",
	"YAML-CPP",
}

filter "system:windows"
	systemversion "latest"

links {
	"Ws2_32.lib",
}

filter "system:linux"
systemversion "latest"

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
