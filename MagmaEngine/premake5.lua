project "MagmaEngine"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"

	targetdir ("%{wks.location}/" .. outputbindir .. "/%{prj.name}")
	objdir ("%{wks.location}/" .. outputintdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"src",
		"vendor/spdlog/include/"
	}

	filter "system:windows"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"MGM_PLATFORM_WINDOWS",
			"MGM_BUILD_DLL"
		}

		postbuildcommands
		{
			("{COPYDIR} \"%{cfg.buildtarget.relpath}\" \"../" .. outputbindir .. "/Sandbox" .. "/\"")
		}

	filter "configurations:Debug"
		defines "MGM_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "MGM_RELEASE"
		runtime "Release"
		optimize "On"
		symbols "On"
		
	filter "configurations:Dist"
		defines "MGM_DIST"
		runtime "Release"
		optimize "On"
		symbols "Off"