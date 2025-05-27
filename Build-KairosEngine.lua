include "ProjectDependencies.lua"

workspace "KairosEngine"
	architecture "x86_64"
	startproject "Editor"

configurations
{
	"Debug",
	"Release",
	"Dist"
}

flags
{
	"MultiProcessorCompile"
}

OutputDir = "%{cfg.buildcfg}-%{cfg.system}"

--Project Build Directories
ProjectBinDir = "%{wks.location}/Binaries/" .. OutputDir .. "/%{prj.name}"
ProjectIntDir = "%{wks.location}/Intermediate/" .. OutputDir .. "/%{prj.name}"

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

group "Dependencies"
	include "Engine/Dependencies/Glad"
	include "Engine/Dependencies/ImGui"
group ""

group "Misc"
group ""