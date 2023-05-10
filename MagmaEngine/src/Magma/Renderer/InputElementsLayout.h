#pragma once

#include "Magma/Core/Base.h"

namespace Magma
{
	enum class ShaderDataType {
		None = 0,
		Bool, Int, Int2, Int3, Int4,
		Float, Float2, Float3, Float4,
		Mat3, Mat4
	};

	static u32 ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::Bool:     return 1;
			case ShaderDataType::Int:      return 4;
			case ShaderDataType::Int2:     return 4 * 2;
			case ShaderDataType::Int3:     return 4 * 3;
			case ShaderDataType::Int4:     return 4 * 4;
			case ShaderDataType::Float:    return 4;
			case ShaderDataType::Float2:   return 4 * 2;
			case ShaderDataType::Float3:   return 4 * 3;
			case ShaderDataType::Float4:   return 4 * 4;
			case ShaderDataType::Mat3:     return 4 * 3 * 3;
			case ShaderDataType::Mat4:     return 4 * 4 * 4;
			default:
				MGM_CORE_ASSERT(false, "Unknown ShaderDataType!");
				return 0;
		}
	}

	struct InputAttribute {
		std::string Name;
		ShaderDataType Type = ShaderDataType::None;
		u32 Size = 0;
		u32 Offset = 0;
		bool Normalized = false;

		InputAttribute() = default;
		InputAttribute(ShaderDataType type, const std::string& name, bool normalized = false)
			: Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized) {}

		u32 GetComponentCount() const
		{
			switch (Type)
			{
				case ShaderDataType::Float:   return 1;
				case ShaderDataType::Float2:  return 2;
				case ShaderDataType::Float3:  return 3;
				case ShaderDataType::Float4:  return 4;
				case ShaderDataType::Mat3:    return 3; // 3* float3
				case ShaderDataType::Mat4:    return 4; // 4* float4
				case ShaderDataType::Int:     return 1;
				case ShaderDataType::Int2:    return 2;
				case ShaderDataType::Int3:    return 3;
				case ShaderDataType::Int4:    return 4;
				case ShaderDataType::Bool:    return 1;
				default:
					return 0;
			}
		}
	};

	class InputElementsLayout {
	public:
		InputElementsLayout() = default;
		InputElementsLayout(const std::initializer_list<InputAttribute>& elements) : m_Elements(elements) { CalculateStrideAndOffset(); }
		InputElementsLayout(const std::vector<InputAttribute>& elements) : m_Elements(elements) { CalculateStrideAndOffset(); }

		inline u32 GetStride() const { return m_Stride; }
		inline const std::vector<InputAttribute>& GetElements() const { return m_Elements; }

		std::vector<InputAttribute>::iterator begin() { return m_Elements.begin(); }
		std::vector<InputAttribute>::iterator end() { return m_Elements.end(); }

	private:
		void CalculateStrideAndOffset()
		{
			u32 offset = 0;
			m_Stride = 0;
			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}

	private:
		std::vector<InputAttribute> m_Elements;
		u32 m_Stride = 0;
	};
}