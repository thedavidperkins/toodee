#include "gfx_command_buffer.h"
#include "gfx_image.h"
#include "gfx_memory.h"
#include "vk/gfx_vk_struct_factory.h"
#include "vk/gfx_vk_vma.h"

#include "common/com_debug.h"


static VkPipelineStageFlags2 GetStageFlagsForUsage( GfxImgUsage usage )
{
	switch ( usage )
	{
	case GfxImgUsage::TransferSrc:
	case GfxImgUsage::TransferDst:
		return VK_PIPELINE_STAGE_2_TRANSFER_BIT;
	case GfxImgUsage::PresentSrc:
	default:
		return VK_PIPELINE_STAGE_NONE;
	}
}


static VkAccessFlags2 GetAccessFlagsForUsage( GfxImgUsage usage )
{
	switch ( usage )
	{
	case GfxImgUsage::TransferSrc:
		return VK_ACCESS_2_TRANSFER_READ_BIT;
	case GfxImgUsage::TransferDst:
		return VK_ACCESS_2_TRANSFER_WRITE_BIT;
	case GfxImgUsage::PresentSrc:
	default:
		return VK_ACCESS_2_NONE;
	}
}


static VkImageLayout GetLayoutForUsage( GfxImgUsage usage )
{
	switch ( usage )
	{
	case GfxImgUsage::TransferSrc:
		return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	case GfxImgUsage::TransferDst:
		return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	case GfxImgUsage::PresentSrc:
		return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	default:
		return VK_IMAGE_LAYOUT_GENERAL;
	}
}


void GfxImage::Init( VkImage img, uint width, uint height, uint depth, uint mipCount, uint layerCount, VkFormat fmt, uint queueFamilyIdx )
{
	Com_Assert_Fatal( m_image == VK_NULL_HANDLE );
	Com_Assert_Fatal( m_queueFamilyIdx == ~0u );

	m_image = img;

	m_resourceOwned = false;
	m_mipCount = mipCount;
	m_layerCount = layerCount;
	m_width = width;
	m_height = height;
	m_depth = depth;
	m_format = fmt;
	m_queueFamilyIdx = queueFamilyIdx;
}


void GfxImage::Init( uint width, uint height, uint depth, uint mipCount, uint layerCount, VkFormat fmt, uint queueFamilyIdx )
{
	Com_Assert_Fatal( m_image == VK_NULL_HANDLE );
	Com_Assert_Fatal( m_queueFamilyIdx == ~0u );

	VkImageType imgType = VK_IMAGE_TYPE_1D;
	if ( depth > 1u )
	{
		Com_Assert_Fatal( height > 1u && width > 1u );
		imgType = VK_IMAGE_TYPE_3D;
	}
	else if ( height > 1u )
	{
		Com_Assert_Fatal( width > 1u );
		imgType = VK_IMAGE_TYPE_2D;
	}

	BEGIN_INFO_STRUCT( VkImageCreateInfo, imgInfo )
		.imageType = imgType,
		.format = fmt,
		.extent = { width, height, depth },
		.mipLevels = mipCount,
		.arrayLayers = layerCount,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
			   | VK_IMAGE_USAGE_TRANSFER_DST_BIT
			   | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
			   | VK_IMAGE_USAGE_SAMPLED_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 1,
		.pQueueFamilyIndices = &queueFamilyIdx,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	END_INFO_STRUCT;

	static constexpr bool dedicated = true;
	AllocImage( imgInfo, m_image, m_allocation, dedicated );

	m_resourceOwned = true;
	m_mipCount = mipCount;
	m_layerCount = layerCount;
	m_width = width;
	m_height = height;
	m_depth = depth;
	m_format = fmt;
	m_queueFamilyIdx = queueFamilyIdx;
}


void GfxImage::Finalize()
{
	if ( m_resourceOwned )
	{
		FreeImage( m_image, m_allocation );
	}

	m_image = VK_NULL_HANDLE;
	m_allocation = VK_NULL_HANDLE;
	m_queueFamilyIdx = ~0u;
}


VkImageMemoryBarrier2 GfxImage::GenerateAndUpdateImageTransitionInfo( GfxImgUsage usage, uint queueFamilyIdx )
{
	VkImageSubresourceRange range =
	{
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0,
		.levelCount = VK_REMAINING_MIP_LEVELS,
		.baseArrayLayer = 0,
		.layerCount = VK_REMAINING_ARRAY_LAYERS,
	};

	BEGIN_INFO_STRUCT( VkImageMemoryBarrier2, transitionInfo )
		.srcStageMask = m_pipelineStageFlags,
		.srcAccessMask = m_accessFlags,
		.dstStageMask = GetStageFlagsForUsage( usage ),
		.dstAccessMask = GetAccessFlagsForUsage( usage ),
		.oldLayout = m_layout,
		.newLayout = GetLayoutForUsage( usage ),
		.srcQueueFamilyIndex = m_queueFamilyIdx,
		.dstQueueFamilyIndex = queueFamilyIdx,
		.image = m_image,
		.subresourceRange = range,
	END_INFO_STRUCT;

	m_pipelineStageFlags = transitionInfo.dstStageMask;
	m_accessFlags = transitionInfo.dstAccessMask;
	m_layout = transitionInfo.newLayout;
	m_queueFamilyIdx = transitionInfo.dstQueueFamilyIndex;

	return transitionInfo;
}
