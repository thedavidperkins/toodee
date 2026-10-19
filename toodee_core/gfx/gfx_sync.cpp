#include "gfx_sync.h"

#include "vk/gfx_vk_struct_factory.h"
#include "vk/gfx_vk_funcs.h"

#include "common/com_debug.h"
#include "common/com_utils.h"

#include <stack>
#include <mutex>

static constexpr uint kGfxFencePoolCount = 3u;
static constexpr uint kInvalidIdx = ~0u;

static VkFence s_pool[kGfxFencePoolCount];

static VkDevice s_device = VK_NULL_HANDLE;


class ConcurrentIndexPool
{
public:
	ConcurrentIndexPool() {}
	~ConcurrentIndexPool() {}

	void push( uint val )
	{
		std::unique_lock lock( m_lockMutex );
		m_indices.push( val );
	}

	uint pop()
	{
		std::unique_lock lock( m_lockMutex );
		if ( m_indices.empty() )
		{
			return kInvalidIdx;
		}

		const uint val = m_indices.top();
		m_indices.pop();
		return val;
	}

	uint FreeCount() const
	{
		return static_cast<uint>( m_indices.size() );
	}

private:
	std::stack<uint> m_indices;
	std::mutex m_lockMutex;
};
static ConcurrentIndexPool s_idxPool;


GfxFence::GfxFence() noexcept
{
	m_poolIdx = s_idxPool.pop();
	if ( m_poolIdx < kGfxFencePoolCount )
	{
		m_fence = s_pool[m_poolIdx];
	}
#if DEBUG_BUILD
	else
	{
		Com_Assert_Fatal( m_poolIdx == kInvalidIdx );
	}
#endif // #if DEBUG_BUILD
}


GfxFence::~GfxFence() noexcept
{
	if ( m_fence != VK_NULL_HANDLE )
	{
		Com_Assert_Fatal( m_poolIdx < kGfxFencePoolCount );

		WaitAndReset();

		s_idxPool.push( m_poolIdx );
	}
}


void GfxFence::InitPool( VkDevice device )
{
	Com_AssertMsg_Fatal( s_device == VK_NULL_HANDLE, "GfxFence::InitPool called for an already initialized pool\n" );
	Com_Assert_Fatal( device != VK_NULL_HANDLE );

	s_device = device;

	CREATE_INFO_STRUCT( VkFenceCreateInfo, createInfo );
	for ( uint idx = 0u; idx < kGfxFencePoolCount; ++idx )
	{
		VK_CALL( vkCreateFence( s_device, &createInfo, nullptr, &s_pool[idx] ) );
		s_idxPool.push( idx );
	}
}


void GfxFence::FinalizePool()
{
	Com_AssertMsg_Fatal( s_device != VK_NULL_HANDLE, "GfxFence::EmptyPool called when the pool was already empty\n" );
	Com_AssertMsg_Fatal( s_idxPool.FreeCount() == kGfxFencePoolCount, "GfxFence::EmptyPool called while fences were still in use\n" );

	while ( s_idxPool.FreeCount() > 0u )
	{
		const uint nextIdx = s_idxPool.pop();
		Com_Assert_Fatal( nextIdx < kGfxFencePoolCount );

		vkDestroyFence( s_device, s_pool[nextIdx], nullptr );
		s_pool[nextIdx] = VK_NULL_HANDLE;
	}

	s_device = VK_NULL_HANDLE;
}


void GfxFence::Wait()
{
	VK_CALL( vkWaitForFences( s_device, 1, &m_fence, VK_TRUE, kFenceWaitTimeout ) );
}


void GfxFence::Reset()
{
	VK_CALL( vkResetFences( s_device, 1, &m_fence ) );
}