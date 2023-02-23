project "MagmaEngine"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "Off"

	targetdir ("%{wks.location}/" .. outputbindir .. "/%{prj.name}")
	objdir ("%{wks.location}/" .. outputintdir .. "/%{prj.name}")

	pchheader "mgmpch.h"
	pchsource "src/mgmpch.cpp"

	files
	{
		"src/**.h",
		"src/**.cpp",
		"vendor/glm/glm/**.hpp",
		"vendor/glm/glm/**.inl"
	}

	includedirs
	{
		"src",
		"vendor/spdlog/include/",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glm}"
	}

	links
	{
		"GLFW",
		"opengl32.lib"
	}

	filter "system:windows"
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