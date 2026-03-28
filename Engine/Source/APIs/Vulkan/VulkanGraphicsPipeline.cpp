#include "kepch.h"
#include "VulkanGraphicsPipeline.h"

#include "VulkanDevice.h"

namespace Engine
{
	VulkanGraphicsPipeline::VulkanGraphicsPipeline(VulkanDevice& device, GraphicsPipelineCreateInfo createInfo)
		: GraphicsPipeline(std::move(createInfo)), m_Device(device)
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

		m_VertexShader = CreateScope<VulkanShader>(m_Device, m_CreateInfo.VertexShader, LoadShaderBinary(vertexPath));
		m_FragmentShader = CreateScope<VulkanShader>(m_Device, m_CreateInfo.FragmentShader, LoadShaderBinary(fragmentPath));

		LOG(LogLevel::Info, "Loaded graphics shaders: {}, {}", vertexPath.string(), fragmentPath.string());
	}

	void VulkanGraphicsPipeline::CreatePipelineCache()
	{
		vk::PipelineCacheCreateInfo createInfo;
		m_PipelineCache = vk::raii::PipelineCache(m_Device.GetDevice(), createInfo);
	}

	void VulkanGraphicsPipeline::CreateDescriptorSetLayoutStub()
	{
		vk::DescriptorSetLayoutCreateInfo createInfo;
		m_DescriptorSetLayout = vk::raii::DescriptorSetLayout(m_Device.GetDevice(), createInfo);
	}

	void VulkanGraphicsPipeline::CreatePipelineLayout()
	{
		vk::PipelineLayoutCreateInfo createInfo;

		m_PipelineLayout = vk::raii::PipelineLayout(m_Device.GetDevice(), createInfo);
	}

	void VulkanGraphicsPipeline::CreatePipeline()
	{
		const std::array shaderStages = {
			vk::PipelineShaderStageCreateInfo({}, m_VertexShader->GetVulkanStage(), *m_VertexShader->GetShaderModule(), m_VertexShader->GetEntryPoint().c_str()),
			vk::PipelineShaderStageCreateInfo({}, m_FragmentShader->GetVulkanStage(), *m_FragmentShader->GetShaderModule(), m_FragmentShader->GetEntryPoint().c_str())
		};

		vk::PipelineVertexInputStateCreateInfo vertexInputState;

		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState;
		inputAssemblyState.topology = m_CreateInfo.Config.Topology;
		inputAssemblyState.primitiveRestartEnable = vk::False;

		const vk::Viewport viewport(0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f);
		const vk::Rect2D scissor(vk::Offset2D(0, 0), vk::Extent2D(1, 1));

		vk::PipelineViewportStateCreateInfo viewportState;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		vk::PipelineRasterizationStateCreateInfo rasterizationState;
		rasterizationState.depthClampEnable = vk::False;
		rasterizationState.rasterizerDiscardEnable = vk::False;
		rasterizationState.polygonMode = m_CreateInfo.Config.PolygonMode;
		rasterizationState.cullMode = m_CreateInfo.Config.CullMode;
		rasterizationState.frontFace = m_CreateInfo.Config.FrontFace;
		rasterizationState.lineWidth = 1.0f;

		vk::PipelineMultisampleStateCreateInfo multisampleState;
		multisampleState.rasterizationSamples = m_CreateInfo.SampleCount;
		multisampleState.sampleShadingEnable = vk::False;

		vk::PipelineDepthStencilStateCreateInfo depthStencilState;
		depthStencilState.depthTestEnable = m_CreateInfo.Config.DepthTest;
		depthStencilState.depthWriteEnable = m_CreateInfo.Config.DepthTest;
		depthStencilState.depthCompareOp = vk::CompareOp::eLess;
		depthStencilState.depthBoundsTestEnable = vk::False;
		depthStencilState.stencilTestEnable = vk::False;

		const vk::PipelineColorBlendAttachmentState colorBlendAttachment(
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

		const std::array dynamicStates = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
		};

		vk::PipelineDynamicStateCreateInfo dynamicState;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
		dynamicState.pDynamicStates = dynamicStates.data();

		vk::PipelineRenderingCreateInfo pipelineRenderingInfo;
		pipelineRenderingInfo.colorAttachmentCount = 1;
		pipelineRenderingInfo.pColorAttachmentFormats = &m_CreateInfo.ColorFormat;
		pipelineRenderingInfo.depthAttachmentFormat = m_CreateInfo.DepthFormat;

		vk::GraphicsPipelineCreateInfo createInfo;
		createInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		createInfo.pStages = shaderStages.data();
		createInfo.pVertexInputState = &vertexInputState;
		createInfo.pInputAssemblyState = &inputAssemblyState;
		createInfo.pViewportState = &viewportState;
		createInfo.pRasterizationState = &rasterizationState;
		createInfo.pMultisampleState = &multisampleState;
		createInfo.pDepthStencilState = &depthStencilState;
		createInfo.pColorBlendState = &colorBlendState;
		createInfo.pDynamicState = &dynamicState;
		createInfo.layout = *m_PipelineLayout;
		createInfo.pNext = &pipelineRenderingInfo;

		m_Pipeline = m_Device.GetDevice().createGraphicsPipeline(m_PipelineCache, createInfo);
	}
}
