VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["GLFW"] = "%{wks.location}/MagmaEngine/vendor/GLFW/include"
IncludeDir["glm"] = "%{wks.location}/MagmaEngine/vendor/glm"
IncludeDir["Vulkan"] = "%{VULKAN_SDK}/Include"

LibraryDirs = {}
LibraryDirs["Vulkan"] = "%{VULKAN_SDK}/Lib"

Libraries = {}
Libraries["Vulkan"] = "%{LibraryDirs.Vulkan}/vulkan-1.lib"
Libraries["ShaderC"] = "%{LibraryDirs.Vulkan}/shaderc_shared.lib"