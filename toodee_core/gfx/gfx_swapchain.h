#pragma once

#include "gfx_image.h"
#include "vk/gfx_vk_api.h"
#include "common/com_types.h"

class AppWindow;
class GfxCmdBfr;
class GfxFence;
struct RGFW_window;

constexpr uint kSwapchainImgCount = 3u;


struct GfxSwapchainInitInfo
{
	VkInstance instance = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	uint submitQueueFamilyIndex = ~0u;
	RGFW_window *window = nullptr;
};


class GfxSwapchain
{
public:
	void Init( VkInstance instance,
			   VkPhysicalDevice physicalDevice,
			   VkDevice device,
			   VkQueue submitQueue,
			   uint submitQueueFamilyIndex,
			   RGFW_window *window );
	void Finalize();

	GfxImage &AcquireNextImage( GfxCmdBfr &cmd, GfxFence &fence );
	void PrepForPresent( GfxCmdBfr &cmd );
	void Present();

	VkSwapchainKHR &Handle() { return m_swapchain; }
private:
	void InitTransitionCommands( uint queueIndex );
	void FinalizeTransitionCommands();

	const uint kUnsetIndex = ~0u;

	VkInstance m_instance = VK_NULL_HANDLE;
	VkDevice m_device = VK_NULL_HANDLE;
	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
	VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
	VkQueue m_submitQueue = VK_NULL_HANDLE;

	GfxImage m_swapchainImages[kSwapchainImgCount]{};
	uint m_currentIndex = kUnsetIndex; // set on AcquireNextImage, unset on Present
};