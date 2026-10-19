#include "gfx_memory.h"
#include "vk/gfx_vk_funcs.h"

static VmaVulkanFunctions s_vmaFuncs{};
static VmaAllocator s_allocator = VK_NULL_HANDLE;


static void SetFnPtrs()
{
#define PASS_FN( fn ) s_vmaFuncs.fn = fn
	PASS_FN( vkGetPhysicalDeviceProperties );
	PASS_FN( vkGetPhysicalDeviceMemoryProperties );
	PASS_FN( vkAllocateMemory );
	PASS_FN( vkFreeMemory );
	PASS_FN( vkMapMemory );
	PASS_FN( vkUnmapMemory );
	PASS_FN( vkFlushMappedMemoryRanges );
	PASS_FN( vkInvalidateMappedMemoryRanges );
	PASS_FN( vkBindBufferMemory );
	PASS_FN( vkBindImageMemory );
	PASS_FN( vkGetBufferMemoryRequirements );
	PASS_FN( vkGetImageMemoryRequirements );
	PASS_FN( vkCreateBuffer );
	PASS_FN( vkDestroyBuffer );
	PASS_FN( vkCreateImage );
	PASS_FN( vkDestroyImage );
	PASS_FN( vkCmdCopyBuffer );

#define PASS_FN_KHR( fn ) s_vmaFuncs.fn##KHR = fn
	PASS_FN_KHR( vkGetBufferMemoryRequirements2 );
	PASS_FN_KHR( vkGetImageMemoryRequirements2 );
	PASS_FN_KHR( vkBindBufferMemory2 );
	PASS_FN_KHR( vkBindImageMemory2 );
	PASS_FN_KHR( vkGetPhysicalDeviceMemoryProperties2 );
}


void InitAllocator( VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device )
{
	Com_Assert_Fatal( s_allocator == VK_NULL_HANDLE );

	SetFnPtrs();

	VmaAllocatorCreateInfo createInfo
	{
		.flags = VMA_ALLOCATOR_CREATE_AMD_DEVICE_COHERENT_MEMORY_BIT
			   | VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
		.physicalDevice = physicalDevice,
		.device = device,
		.pVulkanFunctions = &s_vmaFuncs,
		.instance = instance,
		.vulkanApiVersion = VK_API_VERSION_1_4,
	};

	VK_CALL( vmaCreateAllocator( &createInfo, &s_allocator ) );
}


void FinalizeAllocator()
{
	vmaDestroyAllocator( s_allocator );
}


void AllocImage( const VkImageCreateInfo &imgInfo, VkImage &img, VmaAllocation &alloc, bool dedicatedMemory )
{
	const VmaAllocationCreateInfo allocInfo =
	{
		.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
		.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	};

	VK_CALL( vmaCreateImage( s_allocator, &imgInfo, &allocInfo, &img, &alloc, nullptr ) );
}


void FreeImage( VkImage &img, VmaAllocation &alloc )
{
	vmaDestroyImage( s_allocator, img, alloc );
}