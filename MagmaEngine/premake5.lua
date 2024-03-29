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
		"vendor/glm/glm/**.inl",
		"vendor/stb_image/stb_image.h",
		"vendor/stb_image/stb_image.cpp",
		"vendor/tiny_obj_loader/tiny_obj_loader.h",
		"vendor/tiny_obj_loader/tiny_obj_loader.cpp"
	}

	includedirs
	{
		"src",
		"vendor/spdlog/include/",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.Vulkan}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.tiny_obj_loader}"
	}

	libdirs
	{
		"%{LibraryDirs.Vulkan}"
	}

	links
	{
		"GLFW",
		"%{Libraries.Vulkan}",
		"%{Libraries.ShaderC}"
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