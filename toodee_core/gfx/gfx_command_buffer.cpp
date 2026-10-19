#include "gfx_command_buffer.h"
#include "gfx_sync.h"
#include "vk/gfx_vk_struct_factory.h"
#include "vk/gfx_vk_funcs.h"

#include <bit>
#include <vector>

static constexpr uint kMaxBuffersPerPool = 5;

class GfxCmdPool
{
public:
	GfxCmdPool( VkDevice device, uint queueFamilyIndex )
		: m_device( device )
		, m_queueFamilyIndex( queueFamilyIndex )
	{
		Com_Assert_Fatal( m_device != VK_NULL_HANDLE );
		Com_Assert_Fatal( m_queueFamilyIndex != ~0u );

		BEGIN_INFO_STRUCT( VkCommandPoolCreateInfo, createInfo )
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = m_queueFamilyIndex,
			END_INFO_STRUCT;

		VK_CALL( vkCreateCommandPool( m_device, &createInfo, nullptr, &m_pool ) );
		Com_Assert_Fatal( m_pool != VK_NULL_HANDLE );

		for ( VkCommandBuffer &bfr : m_buffers )
		{
			bfr = VK_NULL_HANDLE;
		}
	}

	GfxCmdPool( const GfxCmdPool &rhs ) = delete;
	GfxCmdPool( GfxCmdPool &&rhs ) = default; // need default move ctor to satisfy vector element requirement
	GfxCmdPool &operator=( const GfxCmdPool &rhs ) = delete;
	GfxCmdPool &operator=( GfxCmdPool &&rhs ) = delete;

	~GfxCmdPool()
	{
		vkFreeCommandBuffers( m_device, m_pool, kMaxBuffersPerPool, m_buffers );
		vkDestroyCommandPool( m_device, m_pool, nullptr );
	}


	void InitBfr( uint bfrIdx )
	{
		Com_Assert_Fatal( bfrIdx < kMaxBuffersPerPool );
		Com_Assert_Fatal( m_buffers[bfrIdx] == VK_NULL_HANDLE );

		BEGIN_INFO_STRUCT( VkCommandBufferAllocateInfo, allocInfo )
			.commandPool = m_pool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
			END_INFO_STRUCT;

		VK_CALL( vkAllocateCommandBuffers( m_device, &allocInfo, &m_buffers[bfrIdx] ) );
		Com_Assert_Fatal( m_buffers[bfrIdx] != VK_NULL_HANDLE );
	}


	std::pair <VkCommandBuffer, uint> AllocBfr()
	{
		uint firstFree = std::countr_one( m_buffersInUseMask );
		Com_AssertMsg_Fatal( firstFree < kMaxBuffersPerPool, "Command buffer allocation overflow. Increase kMaxBuffersPerPool.\n" );

		if ( m_buffers[firstFree] == VK_NULL_HANDLE )
		{
			InitBfr( firstFree );
		}

		m_buffersInUseMask |= 1u << firstFree;
		return { m_buffers[firstFree], firstFree };
	}


	void FreeBfr( uint cmdBfrPoolIdx )
	{
		uint poolBit = 1u << cmdBfrPoolIdx;
		Com_Assert_Fatal( m_buffersInUseMask & poolBit );
		m_buffersInUseMask &= ~poolBit;
	}


	uint GetQueueFamilyIdx() const { return m_queueFamilyIndex; }

private:
	VkDevice m_device = VK_NULL_HANDLE;
	uint m_queueFamilyIndex = ~0u;

	VkCommandPool m_pool = VK_NULL_HANDLE;
	VkCommandBuffer m_buffers[kMaxBuffersPerPool]{};
	uint m_buffersInUseMask = 0u;
};

static std::vector<GfxCmdPool> s_pools; // map key is queue family index, for now assuming only one per queue family

static size_t GetCmdPoolListIdxForQueueFamilyIdx( uint queueFamilyIdx )
{
	auto iter = std::find_if( s_pools.begin(), s_pools.end(), [queueFamilyIdx]( const GfxCmdPool &pool )
	{
		return pool.GetQueueFamilyIdx() == queueFamilyIdx;
	} );

	if ( iter == s_pools.end() )
	{
		return s_pools.size();
	}
	else
	{
		return std::distance( s_pools.begin(), iter );
	}
}


GfxCmdBfr::GfxCmdBfr( VkDevice device, VkQueue queue, uint queueFamilyIdx )
	: m_queue( queue )
	, m_queueFamilyIdx( queueFamilyIdx )
{
	const size_t poolListIdx = GetCmdPoolListIdxForQueueFamilyIdx( m_queueFamilyIdx );

	if ( poolListIdx == s_pools.size() )
	{
		s_pools.emplace_back( device, m_queueFamilyIdx );
	}

	Com_Assert_Fatal( poolListIdx < s_pools.size() );

	auto [cmdBfr, cmdBfrPoolIdx] = s_pools[poolListIdx].AllocBfr();
	m_cmdBfr = cmdBfr;
	m_cmdBfrPoolIdx = cmdBfrPoolIdx;
}


GfxCmdBfr::~GfxCmdBfr()
{
	const size_t poolListIdx = GetCmdPoolListIdxForQueueFamilyIdx( m_queueFamilyIdx );

	Com_Assert_Fatal( s_pools.size() > poolListIdx );
	s_pools[poolListIdx].FreeBfr( m_cmdBfrPoolIdx );
}


void GfxCmdBfr::BeginRecording( bool oneTimeUse )
{
	Com_Assert_Fatal( !m_isRecording );
	CREATE_INFO_STRUCT( VkCommandBufferBeginInfo, beginInfo );
	if ( oneTimeUse )
	{
		beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	}
	VK_CALL( vkBeginCommandBuffer( m_cmdBfr, &beginInfo ) );

	m_isRecording = true;
}


void GfxCmdBfr::EndRecording()
{
	Com_Assert_Fatal( m_isRecording );
	VK_CALL( vkEndCommandBuffer( m_cmdBfr ) );
	m_isRecording = false;
}


void GfxCmdBfr::Submit( GfxFence &fence )
{
	Com_Assert_Fatal( !m_isRecording );

	BEGIN_INFO_STRUCT( VkCommandBufferSubmitInfo, cmdBfrSubmitInfo )
		.commandBuffer = m_cmdBfr,
		END_INFO_STRUCT;

	BEGIN_INFO_STRUCT( VkSubmitInfo2, submitInfo )
		.commandBufferInfoCount = 1,
		.pCommandBufferInfos = &cmdBfrSubmitInfo,
		END_INFO_STRUCT;

	VK_CALL( vkQueueSubmit2( m_queue, 1, &submitInfo, fence ) );
}


void GfxCmdBfr::ClearColorImg( GfxImage &img, VkClearColorValue color )
{
	Com_Assert_Fatal( m_isRecording );

	VkImageSubresourceRange range =
	{
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1,
	};

	vkCmdClearColorImage( m_cmdBfr, img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &color, 1, &range );
}


void GfxCmdBfr::TransitionImgForUsage( GfxImage &img, GfxImgUsage usage )
{
	Com_Assert_Fatal( m_isRecording );

	VkImageMemoryBarrier2 barrier = img.GenerateAndUpdateImageTransitionInfo( usage, m_queueFamilyIdx );
	BEGIN_INFO_STRUCT( VkDependencyInfo, transitionInfo )
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &barrier,
	END_INFO_STRUCT;

	vkCmdPipelineBarrier2( m_cmdBfr, &transitionInfo );
}


void GfxCmdBfr::BlitColor( GfxImage &src, GfxImage &dst )
{
	Com_Assert_Fatal( m_isRecording );
	Com_Assert_Fatal( src.Dims() == dst.Dims() );

	VkImageCopy copy =
	{
		.srcSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = VK_REMAINING_ARRAY_LAYERS },
		.dstSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = VK_REMAINING_ARRAY_LAYERS },
		.extent = { .width = src.Width(), .height = src.Height(), .depth = src.Depth() },
	};

	vkCmdCopyImage( m_cmdBfr, src, src.m_layout, dst, dst.m_layout, 1u, &copy );
}


void GfxCmdBfr::FinalizePools()
{
	s_pools.clear();
}