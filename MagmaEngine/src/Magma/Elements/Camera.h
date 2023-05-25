#pragma once

#include "Magma/Core/Base.h"

#include <glm/glm.hpp>

namespace Magma
{
	class MAGMA_API Camera
	{
	public:
		static Ref<Camera> Main;

	public:
		Camera(const glm::mat4& proj) : m_Projection(proj) {}
		virtual ~Camera() = default;

		inline const glm::mat4& GetProjection() const { return m_Projection; }
		inline void SetProjection(const glm::mat4& proj) { m_Projection = proj; }

		// TODO: make this an interface
		inline const glm::mat4& GetView() const { return m_View; }
		inline void SetView(const glm::mat4& view) { m_View = view; RecalculateViewProjMatrix(); }

		inline const glm::mat4& GetViewProjection() const { return m_ViewProj; }
		inline const glm::vec4& GetPosition() const { return m_Position; }

		virtual void Update();

	protected:
		void RecalculateViewProjMatrix();

	private:
		glm::mat4 m_View = glm::mat4(1.0f);
		glm::mat4 m_Projection;
		glm::mat4 m_ViewProj = m_Projection * m_View;
		glm::vec4 m_Position = glm::vec4(0.0f);
	};
}

