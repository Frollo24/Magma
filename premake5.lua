workspace "Magma"
	architecture "x64"
	startproject "Sandbox"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputbindir = "Build/Binaries/%{cfg.buildcfg}-%{cfg.architecture}"
outputintdir = "Build/Intermediates/%{cfg.buildcfg}-%{cfg.architecture}"

include "MagmaEngine"
include "Sandbox"