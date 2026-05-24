#include "gfx_vk_funcs.h"
#include "common/com_debug.h"

#include <print>
#include <vector>

#define EXPORTED_VULKAN_FUNCTION( function ) PFN_##function function;
#define GLOBAL_LEVEL_VULKAN_FUNCTION( function ) PFN_##function function;
#define INSTANCE_LEVEL_VULKAN_FUNCTION( function ) PFN_##function function;
#define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( function, extension ) PFN_##function function;
#define DEVICE_LEVEL_VULKAN_FUNCTION( function ) PFN_##function function;
#define DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( function, extension ) PFN_##function function;

#include "gfx_vk_funcs.inl"

static HMODULE s_vkLibrary{};

static HMODULE LoadVulkanDLL()
{
	return LoadLibraryEx( VULKAN_DLL, nullptr, 0 );
}


void LoadGlobalVulkanFuncs()
{
	s_vkLibrary = LoadVulkanDLL();
	Com_Assert_Fatal( s_vkLibrary != nullptr );

#define EXPORTED_VULKAN_FUNCTION( function )									\
	function = (PFN_##function)GetProcAddress( s_vkLibrary, #function );		\
	Com_Assert_Fatal( function != nullptr );

#define GLOBAL_LEVEL_VULKAN_FUNCTION( function )								\
	function = (PFN_##function)vkGetInstanceProcAddr( nullptr, #function );		\
	Com_Assert_Fatal( function != nullptr );

#include "gfx_vk_funcs.inl"
}


void LoadInstanceVulkanFuncs( VkInstance instance, const std::vector<const char *> &extensions )
{
#define INSTANCE_LEVEL_VULKAN_FUNCTION( function )								\
	function = (PFN_##function)vkGetInstanceProcAddr( instance, #function );	\
	Com_Assert_Fatal( function != nullptr );

#define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( function, extension )			\
	for ( const char *ext : extensions )												\
	{																					\
		if ( std::strcmp( ext, extension ) == 0 )										\
		{																				\
			function = (PFN_##function)vkGetInstanceProcAddr( instance, #function );	\
			Com_Assert_Fatal( function != nullptr );									\
			break;																		\
		}																				\
	}

#include "gfx_vk_funcs.inl"
}

void ReleaseVulkanLib()
{
	FreeLibrary( s_vkLibrary );
}