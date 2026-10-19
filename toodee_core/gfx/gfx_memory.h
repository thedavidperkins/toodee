#pragma once

#include "vk/gfx_vk_api.h"
#include "vk/gfx_vk_vma.h"


void InitAllocator( VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device );
void FinalizeAllocator();

void AllocImage( const VkImageCreateInfo &imgInfo, VkImage &img, VmaAllocation &alloc, bool dedicatedMemory = false );
void FreeImage( VkImage &img, VmaAllocation &alloc );