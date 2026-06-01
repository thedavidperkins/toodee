#pragma once

#include "gfx_vk_api.h"

namespace StructFactoryInternal
{
template<typename S>
inline VkStructureType GetVkStructureTypeEnum()
{
	return VK_STRUCTURE_TYPE_MAX_ENUM;
}

#define DEF_STRUCT_TYPE_TEMPLATE_SPEC( structType, enumVal )	\
template<>														\
inline VkStructureType GetVkStructureTypeEnum<structType>()		\
{																\
	return VK_STRUCTURE_TYPE_##enumVal;							\
}

DEF_STRUCT_TYPE_TEMPLATE_SPEC( VkApplicationInfo, APPLICATION_INFO )
DEF_STRUCT_TYPE_TEMPLATE_SPEC( VkInstanceCreateInfo, INSTANCE_CREATE_INFO )
DEF_STRUCT_TYPE_TEMPLATE_SPEC( VkPhysicalDeviceFeatures2, PHYSICAL_DEVICE_FEATURES_2 )
DEF_STRUCT_TYPE_TEMPLATE_SPEC( VkPhysicalDeviceVulkan11Features, PHYSICAL_DEVICE_VULKAN_1_1_FEATURES )
DEF_STRUCT_TYPE_TEMPLATE_SPEC( VkPhysicalDeviceVulkan12Features, PHYSICAL_DEVICE_VULKAN_1_2_FEATURES )
DEF_STRUCT_TYPE_TEMPLATE_SPEC( VkPhysicalDeviceVulkan13Features, PHYSICAL_DEVICE_VULKAN_1_3_FEATURES )
DEF_STRUCT_TYPE_TEMPLATE_SPEC( VkPhysicalDeviceVulkan14Features, PHYSICAL_DEVICE_VULKAN_1_4_FEATURES )
DEF_STRUCT_TYPE_TEMPLATE_SPEC( VkPhysicalDeviceAddressBindingReportFeaturesEXT, PHYSICAL_DEVICE_ADDRESS_BINDING_REPORT_FEATURES_EXT )
DEF_STRUCT_TYPE_TEMPLATE_SPEC( VkPhysicalDeviceProperties2, PHYSICAL_DEVICE_PROPERTIES_2 )
DEF_STRUCT_TYPE_TEMPLATE_SPEC( VkPhysicalDeviceVulkan11Properties, PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES )
DEF_STRUCT_TYPE_TEMPLATE_SPEC( VkPhysicalDeviceVulkan12Properties, PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES )
DEF_STRUCT_TYPE_TEMPLATE_SPEC( VkPhysicalDeviceVulkan13Properties, PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES )
DEF_STRUCT_TYPE_TEMPLATE_SPEC( VkPhysicalDeviceVulkan14Properties, PHYSICAL_DEVICE_VULKAN_1_4_PROPERTIES )
DEF_STRUCT_TYPE_TEMPLATE_SPEC( VkDebugUtilsMessengerCreateInfoEXT, DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT )
DEF_STRUCT_TYPE_TEMPLATE_SPEC( VkDeviceQueueCreateInfo, DEVICE_QUEUE_CREATE_INFO )
DEF_STRUCT_TYPE_TEMPLATE_SPEC( VkDeviceCreateInfo, DEVICE_CREATE_INFO )

#undef DEF_STRUCT_TYPE_TEMPLATE_SPEC
}

#define BEGIN_ASSIGN_INFO_STRUCT( structType, var ) \
var = { .sType = StructFactoryInternal::GetVkStructureTypeEnum<structType>(),

#define BEGIN_INFO_STRUCT( structType, var ) \
structType var; \
BEGIN_ASSIGN_INFO_STRUCT( structType, var )

#define END_INFO_STRUCT }

#define CREATE_INFO_STRUCT( structType, var )	\
	BEGIN_INFO_STRUCT( structType, var )		\
	END_INFO_STRUCT