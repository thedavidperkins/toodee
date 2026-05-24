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

	GfxSystem();
	~GfxSystem();
	GfxSystem( const GfxSystem & ) = delete;
	GfxSystem( GfxSystem && ) = delete;
	GfxSystem &operator=( const GfxSystem & ) = delete;
	GfxSystem &operator=( GfxSystem && ) = delete;

	VkInstance m_instance;
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_device;
	std::vector<const char *> m_enabledInstanceExtensions;
	std::vector<const char *> m_enabledDeviceExtensions;
};