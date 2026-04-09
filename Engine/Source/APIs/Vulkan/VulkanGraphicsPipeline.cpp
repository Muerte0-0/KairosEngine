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

	VulkanGraphicsPipeline::VulkanGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo)
		: m_CreateInfo(createInfo)
	{
		ASSERT(m_CreateInfo.Shader,
			"VulkanGraphicsPipeline: Shader is null.");
		ASSERT(m_CreateInfo.Shader->HasStage(ShaderStage::Vertex),
			"VulkanGraphicsPipeline: shader '{}' has no Vertex stage.",
			m_CreateInfo.Shader->GetName());
		ASSERT(m_CreateInfo.Shader->HasStage(ShaderStage::Fragment),
			"VulkanGraphicsPipeline: shader '{}' has no Fragment stage.",
			m_CreateInfo.Shader->GetName());

		CreatePipelineCache();
		CreatePipelineLayout();
		CreatePipeline();
		CreateDescriptorPool();
		CreateDescriptorSets();

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
		std::array bindings = {
			vk::DescriptorSetLayoutBinding(
				0, vk::DescriptorType::eUniformBuffer, 1,
				vk::ShaderStageFlagBits::eVertex, nullptr),
		};

		vk::DescriptorSetLayoutCreateInfo layoutInfo;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings    = bindings.data();
		m_DescriptorSetLayout   = vk::raii::DescriptorSetLayout(GetDevice(), layoutInfo);

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo;
		pipelineLayoutInfo.setLayoutCount         = 1;
		pipelineLayoutInfo.pSetLayouts            = &*m_DescriptorSetLayout;
		constexpr vk::PushConstantRange pushConstantRange(
			vk::ShaderStageFlagBits::eVertex,
			0,
			sizeof(PushConstantObject));
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges    = &pushConstantRange;
		m_PipelineLayout = vk::raii::PipelineLayout(GetDevice(), pipelineLayoutInfo);
	}

	void VulkanGraphicsPipeline::CreatePipeline()
	{
		auto* vulkanShader = dynamic_cast<VulkanShader*>(m_CreateInfo.Shader.get());

		// Shader stages
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

		// Vertex input — built from the VertexLayout in createInfo (no mesh reference)
		vk::VertexInputBindingDescription bindDesc =
			VulkanUtils::CreateBindingDescription(m_CreateInfo.VertexLayout);
		std::vector<vk::VertexInputAttributeDescription> attribDescs =
			VulkanUtils::CreateAttributeDescriptions(m_CreateInfo.VertexLayout);

		vk::PipelineVertexInputStateCreateInfo vertexInputState;
		vertexInputState.vertexBindingDescriptionCount   = 1;
		vertexInputState.pVertexBindingDescriptions      = &bindDesc;
		vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribDescs.size());
		vertexInputState.pVertexAttributeDescriptions    = attribDescs.data();

		vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
		inputAssembly.topology               = vk::PrimitiveTopology::eTriangleList;
		inputAssembly.primitiveRestartEnable = vk::False;

		constexpr vk::Viewport viewport(0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f);
		constexpr vk::Rect2D   scissor(vk::Offset2D(0, 0), vk::Extent2D(1, 1));

		vk::PipelineViewportStateCreateInfo viewportState;
		viewportState.viewportCount = 1;
		viewportState.pViewports    = &viewport;
		viewportState.scissorCount  = 1;
		viewportState.pScissors     = &scissor;

		vk::PipelineRasterizationStateCreateInfo rasterState;
		rasterState.polygonMode = vk::PolygonMode::eFill;
		rasterState.cullMode    = vk::CullModeFlagBits::eBack;
		rasterState.frontFace   = vk::FrontFace::eClockwise;
		rasterState.lineWidth   = 1.0f;

		vk::PipelineMultisampleStateCreateInfo msaaState;
		msaaState.rasterizationSamples = VulkanUtils::ToVulkanSampleCount(m_CreateInfo.SampleCount);
		msaaState.sampleShadingEnable  = vk::False;

		vk::PipelineDepthStencilStateCreateInfo depthState;
		depthState.depthTestEnable  = vk::True;
		depthState.depthWriteEnable = vk::True;
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

		vk::Format colorFmt = VulkanUtils::ToVulkanFormat(m_CreateInfo.ColorFormat);
		vk::Format depthFmt = VulkanUtils::ToVulkanFormat(m_CreateInfo.DepthFormat);

		vk::PipelineRenderingCreateInfo renderingInfo;
		renderingInfo.colorAttachmentCount    = 1;
		renderingInfo.pColorAttachmentFormats = &colorFmt;
		renderingInfo.depthAttachmentFormat   = depthFmt;

		vk::GraphicsPipelineCreateInfo pipelineInfo;
		pipelineInfo.stageCount          = static_cast<uint32_t>(shaderStages.size());
		pipelineInfo.pStages             = shaderStages.data();
		pipelineInfo.pVertexInputState   = &vertexInputState;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
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

		m_Pipeline = vk::raii::Pipeline(
			GetDevice(), m_PipelineCache, chain.get<vk::GraphicsPipelineCreateInfo>());
	}

	void VulkanGraphicsPipeline::CreateDescriptorPool()
	{
		std::array poolSizes{
			vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, MAX_FRAMES_IN_FLIGHT),
		};

		vk::DescriptorPoolCreateInfo poolInfo(
			vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
			MAX_FRAMES_IN_FLIGHT, poolSizes);
		m_DescriptorPool = vk::raii::DescriptorPool(GetDevice(), poolInfo);
	}

	void VulkanGraphicsPipeline::CreateDescriptorSets()
	{
		auto* api = dynamic_cast<VulkanRenderAPI*>(Renderer::GetAPI());

		std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *m_DescriptorSetLayout);
		vk::DescriptorSetAllocateInfo allocInfo(
			m_DescriptorPool,
			static_cast<uint32_t>(layouts.size()),
			layouts.data());

		m_DescriptorSets = api->GetVulkanDevice()->GetDevice().allocateDescriptorSets(allocInfo);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vk::DescriptorBufferInfo bufferInfo(
				*api->GetUniformBuffers()[i], 0, sizeof(UniformBufferObject));

			std::array<vk::WriteDescriptorSet, 1> writes{};
			writes[0].setDstSet(*m_DescriptorSets[i]);
			writes[0].setDstBinding(0);
			writes[0].setDstArrayElement(0);
			writes[0].setDescriptorCount(1);
			writes[0].setDescriptorType(vk::DescriptorType::eUniformBuffer);
			writes[0].setPBufferInfo(&bufferInfo);

			GetDevice().updateDescriptorSets(writes, {});
		}
	}
}
