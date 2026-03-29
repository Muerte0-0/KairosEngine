#include "kepch.h"
#include "VulkanGraphicsPipeline.h"

#include "VulkanDevice.h"
#include "VulkanUtils.h"

namespace Engine
{
	VulkanGraphicsPipeline::VulkanGraphicsPipeline(VulkanDevice& device, GraphicsPipelineCreateInfo createInfo)
		: GraphicsPipeline(std::move(createInfo)), m_VulkanDevice(device)
	{
		CreateShaders();
		CreatePipelineCache();
		CreatePipelineLayout();
		CreatePipeline();
	}

	ShaderBinary VulkanGraphicsPipeline::LoadShaderBinary(const std::filesystem::path& filepath) const
	{
		std::ifstream stream(filepath, std::ios::binary | std::ios::ate);
		KE_CORE_ASSERT(stream.is_open(), "Failed to open compiled shader {}", filepath.string());

		const std::streamsize fileSize = stream.tellg();
		KE_CORE_ASSERT(fileSize > 0, "Compiled shader is empty {}", filepath.string());
		KE_CORE_ASSERT((fileSize % static_cast<std::streamsize>(sizeof(uint32_t))) == 0, "Compiled shader {} is not valid SPIR-V", filepath.string());

		stream.seekg(0, std::ios::beg);

		ShaderBinary shaderBinary;
		shaderBinary.Bytecode.resize(static_cast<size_t>(fileSize / sizeof(uint32_t)));
		stream.read(reinterpret_cast<char*>(shaderBinary.Bytecode.data()), fileSize);
		KE_CORE_ASSERT(stream.good() || stream.eof(), "Failed to read compiled shader {}", filepath.string());

		return shaderBinary;
	}

	void VulkanGraphicsPipeline::CreateShaders()
	{
		const std::filesystem::path vertexPath = m_CreateInfo.ShaderDirectory / m_CreateInfo.VertexShader.Filepath;
		const std::filesystem::path fragmentPath = m_CreateInfo.ShaderDirectory / m_CreateInfo.FragmentShader.Filepath;

		m_VertexShader = CreateScope<VulkanShader>(m_VulkanDevice, m_CreateInfo.VertexShader, LoadShaderBinary(vertexPath));
		m_FragmentShader = CreateScope<VulkanShader>(m_VulkanDevice, m_CreateInfo.FragmentShader, LoadShaderBinary(fragmentPath));

		LOG(LogLevel::Info, "Loaded Shaders: {}, {}", vertexPath.string(), fragmentPath.string());
	}

	void VulkanGraphicsPipeline::CreatePipelineCache()
	{
		vk::PipelineCacheCreateInfo createInfo;
		m_PipelineCache = vk::raii::PipelineCache(m_VulkanDevice.GetDevice(), createInfo);
	}

	void VulkanGraphicsPipeline::CreateDescriptorSetLayoutStub()
	{
		vk::DescriptorSetLayoutCreateInfo createInfo;
		m_DescriptorSetLayout = vk::raii::DescriptorSetLayout(m_VulkanDevice.GetDevice(), createInfo);
	}

	void VulkanGraphicsPipeline::CreatePipelineLayout()
	{
		vk::PipelineLayoutCreateInfo createInfo;

		m_PipelineLayout = vk::raii::PipelineLayout(m_VulkanDevice.GetDevice(), createInfo);
	}

	void VulkanGraphicsPipeline::CreatePipeline()
	{
		const std::array shaderStages = {
			vk::PipelineShaderStageCreateInfo({}, m_VertexShader->GetVulkanStage(), *m_VertexShader->GetShaderModule(), m_VertexShader->GetEntryPoint().c_str()),
			vk::PipelineShaderStageCreateInfo({}, m_FragmentShader->GetVulkanStage(), *m_FragmentShader->GetShaderModule(), m_FragmentShader->GetEntryPoint().c_str())
		};

		vk::PipelineVertexInputStateCreateInfo vertexInputState;
		vertexInputState.vertexBindingDescriptionCount = 0;
		vertexInputState.pVertexBindingDescriptions = nullptr;
		vertexInputState.vertexAttributeDescriptionCount = 0;
		vertexInputState.pVertexAttributeDescriptions = nullptr;

		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;
		inputAssemblyState.topology = vk::PrimitiveTopology::eTriangleList;
		inputAssemblyState.primitiveRestartEnable = vk::False;

		constexpr vk::Viewport viewport(0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f);
		constexpr vk::Rect2D scissor(vk::Offset2D(0, 0), vk::Extent2D(1, 1));

		vk::PipelineViewportStateCreateInfo viewportState;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		vk::PipelineRasterizationStateCreateInfo rasterizationState;
		rasterizationState.depthClampEnable = vk::False;
		rasterizationState.rasterizerDiscardEnable = vk::False;
		rasterizationState.polygonMode = vk::PolygonMode::eFill;
		rasterizationState.cullMode = vk::CullModeFlagBits::eBack;
		rasterizationState.frontFace = vk::FrontFace::eCounterClockwise;
		rasterizationState.lineWidth = 1.0f;

		vk::PipelineMultisampleStateCreateInfo multisampleState;
		multisampleState.rasterizationSamples = VulkanUtils::ToVulkanSampleCount(m_CreateInfo.SampleCount);
		multisampleState.sampleShadingEnable = vk::False;

		vk::PipelineDepthStencilStateCreateInfo depthStencilState;
		depthStencilState.depthTestEnable = false;
		depthStencilState.depthWriteEnable = false;
		depthStencilState.depthCompareOp = vk::CompareOp::eLess;
		depthStencilState.depthBoundsTestEnable = vk::False;
		depthStencilState.stencilTestEnable = vk::False;

		constexpr vk::PipelineColorBlendAttachmentState colorBlendAttachment(
			vk::False,
			vk::BlendFactor::eOne,
			vk::BlendFactor::eZero,
			vk::BlendOp::eAdd,
			vk::BlendFactor::eOne,
			vk::BlendFactor::eZero,
			vk::BlendOp::eAdd,
			vk::ColorComponentFlagBits::eR |
				vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB |
				vk::ColorComponentFlagBits::eA);

		vk::PipelineColorBlendStateCreateInfo colorBlendState;
		colorBlendState.logicOpEnable = vk::False;
		colorBlendState.attachmentCount = 1;
		colorBlendState.pAttachments = &colorBlendAttachment;

		constexpr std::array dynamicStates = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
		};

		vk::PipelineDynamicStateCreateInfo dynamicState;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		vk::PipelineRenderingCreateInfo renderingInfo;
		renderingInfo.colorAttachmentCount = 1;
		vk::Format colorFormat = VulkanUtils::ToVulkanFormat(m_CreateInfo.ColorFormat);
		vk::Format depthFormat = VulkanUtils::ToVulkanFormat(TextureFormat::Undefined);
		renderingInfo.pColorAttachmentFormats = &colorFormat;
		renderingInfo.depthAttachmentFormat = depthFormat;

		vk::GraphicsPipelineCreateInfo pipelineCreateInfo;
		pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineCreateInfo.pStages = shaderStages.data();
		pipelineCreateInfo.pVertexInputState = &vertexInputState;
		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		pipelineCreateInfo.layout = *m_PipelineLayout;
		pipelineCreateInfo.renderPass = nullptr;

		vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain{ pipelineCreateInfo, renderingInfo };
		
		m_Pipeline = vk::raii::Pipeline(m_VulkanDevice.GetDevice(), nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
	}
}
