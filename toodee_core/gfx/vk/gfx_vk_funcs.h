#pragma once

#include "gfx_vk_api.h"
#include <vector>

#define EXPORTED_VULKAN_FUNCTION( function ) extern PFN_##function function;
#define GLOBAL_LEVEL_VULKAN_FUNCTION( function ) extern PFN_##function function;
#define INSTANCE_LEVEL_VULKAN_FUNCTION( function ) extern PFN_##function function;
#define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( function, extension ) extern PFN_##function function;
#define DEVICE_LEVEL_VULKAN_FUNCTION( function ) extern PFN_##function function;
#define DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( function, extension ) extern PFN_##function function;

#include "gfx_vk_funcs.inl"

void LoadGlobalVulkanFuncs();
void LoadInstanceVulkanFuncs( VkInstance instance, const std::vector<const char *> &extensions );
void LoadDeviceVulkanFuncs( VkDevice device, const std::vector<const char *> &extensions );
void ReleaseVulkanLib();

#define VK_CALL_WITH_LINE_IMPL( fn, line ) \
VkResult res##line = fn; \
Com_Assert_Warn( res##line == VK_SUCCESS );

#define VK_CALL_WITH_LINE( fn, line ) VK_CALL_WITH_LINE_IMPL( fn, line )

#define VK_CALL( fn ) VK_CALL_WITH_LINE( fn, __LINE__ )
