#include "SandboxLayer.h"

// TEMPORARY
#include <glm/gtc/matrix_transform.hpp>

static Magma::Ref<Magma::Shader> s_Shader = nullptr;
static Magma::Ref<Magma::Pipeline> s_Pipeline = nullptr;
static Magma::Ref<Magma::Model> s_Model = nullptr;
static glm::mat4 s_ViewProj;

SandboxLayer::SandboxLayer() : Layer("Sandbox Layer")
{
	s_Shader = Magma::Shader::Create("assets/shaders/VulkanTestModel.glsl");
	Magma::PipelineSpecification spec{};
	spec.InputElementsLayout = {
		{Magma::ShaderDataType::Float3, "a_Position"},
		{Magma::ShaderDataType::Float3, "a_Normal"},
		{Magma::ShaderDataType::Float2, "a_TexCoord"},
		{Magma::ShaderDataType::Float3, "a_Tangent"},
		{Magma::ShaderDataType::Float3, "a_Bitangent"},
	};
	spec.Shader = s_Shader;

	const auto& instance = Magma::Application::Instance().GetWindow().GetGraphicsInstance();
	const auto& device = instance->GetDevice();
	const auto& renderPass = instance->GetSwapchain()->GetMainRenderPass();
	s_Pipeline = Magma::Pipeline::Create(spec, device, renderPass);

	s_Model = Magma::CreateRef<Magma::Model>("assets/models/UVSphere.obj", device);

	s_ViewProj = glm::perspective(glm::radians(60.0f), 16.0f / 9.0f, 0.1f, 25.0f)
		* glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

SandboxLayer::~SandboxLayer()
{
	s_Model = nullptr;
	s_Pipeline = nullptr;
	s_Shader = nullptr;
}

void SandboxLayer::OnUpdate()
{
	// TEMPORARY
	const auto& window = Magma::Application::Instance().GetWindow();
	const auto& renderPass = window.GetGraphicsInstance()->GetSwapchain()->GetMainRenderPass();

	struct TestConstantData
	{
		glm::mat4 viewProj;
		glm::vec4 tintColor;
		glm::vec3 posOffset;
	};

	TestConstantData data{};
	data.viewProj = s_ViewProj;

	Magma::RenderCommand::BeginRenderPass(renderPass);
	Magma::RenderCommand::SetViewport(0, 0, window.GetWidth(), window.GetHeight());
	Magma::RenderCommand::SetScissor(0, 0, window.GetWidth(), window.GetHeight());
	Magma::RenderCommand::BindPipeline(s_Pipeline);
	{
		data.tintColor = glm::vec4(0.5f, 0.3f, 0.2f, 1.0f);
		data.posOffset = glm::vec4(-1.0f);
		Magma::RenderCommand::UploadConstantData(s_Pipeline, sizeof(TestConstantData), &data);
		s_Model->Render();
	}
	{
		data.tintColor = glm::vec4(1.0f);
		data.posOffset = glm::vec4(0.0f);
		Magma::RenderCommand::UploadConstantData(s_Pipeline, sizeof(TestConstantData), &data);
		s_Model->Render();
	}
	{
		data.tintColor = glm::vec4(0.4f, 1.0f, 0.3f, 1.0f);
		data.posOffset = glm::vec4(1.0f);
		Magma::RenderCommand::UploadConstantData(s_Pipeline, sizeof(TestConstantData), &data);
		s_Model->Render();
	}
	
	Magma::RenderCommand::EndRenderPass(renderPass);
}

void SandboxLayer::OnEvent(Magma::Event& event)
{
	MGM_TRACE("{0}", event);
}
