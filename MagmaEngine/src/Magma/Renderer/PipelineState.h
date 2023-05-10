#pragma once

#include "Magma/Core/Base.h"

namespace Magma
{
#pragma region PipelineDepthState
	enum class DepthComparison
	{
		None,
		Less,
		Equal,
		LessOrEqual,
		Greater,
		NotEqual,
		GreaterOrEqual,
		Always
	};

	struct DepthBoundaries
	{
		float MinDepthBoundary = 0.0f;
		float MaxDepthBoundary = 1.0f;
	};

	struct PipelineDepthState
	{
		bool DepthTest = true;
		bool DepthWrite = true;
		DepthComparison DepthFunc = DepthComparison::Less;
		DepthBoundaries DepthRange{};
	};
#pragma endregion

#pragma region PipelineStencilState
	// To be done in the future...
#pragma endregion

#pragma region PipelineBlendState
	enum ColorWriteMask
	{
		ColorWriteMaskR = MGM_BIT(0), // 1
		ColorWriteMaskG = MGM_BIT(1), // 2
		ColorWriteMaskB = MGM_BIT(2), // 4
		ColorWriteMaskA = MGM_BIT(3), // 8
		ColorWriteMaskAll = ColorWriteMaskR | ColorWriteMaskG | ColorWriteMaskB | ColorWriteMaskA, // 15
	};

	enum class BlendFactor
	{
		Zero, One,
		SrcColor, OneMinusSrcColor, DstColor, OneMinusDstColor,
		SrcAlpha, OneMinusSrcAlpha, DstAlpha, OneMinusDstAlpha,
		ConstantColor, OneMinusConstantColor,
		ConstantAlpha, OneMinusConstantAlpha,
	};

	enum class BlendOperation
	{
		Add, Subtract, ReverseSubtract, Minimum, Maximum,
		SrcMinusDst = Subtract,
		DstMinusSrc = ReverseSubtract,
	};

	struct BlendingEquation
	{
		BlendFactor SrcFactor = BlendFactor::One;
		BlendFactor DstFactor = BlendFactor::Zero;
		BlendOperation Operation = BlendOperation::Add;
	};

	struct BlendConstants
	{
		float R = 0.0f;
		float G = 0.0f;
		float B = 0.0f;
		float A = 0.0f;
	};

	struct PipelineBlendState
	{
		bool BlendEnable = false;
		ColorWriteMask ColorWriteMask = ColorWriteMaskAll;
		BlendingEquation ColorEquation{};
		BlendingEquation AlphaEquation{};
		BlendConstants ConstantColor{};
	};

	MGM_DEFINE_ENUM_FLAG_OPERATORS(ColorWriteMask);
#pragma endregion

#pragma region PipelinePolygonState
	enum class PolygonRasterMode
	{
		Fill,
		Line,
		Point
	};

	enum class CullingMode
	{
		None,
		Front,
		Back,
		FrontAndBack
	};

	enum class FrontFaceMode
	{
		CounterClockwise,
		Clockwise
	};

	struct PipelinePolygonState
	{
		PolygonRasterMode PolygonMode = PolygonRasterMode::Fill;
		CullingMode CullMode = CullingMode::Back;
		FrontFaceMode FrontFace = FrontFaceMode::Clockwise;
	};
#pragma endregion
}