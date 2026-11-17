#include "gfx_pso.h"
#include "gfx_command_buffer.h"
#include "vk/gfx_vk_funcs.h"
#include "vk/gfx_vk_struct_factory.h"

#include "common/com_utils.h"

void GfxPipeline::Init( VkDevice device, VkShaderModule vs, VkShaderModule fs, uvec2 viewSize, VkFormat rtColorFmt )
{
	Com_Assert_Fatal( m_device == VK_NULL_HANDLE );
	m_device = device;

	//VkPushConstantRange pushConstRange{};
	//pushConstRange.stageFlags = VK_SHADER_STAGE_ALL;
	//pushConstRange.size = 256u;

	//BEGIN_INFO_STRUCT( VkPipelineLayoutCreateInfo, layoutInfo )
	//	.flags = 0,
	//	.pushConstantRangeCount = 1u,
	//	.pPushConstantRanges = &pushConstRange,
	//END_INFO_STRUCT;

	//VK_CALL( vkCreatePipelineLayout( m_device, &layoutInfo, nullptr, &m_layout ) );

	BEGIN_INFO_STRUCT( VkPipelineShaderStageCreateInfo, vsInfo )
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = vs,
		.pName = "main",
	END_INFO_STRUCT;

	BEGIN_INFO_STRUCT( VkPipelineShaderStageCreateInfo, fsInfo )
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = fs,
		.pName = "main",
	END_INFO_STRUCT;

	const VkPipelineShaderStageCreateInfo stageInfos[] =
	{
		vsInfo,
		fsInfo,
	};

	//const VkVertexInputBindingDescription vertBindingInfo =
	//{
	//	.binding = 0u,
	//	.stride = 3u * sizeof( float ),
	//	.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
	//};

	//const VkVertexInputAttributeDescription vertAttrInfo =
	//{
	//	.location = 0u,
	//	.binding = 0u,
	//	.format = VK_FORMAT_R32G32B32_SFLOAT,
	//	.offset = 0u,
	//};

	BEGIN_INFO_STRUCT( VkPipelineVertexInputStateCreateInfo, vertInfo )
		//.vertexBindingDescriptionCount = 1u,
		//.pVertexBindingDescriptions = &vertBindingInfo,
		//.vertexAttributeDescriptionCount = 1u,
		//.pVertexAttributeDescriptions = &vertAttrInfo,
	END_INFO_STRUCT;

	BEGIN_INFO_STRUCT( VkPipelineInputAssemblyStateCreateInfo, inputInfo )
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
	END_INFO_STRUCT;

	//BEGIN_INFO_STRUCT( VkPipelineTessellationStateCreateInfo, tessInfo )
	//END_INFO_STRUCT;

	const VkViewport viewport =
	{
		.x = 0.f,
		.y = 0.f,
		.width = static_cast<float>( viewSize.x ),
		.height = static_cast<float>( viewSize.y ),
		.minDepth = 0.f,
		.maxDepth = 1.f,
	};

	const VkRect2D scissor =
	{
		.offset = { 0, 0 },
		.extent = { viewSize.x, viewSize.y },
	};

	BEGIN_INFO_STRUCT( VkPipelineViewportStateCreateInfo, viewportInfo )
		.viewportCount = 1u,
		.pViewports = &viewport,
		.scissorCount = 1u,
		.pScissors = &scissor,
	END_INFO_STRUCT;

	BEGIN_INFO_STRUCT( VkPipelineRasterizationStateCreateInfo, rastInfo )
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_NONE,
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
	END_INFO_STRUCT;

	BEGIN_INFO_STRUCT( VkPipelineMultisampleStateCreateInfo, msInfo )
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
	END_INFO_STRUCT;

	//BEGIN_INFO_STRUCT( VkPipelineDepthStencilStateCreateInfo, dsInfo )
	//END_INFO_STRUCT;

	VkPipelineColorBlendAttachmentState attachmentState
	{
		.blendEnable = VK_TRUE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD,
		.colorWriteMask = VK_COLOR_COMPONENT_FLAG_BITS_MAX_ENUM,
	};

	BEGIN_INFO_STRUCT( VkPipelineColorBlendStateCreateInfo, blendInfo )
		.attachmentCount = 1u,
		.pAttachments = &attachmentState,
	END_INFO_STRUCT;

	BEGIN_INFO_STRUCT( VkPipelineDynamicStateCreateInfo, dynInfo )
		.dynamicStateCount = 0u,
	END_INFO_STRUCT;


	const VkFormat attachmentColorFmt = rtColorFmt;

	BEGIN_INFO_STRUCT( VkPipelineRenderingCreateInfo, renderingInfo )
		.colorAttachmentCount = 1u,
		.pColorAttachmentFormats = &attachmentColorFmt,
	END_INFO_STRUCT;


	BEGIN_INFO_STRUCT( VkPipelineCreateFlags2CreateInfo, flags2 )
		.pNext = &renderingInfo,
		.flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT,
	END_INFO_STRUCT;


	BEGIN_INFO_STRUCT( VkGraphicsPipelineCreateInfo, pipelineInfo )
		.pNext = &flags2,
		//.pNext = &renderingInfo,
		.flags = 0,
		.stageCount = ARRAY_COUNT( stageInfos ),
		.pStages = stageInfos,
		.pVertexInputState = &vertInfo,
		.pInputAssemblyState = &inputInfo,
		//.pTessellationState = &tessInfo,
		.pViewportState = &viewportInfo,
		.pRasterizationState = &rastInfo,
		.pMultisampleState = &msInfo,
		//.pDepthStencilState = &dsInfo,
		.pColorBlendState = &blendInfo,
		.pDynamicState = &dynInfo,
		.layout = VK_NULL_HANDLE,
		//.layout = m_layout,
		.renderPass = VK_NULL_HANDLE, // strictly dynamic rendering for now
	END_INFO_STRUCT;

	constexpr VkPipelineCache dummyCache = VK_NULL_HANDLE;
	VK_CALL( vkCreateGraphicsPipelines( m_device, dummyCache, 1, &pipelineInfo, nullptr, &m_pso ) );
}


void GfxPipeline::Finalize()
{
	vkDestroyPipeline( m_device, m_pso, nullptr );
	m_pso = VK_NULL_HANDLE;

	//vkDestroyPipelineLayout( m_device, m_layout, nullptr );
	//m_layout = VK_NULL_HANDLE;

	m_device = VK_NULL_HANDLE;
}
