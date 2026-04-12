include "ProjectDependencies.lua"

workspace "KairosEngine"
	architecture "x86_64"
	startproject "Sandbox"
	configurations { "Debug", "Release", "Dist" }

filter "action:vs*"
	buildoptions { "/utf-8", }
	linkoptions { "/IGNORE:4006" }

linktimeoptimization ("Default")
multiprocessorcompile ("On")

OutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

--Project Build Directories
ProjectBinDir = "%{wks.location}/Binaries/" .. OutputDir .. "/%{prj.name}"
ProjectIntDir = "%{wks.location}/Intermediate/" .. OutputDir .. "/%{prj.name}"

--Project Build Directories
ToolsBinDir = "%{wks.location}/Binaries/" .. OutputDir .. "/Tools/%{prj.name}"
ToolsIntDir = "%{wks.location}/Intermediate/" .. OutputDir .. "/Tools/%{prj.name}"

--Third Party Libraries Build Directories
ThirdPartyBinDir = "%{wks.location}/Binaries/" .. OutputDir .. "/ThirdParty/%{prj.name}"
ThirdPartyIntDir = "%{wks.location}/Intermediate/" .. OutputDir .. "/ThirdParty/%{prj.name}"

group "Apps"
	include "Editor/Build-Editor.lua"
	--include "ProjectBrowser/Build-ProjectBrowser.lua"
group ""

group "Core"
	include "Engine/Build-Engine.lua"
group ""

group "Tools"
	include "Tools/ShaderCompiler/Build-ShaderCompiler.lua"
group ""

group "ThirdParty"
	include "Vendor/GLFW/Build-GLFW.lua"
	include "Vendor/ocornut/Build-ImGui.lua"
	include "Vendor/Assimp/Build-Assimp.lua"
	include "Vendor/YAML-CPP/Build-YAML-CPP.lua"
group ""

group "Misc"
	
group ""