#include "SandboxLayer.h"

#include <glm/gtc/matrix_transform.hpp>

SandboxLayer::SandboxLayer() : Layer("Sandbox Layer")
{
	const auto& instance = Magma::Application::Instance().GetWindow().GetGraphicsInstance();
	const auto& device = instance->GetDevice();
	const auto& model = Magma::CreateRef<Magma::Model>("assets/models/UVSphere.obj", device);
	const auto& whiteMaterial = Magma::CreateRef<Magma::Material>();
	const auto& redMaterial = Magma::CreateRef<Magma::Material>();
	redMaterial->SetColor({1.0f, 0.0f, 0.0f, 1.0f});
	redMaterial->SetMetallic(1.0f);
	redMaterial->SetRoughness(0.2f);

	const auto& transform = Magma::CreateRef<Magma::Transform>(glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, 0.5f, -0.75f)));
	const auto& gameObject1 = Magma::CreateRef<Magma::GameObject>(transform);
	const auto& meshRenderer1 = Magma::CreateRef<Magma::MeshRenderer>(model);
	meshRenderer1->SetMaterial(whiteMaterial);
	gameObject1->SetMeshRenderer(meshRenderer1);
	Magma::Renderer::AddGameObject(gameObject1);

	const auto& gameObject2 = Magma::CreateRef<Magma::GameObject>();
	const auto& meshRenderer2 = Magma::CreateRef<Magma::MeshRenderer>(model);
	meshRenderer2->SetMaterial(redMaterial);
	gameObject2->SetMeshRenderer(meshRenderer2);
	Magma::Renderer::AddGameObject(gameObject2);
}

SandboxLayer::~SandboxLayer()
{
}

void SandboxLayer::OnUpdate()
{
	Magma::Camera::Main->Update();
	Magma::Renderer::RenderGameObjects();
	Magma::Renderer::DrawToScreen();
}

void SandboxLayer::OnEvent(Magma::Event& event)
{
	MGM_TRACE("{0}", event);
}
