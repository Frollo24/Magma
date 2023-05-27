#include "mgmpch.h"
#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Magma
{
	Ref<Camera> Camera::Main = nullptr;

	void Camera::RecalculateViewProjMatrix()
	{
		m_ViewProj = m_Projection * m_View;
	}

	void Camera::Update()
	{
		if (Input::IsKeyPressed(KeyCode::W))
			m_View = glm::translate(glm::mat4(1.0f), (float)Time::DeltaTime * glm::vec3(0.0f, 0.0f, 4.0f)) * m_View;
		if (Input::IsKeyPressed(KeyCode::S))
			m_View = glm::translate(glm::mat4(1.0f), (float)Time::DeltaTime * glm::vec3(0.0f, 0.0f, -4.0f)) * m_View;
		if (Input::IsKeyPressed(KeyCode::A))
			m_View = glm::translate(glm::mat4(1.0f), (float)Time::DeltaTime * glm::vec3(4.0f, 0.0f, 0.0f)) * m_View;
		if (Input::IsKeyPressed(KeyCode::D))
			m_View = glm::translate(glm::mat4(1.0f), (float)Time::DeltaTime * glm::vec3(-4.0f, 0.0f, 0.0f)) * m_View;
		if (Input::IsKeyPressed(KeyCode::Q))
			m_View = glm::translate(glm::mat4(1.0f), (float)Time::DeltaTime * glm::vec3(0.0f, 4.0f, 0.0f)) * m_View;
		if (Input::IsKeyPressed(KeyCode::E))
			m_View = glm::translate(glm::mat4(1.0f), (float)Time::DeltaTime * glm::vec3(0.0f, -4.0f, 0.0f)) * m_View;

		if (Input::IsKeyPressed(KeyCode::Left))
			m_View = glm::rotate(glm::mat4(1.0f), glm::radians(-30.0f) * (float)Time::DeltaTime, glm::vec3(m_View[1])) * m_View;
		if (Input::IsKeyPressed(KeyCode::Right))
			m_View = glm::rotate(glm::mat4(1.0f), glm::radians(30.0f) * (float)Time::DeltaTime, glm::vec3(m_View[1])) * m_View;

		if (Input::IsKeyPressed(KeyCode::Up))
			m_View = glm::rotate(glm::mat4(1.0f), glm::radians(-30.0f) * (float)Time::DeltaTime, glm::vec3(1.0f, 0.0f, 0.0f)) * m_View;
		if (Input::IsKeyPressed(KeyCode::Down))
			m_View = glm::rotate(glm::mat4(1.0f), glm::radians(30.0f) * (float)Time::DeltaTime, glm::vec3(1.0f, 0.0f, 0.0f)) * m_View;

		RecalculateViewProjMatrix();
	}
}