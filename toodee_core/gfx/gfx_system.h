#pragma once

#include "gfx_swapchain.h"
#include "vk/gfx_vk_api.h"
#include "common/com_types.h"

#include <memory>
#include <vector>

class AppWindow;

constexpr uint kBufferCount = 2u;

class GfxSystem
{
public:
	static GfxSystem &Inst()
	{
		static GfxSystem instance;
		return instance;
	}

	void Init( AppWindow &window );
	void Finalize();

	void RenderFrame();

private:
	void InitVulkanInstance();
	void ShutdownVulkanInstance();

	void CreateVulkanDevice();
	void DestroyVulkanDevice();

	void SetupQueues();
	void UnsetQueues();

	void InitPersistentResources();
	void ShutdownPersistentResources();

	GfxSystem() {}
	~GfxSystem() {}
	GfxSystem( const GfxSystem & ) = delete;
	GfxSystem( GfxSystem && ) = delete;
	GfxSystem &operator=( const GfxSystem & ) = delete;
	GfxSystem &operator=( GfxSystem && ) = delete;

	AppWindow *m_window = nullptr;

	VkInstance m_instance = VK_NULL_HANDLE;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkDevice m_device = VK_NULL_HANDLE;

	uint m_queueFamilyIdxGfx = 0;
	uint m_queueFamilyIdxCompute = 0;
	VkQueue m_queueGfx = VK_NULL_HANDLE;
	VkQueue m_queueCompute = VK_NULL_HANDLE;

	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
	GfxSwapchain m_swapchain;

	GfxImage m_sceneRT[kBufferCount]{};

#if DEBUG_BUILD
	void SetupDebugMessenger();
	VkDebugUtilsMessengerCreateInfoEXT m_debugMsgrInfo{};
	VkDebugUtilsMessengerEXT m_debugMsgr = VK_NULL_HANDLE;
#endif // #if DEBUG_BUILD
};