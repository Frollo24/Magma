project "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("%{wks.location}/" .. outputbindir .. "/%{prj.name}")
	objdir ("%{wks.location}/" .. outputintdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"src/",
		"%{wks.location}/MagmaEngine/src/",
		"%{wks.location}/MagmaEngine/vendor/spdlog/include/",
		"%{wks.location}/MagmaEngine/vendor/glm/"
	}

	links
	{
		"MagmaEngine"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"MGM_PLATFORM_WINDOWS"
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
		kind "WindowedApp"
		defines "MGM_DIST"
		runtime "Release"
		optimize "On"
		symbols "Off"