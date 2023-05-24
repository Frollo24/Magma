#pragma once

// For use by Magma applications

// Core elements
#include "Magma/Core/Application.h"
#include "Magma/Core/Layer.h"
#include "Magma/Core/Log.h"
#include "Magma/Core/Time.h"

// Elements
#include "Magma/Elements/Model.h"

// Renderer
#include "Magma/Renderer/Renderer.h"
#include "Magma/Renderer/RenderCommand.h"
#include "Magma/Renderer/GraphicsInstance.h"
#include "Magma/Renderer/RenderDevice.h"
#include "Magma/Renderer/RenderSwapchain.h"
#include "Magma/Renderer/Descriptors.h"
#include "Magma/Renderer/Framebuffer.h"

#ifdef MAGMA_ENTRY_POINT
// Entry Point
#include "Magma/Core/EntryPoint.h"
#endif // MAGMA_ENTRY_POINT