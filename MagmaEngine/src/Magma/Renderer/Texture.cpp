#include "mgmpch.h"
#include "Texture.h"
#include "RenderContext.h"

#include "RenderingAPI/Vulkan/VulkanTexture.h"

namespace Magma
{
	Ref<Texture2D> Texture2D::Create(const Ref<RenderDevice>& device, const std::string& filepath, const TextureSpecs& specs, const bool generateMipmapsOnLoad)
	{
		switch (RenderContext::GetAPI())
		{
			case RenderAPI::None:      MGM_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
			case RenderAPI::Vulkan:	   return CreateRef<VulkanTexture2D>(device, filepath, specs, generateMipmapsOnLoad);
		}

		MGM_CORE_ASSERT(false, "Unknown render API!"); return nullptr;
	}

	Ref<TextureCube> TextureCube::Create(const Ref<RenderDevice>& device, const std::array<std::string, 6>& filepaths, const bool generateMipmapsOnLoad)
	{
		switch (RenderContext::GetAPI())
		{
			case RenderAPI::None:      MGM_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
			case RenderAPI::Vulkan:	   return CreateRef<VulkanTextureCube>(device, filepaths, generateMipmapsOnLoad);
		}

		MGM_CORE_ASSERT(false, "Unknown render API!"); return nullptr;
	}

	Ref<FramebufferTexture2D> FramebufferTexture2D::Create(const Ref<RenderDevice>& device, FramebufferTextureFormat format, const u32 width, const u32 height, const u32 numSamples)
	{
		switch (RenderContext::GetAPI())
		{
			case RenderAPI::None:      MGM_CORE_ASSERT(false, "RenderAPI::None is currently not supported!"); return nullptr;
			case RenderAPI::Vulkan:	   return CreateRef<VulkanFramebufferTexture2D>(device, format, width, height, numSamples);
		}

		MGM_CORE_ASSERT(false, "Unknown render API!"); return nullptr;
	}

}