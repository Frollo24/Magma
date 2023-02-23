#pragma once

#include "Magma/Core/Base.h"
#include "Magma/Events/Event.h"

namespace Magma
{
	class MAGMA_API Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnEvent(Event& e) {}

		inline const std::string& GetName() { return m_DebugName; }
	private:
		std::string m_DebugName;
	};
}

