#pragma once

#include "vk/gfx_vk_api.h"
#include "vk/gfx_vk_vma.h"

#include "common/com_types.h"

#include <unordered_map>

class GfxCmdBfr;

enum class GfxImgUsage : byte
{
	TransferSrc,
	TransferDst,
	PresentSrc,
	RenderTarget,
};


class GfxImage
{
public:
	GfxImage() {};
	GfxImage( const GfxImage &rhs ) = delete;
	GfxImage( GfxImage &&rhs ) = delete;
	GfxImage &operator=( const GfxImage &rhs ) = delete;
	GfxImage &operator=( GfxImage &&rhs ) = delete;

	void Init( VkDevice device, VkImage img, uint width, uint height, uint depth, uint mipCount, uint layerCount, VkFormat fmt, uint queueFamilyIdx ); // init a wrapper for an unowned, single-mip, single-layer resource (e.g. swapchain image)
	void Init( VkDevice device, uint width, uint height, uint depth, uint mipCount, uint layerCount, VkFormat fmt, uint queueFamilyIdx );
	void Finalize();

	bool ReadyForUsage( GfxImgUsage usage ) const;

	uvec3 Dims() const { return { m_width, m_height, m_depth }; }
	uint Width() const { return m_width; }
	uint Height() const { return m_height; }
	uint Depth() const { return m_depth; }
	uint MipCount() const { return m_mipCount; }
	uint LayerCount() const { return m_layerCount; }

	operator VkImage() { return m_image; }

	VkImageView View( VkImageAspectFlags aspectMask, uint baseMip, uint mipCount, uint baseLayer, uint layerCount );

private:
	VkImageMemoryBarrier2 GenerateAndUpdateImageTransitionInfo( GfxImgUsage usage, uint queueFamilyIndex );
	VkImageViewType GenerateViewType() const;

	VkDevice m_device = VK_NULL_HANDLE;
	VkImage m_image = VK_NULL_HANDLE;
	std::unordered_map<size_t, VkImageView> m_views;

	VkPipelineStageFlags2 m_pipelineStageFlags = VK_PIPELINE_STAGE_2_NONE;
	VkAccessFlags2 m_accessFlags = VK_ACCESS_2_NONE;
	VkImageLayout m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
	uint m_queueFamilyIdx = ~0u;

	uint m_mipCount = 1u;
	uint m_layerCount = 1u;
	uint m_width = 0u;
	uint m_height = 0u;
	uint m_depth = 0u;
	VkFormat m_format = VK_FORMAT_UNDEFINED;

	VmaAllocation m_allocation = VK_NULL_HANDLE;

	bool m_resourceOwned = false;

	friend class GfxCmdBfr;
};
