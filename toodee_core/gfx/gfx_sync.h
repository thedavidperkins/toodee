#pragma once

#include "vk/gfx_vk_api.h"

#include "common/com_types.h"

#include <atomic>
#include <memory>

static constexpr uint kFenceWaitTimeout = 1u << 30;

class GfxFence
{
public:
	GfxFence() noexcept;
	GfxFence( const GfxFence &rhs ) = delete;
	GfxFence &operator=( const GfxFence &rhs ) = delete;
	GfxFence( GfxFence &&rhs ) = delete;
	GfxFence &operator=( GfxFence &&rhs ) = delete;

	~GfxFence() noexcept;

	static void InitPool( VkDevice device );
	static void FinalizePool();

	void Wait();
	void Reset();
	inline void WaitAndReset()
	{
		Wait();
		Reset();
	}

	operator bool() const { return m_fence != VK_NULL_HANDLE; }
	operator VkFence() const { return m_fence; }

private:
	uint m_poolIdx = ~0u;
	VkFence m_fence = VK_NULL_HANDLE;
};
