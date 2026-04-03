#include "kepch.h"
#include "VulkanGraphicsPipeline.h"

#include "VulkanRenderAPI.h"
#include "VulkanUtils.h"
#include "Engine/Renderer/Renderer.h"

namespace Engine
{
	namespace
	{
		vk::raii::Device& GetDevice()
		{
			auto* api = dynamic_cast<VulkanRenderAPI*>(Renderer::GetAPI());
			ASSERT(api, "VulkanGraphicsPipeline: active RenderAPI is not VulkanRenderAPI.");
			return api->GetVulkanDevice()->GetDevice();
		}
	}

	// -----------------------------------------------------------------------
	// Construction / destruction
	// -----------------------------------------------------------------------

	VulkanGraphicsPipeline::VulkanGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) : m_CreateInfo(createInfo)
	{
		ASSERT(m_CreateInfo.Shader,
			"VulkanGraphicsPipeline::Init — Shader is null.");
		ASSERT(m_CreateInfo.Shader->HasStage(ShaderStage::Vertex),
			"VulkanGraphicsPipeline::Init — shader '{}' has no Vertex stage.",
			m_CreateInfo.Shader->GetName());
		ASSERT(m_CreateInfo.Shader->HasStage(ShaderStage::Fragment),
			"VulkanGraphicsPipeline::Init — shader '{}' has no Fragment stage.",
			m_CreateInfo.Shader->GetName());

		CreatePipelineCache();
		CreatePipelineLayout();
		CreatePipeline();

		LOG(LogLevel::Info, "VulkanGraphicsPipeline: built pipeline for shader '{}'.",
			m_CreateInfo.Shader->GetName());
	}

	VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
	{
		m_Pipeline            = nullptr;
		m_PipelineLayout      = nullptr;
		m_DescriptorSetLayout = nullptr;
		m_PipelineCache       = nullptr;
	}

	// -----------------------------------------------------------------------
	// Private helpers
	// -----------------------------------------------------------------------

	void VulkanGraphicsPipeline::CreatePipelineCache()
	{
		vk::PipelineCacheCreateInfo info;
		m_PipelineCache = vk::raii::PipelineCache(GetDevice(), info);
	}

	void VulkanGraphicsPipeline::CreatePipelineLayout()
	{
		vk::DescriptorSetLayoutCreateInfo setLayoutInfo;
		m_DescriptorSetLayout = vk::raii::DescriptorSetLayout(GetDevice(), setLayoutInfo);

		vk::PipelineLayoutCreateInfo layoutInfo;
		m_PipelineLayout = vk::raii::PipelineLayout(GetDevice(), layoutInfo);
	}

	void VulkanGraphicsPipeline::CreatePipeline()
	{
		auto* vulkanShader = dynamic_cast<VulkanShader*>(m_CreateInfo.Shader.get());

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		for (const ShaderStageData* stageData : m_CreateInfo.Shader->GetStages())
		{
			shaderStages.emplace_back(
				vk::PipelineShaderStageCreateFlags{},
				VulkanShader::ToVkStage(stageData->Stage),
				*vulkanShader->GetModule(stageData->Stage),
				stageData->EntryPoint.c_str());
		}

		ASSERT(!shaderStages.empty(),
			"VulkanGraphicsPipeline: shader '{}' produced zero stage infos.",
			m_CreateInfo.Shader->GetName());

		// ------------------------------------------------------------------
		// Fixed pipeline state
		// ------------------------------------------------------------------
		vk::PipelineVertexInputStateCreateInfo vertexInputState;
		
		Mesh* mesh = m_CreateInfo.Meshes[0].get();
		vk::VertexInputBindingDescription bindDesc = VulkanUtils::CreateBindingDescription(mesh->GetLayout());
		std::array<vk::VertexInputBindingDescription, 1> bindings = { bindDesc };
		std::vector<vk::VertexInputAttributeDescription> vertAttribDesc = VulkanUtils::CreateAttributeDescriptions(mesh->GetLayout());
		
		vertexInputState.vertexBindingDescriptionCount		= static_cast<uint32_t>(bindings.size());
		vertexInputState.pVertexBindingDescriptions			= bindings.data();
		vertexInputState.vertexAttributeDescriptionCount	= static_cast<uint32_t>(vertAttribDesc.size());
		vertexInputState.pVertexAttributeDescriptions		= vertAttribDesc.data();

		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;
		inputAssemblyState.topology					= vk::PrimitiveTopology::eTriangleList;
		inputAssemblyState.primitiveRestartEnable	= vk::False;

		constexpr vk::Viewport	viewport(0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f);
		constexpr vk::Rect2D	scissor(vk::Offset2D(0, 0), vk::Extent2D(1, 1));

		vk::PipelineViewportStateCreateInfo viewportState;
		viewportState.viewportCount = 1;
		viewportState.pViewports    = &viewport;
		viewportState.scissorCount  = 1;
		viewportState.pScissors     = &scissor;

		vk::PipelineRasterizationStateCreateInfo rasterState;
		rasterState.polygonMode = vk::PolygonMode::eFill;
		rasterState.cullMode    = vk::CullModeFlagBits::eBack;
		rasterState.frontFace   = vk::FrontFace::eCounterClockwise;
		rasterState.lineWidth   = 1.0f;

		vk::PipelineMultisampleStateCreateInfo msaaState;
		msaaState.rasterizationSamples = VulkanUtils::ToVulkanSampleCount(m_CreateInfo.SampleCount);
		msaaState.sampleShadingEnable  = vk::False;

		vk::PipelineDepthStencilStateCreateInfo depthState;
		depthState.depthTestEnable  = vk::False;
		depthState.depthWriteEnable = vk::False;
		depthState.depthCompareOp   = vk::CompareOp::eLess;

		constexpr vk::PipelineColorBlendAttachmentState blendAttachment(
			vk::False,
			vk::BlendFactor::eOne,  vk::BlendFactor::eZero, vk::BlendOp::eAdd,
			vk::BlendFactor::eOne,  vk::BlendFactor::eZero, vk::BlendOp::eAdd,
			vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
			vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

		vk::PipelineColorBlendStateCreateInfo blendState;
		blendState.logicOpEnable   = vk::False;
		blendState.attachmentCount = 1;
		blendState.pAttachments    = &blendAttachment;

		constexpr std::array dynamicStates = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor,
		};
		vk::PipelineDynamicStateCreateInfo dynamicState;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates    = dynamicStates.data();

		// ------------------------------------------------------------------
		// Dynamic rendering (no render pass)
		// ------------------------------------------------------------------
		vk::Format colorFmt = VulkanUtils::ToVulkanFormat(m_CreateInfo.ColorFormat);
		vk::Format depthFmt = VulkanUtils::ToVulkanFormat(m_CreateInfo.DepthFormat);

		vk::PipelineRenderingCreateInfo renderingInfo;
		renderingInfo.colorAttachmentCount    = 1;
		renderingInfo.pColorAttachmentFormats = &colorFmt;
		renderingInfo.depthAttachmentFormat   = depthFmt;

		// ------------------------------------------------------------------
		// Assemble and create
		// ------------------------------------------------------------------
		vk::GraphicsPipelineCreateInfo pipelineInfo;
		pipelineInfo.stageCount          = static_cast<uint32_t>(shaderStages.size());
		pipelineInfo.pStages             = shaderStages.data();
		pipelineInfo.pVertexInputState   = &vertexInputState;
		pipelineInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineInfo.pViewportState      = &viewportState;
		pipelineInfo.pRasterizationState = &rasterState;
		pipelineInfo.pMultisampleState   = &msaaState;
		pipelineInfo.pDepthStencilState  = &depthState;
		pipelineInfo.pColorBlendState    = &blendState;
		pipelineInfo.pDynamicState       = &dynamicState;
		pipelineInfo.layout              = *m_PipelineLayout;
		pipelineInfo.renderPass          = nullptr;

		vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> chain{
			pipelineInfo, renderingInfo
		};

		m_Pipeline = vk::raii::Pipeline(GetDevice(), m_PipelineCache, chain.get<vk::GraphicsPipelineCreateInfo>());
	}
}
