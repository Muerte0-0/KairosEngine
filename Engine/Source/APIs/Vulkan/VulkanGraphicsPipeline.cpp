#include "kepch.h"
#include "VulkanGraphicsPipeline.h"

#include "VulkanRenderAPI.h"
#include "VulkanUtils.h"
#include "Engine/Renderer/Renderer.h"

namespace Engine
{
	// -----------------------------------------------------------------------
	// Helpers
	// -----------------------------------------------------------------------

	namespace
	{
		/** Retrieve the logical Vulkan device from the currently active renderer. */
		vk::raii::Device& GetDevice()
		{
			auto* api = dynamic_cast<VulkanRenderAPI*>(Renderer::GetAPI());
			KE_CORE_ASSERT(api, "VulkanGraphicsPipeline: active RenderAPI is not VulkanRenderAPI");
			return api->GetVulkanDevice()->GetDevice();
		}
	}

	// -----------------------------------------------------------------------
	// Construction
	// -----------------------------------------------------------------------

	VulkanGraphicsPipeline::VulkanGraphicsPipeline(GraphicsPipelineCreateInfo createInfo)
		: GraphicsPipeline(std::move(createInfo))
	{
		CreateShaders();
		CreatePipelineCache();
		CreatePipelineLayout();
		CreatePipeline();
	}

	// -----------------------------------------------------------------------
	// Private helpers
	// -----------------------------------------------------------------------

	ShaderBinary VulkanGraphicsPipeline::LoadShaderBinary(const std::filesystem::path& filepath) const
	{
		std::ifstream stream(filepath, std::ios::binary | std::ios::ate);
		KE_CORE_ASSERT(stream.is_open(),
			"VulkanGraphicsPipeline: failed to open compiled shader '{}'", filepath.string());

		const std::streamsize fileSize = stream.tellg();
		KE_CORE_ASSERT(fileSize > 0,
			"VulkanGraphicsPipeline: compiled shader is empty '{}'", filepath.string());
		KE_CORE_ASSERT((fileSize % static_cast<std::streamsize>(sizeof(uint32_t))) == 0,
			"VulkanGraphicsPipeline: '{}' is not valid SPIR-V", filepath.string());

		stream.seekg(0, std::ios::beg);

		ShaderBinary binary;
		binary.Bytecode.resize(static_cast<size_t>(fileSize / sizeof(uint32_t)));
		stream.read(reinterpret_cast<char*>(binary.Bytecode.data()), fileSize);
		KE_CORE_ASSERT(stream.good() || stream.eof(),
			"VulkanGraphicsPipeline: failed to read compiled shader '{}'", filepath.string());

		return binary;
	}

	void VulkanGraphicsPipeline::CreateShaders()
	{
		const std::filesystem::path vertexPath   = m_CreateInfo.ShaderDirectory / m_CreateInfo.VertexShader.Filepath;
		const std::filesystem::path fragmentPath = m_CreateInfo.ShaderDirectory / m_CreateInfo.FragmentShader.Filepath;

		// Use the Shader factory — it routes through VulkanShader internally.
		m_VertexShader   = CreateScope<VulkanShader>(m_CreateInfo.VertexShader,   LoadShaderBinary(vertexPath));
		m_FragmentShader = CreateScope<VulkanShader>(m_CreateInfo.FragmentShader, LoadShaderBinary(fragmentPath));

		LOG(LogLevel::Info, "Loaded shaders: '{}', '{}'",
			m_CreateInfo.VertexShader.Filepath.string(),
			m_CreateInfo.FragmentShader.Filepath.string());
	}

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
		const std::array shaderStages = {
			vk::PipelineShaderStageCreateInfo({},
				m_VertexShader->GetVulkanStage(),
				*m_VertexShader->GetShaderModule(),
				m_VertexShader->GetEntryPoint().c_str()),
			vk::PipelineShaderStageCreateInfo({},
				m_FragmentShader->GetVulkanStage(),
				*m_FragmentShader->GetShaderModule(),
				m_FragmentShader->GetEntryPoint().c_str()),
		};

		vk::PipelineVertexInputStateCreateInfo vertexInputState;
		vertexInputState.vertexBindingDescriptionCount   = 0;
		vertexInputState.pVertexBindingDescriptions      = nullptr;
		vertexInputState.vertexAttributeDescriptionCount = 0;
		vertexInputState.pVertexAttributeDescriptions    = nullptr;

		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;
		inputAssemblyState.topology               = vk::PrimitiveTopology::eTriangleList;
		inputAssemblyState.primitiveRestartEnable = vk::False;

		constexpr vk::Viewport viewport(0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f);
		constexpr vk::Rect2D   scissor(vk::Offset2D(0, 0), vk::Extent2D(1, 1));

		vk::PipelineViewportStateCreateInfo viewportState;
		viewportState.viewportCount = 1;
		viewportState.pViewports    = &viewport;
		viewportState.scissorCount  = 1;
		viewportState.pScissors     = &scissor;

		vk::PipelineRasterizationStateCreateInfo rasterizationState;
		rasterizationState.depthClampEnable        = vk::False;
		rasterizationState.rasterizerDiscardEnable = vk::False;
		rasterizationState.polygonMode             = vk::PolygonMode::eFill;
		rasterizationState.cullMode                = vk::CullModeFlagBits::eBack;
		rasterizationState.frontFace               = vk::FrontFace::eCounterClockwise;
		rasterizationState.lineWidth               = 1.0f;

		vk::PipelineMultisampleStateCreateInfo multisampleState;
		multisampleState.rasterizationSamples = VulkanUtils::ToVulkanSampleCount(m_CreateInfo.SampleCount);
		multisampleState.sampleShadingEnable  = vk::False;

		vk::PipelineDepthStencilStateCreateInfo depthStencilState;
		depthStencilState.depthTestEnable       = vk::False;
		depthStencilState.depthWriteEnable      = vk::False;
		depthStencilState.depthCompareOp        = vk::CompareOp::eLess;
		depthStencilState.depthBoundsTestEnable = vk::False;
		depthStencilState.stencilTestEnable     = vk::False;

		constexpr vk::PipelineColorBlendAttachmentState colorBlendAttachment(
			vk::False,
			vk::BlendFactor::eOne,   vk::BlendFactor::eZero, vk::BlendOp::eAdd,
			vk::BlendFactor::eOne,   vk::BlendFactor::eZero, vk::BlendOp::eAdd,
			vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
			vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

		vk::PipelineColorBlendStateCreateInfo colorBlendState;
		colorBlendState.logicOpEnable   = vk::False;
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments    = &colorBlendAttachment;

		constexpr std::array dynamicStates = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor,
		};

		vk::PipelineDynamicStateCreateInfo dynamicState;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates    = dynamicStates.data();

		vk::Format colorFormat = VulkanUtils::ToVulkanFormat(m_CreateInfo.ColorFormat);
		vk::Format depthFormat = VulkanUtils::ToVulkanFormat(TextureFormat::Undefined);

		vk::PipelineRenderingCreateInfo renderingInfo;
		renderingInfo.colorAttachmentCount    = 1;
		renderingInfo.pColorAttachmentFormats = &colorFormat;
		renderingInfo.depthAttachmentFormat   = depthFormat;

		vk::GraphicsPipelineCreateInfo pipelineInfo;
		pipelineInfo.stageCount          = static_cast<uint32_t>(shaderStages.size());
		pipelineInfo.pStages             = shaderStages.data();
		pipelineInfo.pVertexInputState   = &vertexInputState;
		pipelineInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineInfo.pViewportState      = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizationState;
		pipelineInfo.pMultisampleState   = &multisampleState;
		pipelineInfo.pDepthStencilState  = &depthStencilState;
		pipelineInfo.pColorBlendState    = &colorBlendState;
		pipelineInfo.pDynamicState       = &dynamicState;
		pipelineInfo.layout              = *m_PipelineLayout;
		pipelineInfo.renderPass          = nullptr;

		vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> chain{
			pipelineInfo, renderingInfo
		};

		m_Pipeline = vk::raii::Pipeline(
			GetDevice(), m_PipelineCache,
			chain.get<vk::GraphicsPipelineCreateInfo>());
	}
}
