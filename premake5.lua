include "dependencies.lua"

workspace "Magma"
	architecture "x64"
	startproject "Sandbox"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.architecture}"
outputbindir = "Build/Binaries/%{cfg.buildcfg}-%{cfg.architecture}"
outputintdir = "Build/Intermediates/%{cfg.buildcfg}-%{cfg.architecture}"

group "Dependencies"
	include "MagmaEngine/vendor/GLFW"
group ""

include "MagmaEngine"
include "Sandbox"