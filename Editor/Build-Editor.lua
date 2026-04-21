project "Editor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

targetdir (ProjectBinDir)
objdir (ProjectIntDir)

defines {
	"GLM_ENABLE_EXPERIMENTAL",
	"IMGUI_DEFINE_MATH_OPERATORS",
}

files {
	"Resources/**.**",
	
	"Source/**.h",
	"Source/**.cpp",
}

externalincludedirs {
	"%{IncludeDir.spdlog}",
	"%{wks.location}/Engine/Source",
	"%{wks.location}/ThirdParty/",
	"%{IncludeDir.GLFW}",
	"%{IncludeDir.JSON}",
	"%{IncludeDir.ImGui}",
	"%{IncludeDir.ImGuizmo}",
	"%{IncludeDir.ImNodeFlow}",
	"%{IncludeDir.GLM}",
	"%{IncludeDir.EnTT}",
}

links {
	"Engine",
	"GLFW",
	"ImGui",
	"ImNodeFlow",
	"Assimp",
	"YAML-CPP",
}

filter "system:windows"
	systemversion "latest"

defines {
	"WIN32_LEAN_AND_MEAN",
}

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
