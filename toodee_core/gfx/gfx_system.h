#pragma once

#include "vk/gfx_vk_api.h"
#include <memory>
#include <vector>

class GfxSystem
{
public:
	static GfxSystem &Inst()
	{
		static GfxSystem instance;
		return instance;
	}

private:
	void InitVulkanInstance();
	void ShutdownVulkanInstance();

	void CreateVulkanDevice();
	void DestroyVulkanDevice();

	void SetupQueues();


	GfxSystem();
	~GfxSystem();
	GfxSystem( const GfxSystem & ) = delete;
	GfxSystem( GfxSystem && ) = delete;
	GfxSystem &operator=( const GfxSystem & ) = delete;
	GfxSystem &operator=( GfxSystem && ) = delete;

	VkInstance m_instance = VK_NULL_HANDLE;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkDevice m_device = VK_NULL_HANDLE;

	uint m_gfxQueueFamilyIdx = 0;
	uint m_computeQueueFamilyIdx = 0;
	VkQueue m_gfxQueue = VK_NULL_HANDLE;
	VkQueue m_computeQueue = VK_NULL_HANDLE;

#if DEBUG_BUILD
	void SetupDebugMessenger();
	VkDebugUtilsMessengerCreateInfoEXT m_debugMsgrInfo{};
	VkDebugUtilsMessengerEXT m_debugMsgr = VK_NULL_HANDLE;
#endif // #if DEBUG_BUILD
};