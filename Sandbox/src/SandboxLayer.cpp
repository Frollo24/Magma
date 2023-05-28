#include "SandboxLayer.h"

#include <glm/gtc/matrix_transform.hpp>

SandboxLayer::SandboxLayer() : Layer("Sandbox Layer")
{
	const auto& instance = Magma::Application::Instance().GetWindow().GetGraphicsInstance();
	const auto& device = instance->GetDevice();
	const auto& model = Magma::CreateRef<Magma::Model>("assets/models/UVSphere.obj", device);
	const auto& bunny = Magma::CreateRef<Magma::Model>("assets/models/reshiram.obj", device);
	const auto& whiteMaterial = Magma::CreateRef<Magma::Material>();
	whiteMaterial->SetRoughness(0.0f);
	const auto& redMaterial = Magma::CreateRef<Magma::Material>();
	redMaterial->SetColor({1.0f, 0.0f, 0.0f, 1.0f});
	redMaterial->SetMetallic(1.0f);
	redMaterial->SetRoughness(0.0f);
	const auto& simpleMaterial = Magma::CreateRef<Magma::Material>();
	simpleMaterial->SetColor({ 0.0f, 1.0f, 0.0f, 1.0f });
	simpleMaterial->SetIsPBR(false);
	const auto& bunnyMaterial = Magma::CreateRef<Magma::Material>();
	bunnyMaterial->SetIsPBR(false);
	const auto& ironMaterial = Magma::CreateRef<Magma::Material>();
	ironMaterial->SetTexture(Magma::Texture2D::Create(device, "assets/textures/iron_rust/rustediron2_basecolor.png"), Magma::TextureType::Albedo);
	ironMaterial->SetTexture(Magma::Texture2D::Create(device, "assets/textures/iron_rust/rustediron2_normal.png"), Magma::TextureType::Normal);
	ironMaterial->SetTexture(Magma::Texture2D::Create(device, "assets/textures/iron_rust/rustediron2_metallic.png"), Magma::TextureType::Metallic);
	ironMaterial->SetTexture(Magma::Texture2D::Create(device, "assets/textures/iron_rust/rustediron2_roughness.png"), Magma::TextureType::Roughness);

	const auto& transform1 = Magma::CreateRef<Magma::Transform>(glm::translate(glm::mat4(1.0f), glm::vec3(1.5f, 0.5f, -0.75f)));
	const auto& gameObject1 = Magma::CreateRef<Magma::GameObject>(transform1);
	const auto& meshRenderer1 = Magma::CreateRef<Magma::MeshRenderer>(model);
	meshRenderer1->SetMaterial(whiteMaterial);
	gameObject1->SetMeshRenderer(meshRenderer1);
	Magma::Renderer::AddGameObject(gameObject1);

	const auto& gameObject2 = Magma::CreateRef<Magma::GameObject>();
	const auto& meshRenderer2 = Magma::CreateRef<Magma::MeshRenderer>(model);
	meshRenderer2->SetMaterial(redMaterial);
	gameObject2->SetMeshRenderer(meshRenderer2);
	Magma::Renderer::AddGameObject(gameObject2);

	const auto& transform3 = Magma::CreateRef<Magma::Transform>(glm::translate(glm::mat4(1.0f), glm::vec3(-2.5f, 0.5f, -1.5f)));
	const auto& gameObject3 = Magma::CreateRef<Magma::GameObject>(transform3);
	const auto& meshRenderer3 = Magma::CreateRef<Magma::MeshRenderer>(model);
	meshRenderer3->SetMaterial(simpleMaterial);
	gameObject3->SetMeshRenderer(meshRenderer3);
	Magma::Renderer::AddGameObject(gameObject3);

	const auto& transform4 = Magma::CreateRef<Magma::Transform>(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.5f, -2.5f)));
	const auto& gameObject4 = Magma::CreateRef<Magma::GameObject>(transform4);
	const auto& meshRenderer4 = Magma::CreateRef<Magma::MeshRenderer>(model);
	meshRenderer4->SetMaterial(ironMaterial);
	gameObject4->SetMeshRenderer(meshRenderer4);
	Magma::Renderer::AddGameObject(gameObject4);

	const auto& transform5 = Magma::CreateRef<Magma::Transform>(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.5f, -2.5f)), glm::vec3(0.05f)));
	const auto& gameObject5 = Magma::CreateRef<Magma::GameObject>(transform5);
	const auto& meshRenderer5 = Magma::CreateRef<Magma::MeshRenderer>(bunny);
	meshRenderer5->SetMaterial(bunnyMaterial);
	gameObject5->SetMeshRenderer(meshRenderer5);
	Magma::Renderer::AddGameObject(gameObject5);
}

SandboxLayer::~SandboxLayer()
{
}

void SandboxLayer::OnUpdate()
{
	Magma::Camera::Main->Update();
	Magma::Renderer::Update();
	Magma::Renderer::RenderGameObjects();
	Magma::Renderer::DrawToScreen();
}

void SandboxLayer::OnEvent(Magma::Event& event)
{
	MGM_TRACE("{0}", event);
}
