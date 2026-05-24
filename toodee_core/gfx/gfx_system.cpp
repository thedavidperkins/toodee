#include "common/com_types.h"
#include "common/com_debug.h"
#include "common/com_utils.h"

#include "gfx_system.h"

#include "vk/gfx_vk_funcs.h"
#include "vk/gfx_vk_struct_factory.h"

void GfxSystem::InitVulkanInstance()
{
	uint extensionsCount = 0u;
	vkEnumerateInstanceExtensionProperties( nullptr, &extensionsCount, nullptr );
	Com_Assert_Fatal( extensionsCount > 0u );

	std::vector<VkExtensionProperties> availableExtensions( extensionsCount );
	vkEnumerateInstanceExtensionProperties( nullptr, &extensionsCount, availableExtensions.data() );

	const char *const desiredExtensions[] =
	{
		VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
		VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME,
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
	};

	m_enabledInstanceExtensions.reserve( ARRAY_COUNT( desiredExtensions ) );
	for ( const char *const desiredExtension : desiredExtensions )
	{
		auto it = std::find_if( availableExtensions.begin(), availableExtensions.end(), [&desiredExtension]( const VkExtensionProperties &ext )
		{
			return std::strcmp( ext.extensionName, desiredExtension ) == 0;
		} );

		Com_Assert_Warn( it != availableExtensions.end() );
		m_enabledInstanceExtensions.push_back( desiredExtension );
	}

	BEGIN_INFO_STRUCT( VkApplicationInfo, appInfo )
		.pApplicationName = "TooDeeTesting",
		.applicationVersion = VK_MAKE_VERSION( 0, 0, 0 ),
		.pEngineName = "TooDeeEngine",
		.engineVersion = VK_MAKE_VERSION( 0, 0, 0 ),
		.apiVersion = VK_API_VERSION_1_4,
		END_INFO_STRUCT;

	BEGIN_INFO_STRUCT( VkInstanceCreateInfo, instInfo )
		.pApplicationInfo = &appInfo,
		.enabledExtensionCount = static_cast<uint>( m_enabledInstanceExtensions.size() ),
		.ppEnabledExtensionNames = m_enabledInstanceExtensions.data(),
		END_INFO_STRUCT;

	VK_CALL( vkCreateInstance( &instInfo, nullptr, &m_instance ) );
}


void GfxSystem::CreateVulkanDevice()
{
	uint deviceCount = 0;
	vkEnumeratePhysicalDevices( m_instance, &deviceCount, nullptr );

	std::vector<VkPhysicalDevice> devices( deviceCount );
	vkEnumeratePhysicalDevices( m_instance, &deviceCount, devices.data() );

	m_physicalDevice = devices[0];

	CREATE_INFO_STRUCT( VkPhysicalDeviceVulkan14Features, features14 );
	BEGIN_INFO_STRUCT( VkPhysicalDeviceVulkan13Features, features13 )
		.pNext = &features14,
		END_INFO_STRUCT;
	BEGIN_INFO_STRUCT( VkPhysicalDeviceVulkan12Features, features12 )
		.pNext = &features13,
		END_INFO_STRUCT;
	BEGIN_INFO_STRUCT( VkPhysicalDeviceVulkan11Features, features11 )
		.pNext = &features12,
		END_INFO_STRUCT;
	BEGIN_INFO_STRUCT( VkPhysicalDeviceFeatures2, features )
		.pNext = &features11,
		END_INFO_STRUCT;

	CREATE_INFO_STRUCT( VkPhysicalDeviceVulkan14Properties, properties14 );
	BEGIN_INFO_STRUCT( VkPhysicalDeviceVulkan13Properties, properties13 )
		.pNext = &properties14,
		END_INFO_STRUCT;
	BEGIN_INFO_STRUCT( VkPhysicalDeviceVulkan12Properties, properties12 )
		.pNext = &properties13,
		END_INFO_STRUCT;
	BEGIN_INFO_STRUCT( VkPhysicalDeviceVulkan11Properties, properties11 )
		.pNext = &properties12,
		END_INFO_STRUCT;
	BEGIN_INFO_STRUCT( VkPhysicalDeviceProperties2, properties )
		.pNext = &properties11,
		END_INFO_STRUCT;

	vkGetPhysicalDeviceProperties2( m_physicalDevice, &properties );
	vkGetPhysicalDeviceFeatures2( m_physicalDevice, &features );

	Com_Assert_Fatal( properties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU );

	uint extensionCount = 0;
	vkEnumerateDeviceExtensionProperties( m_physicalDevice, nullptr, &extensionCount, nullptr );

	std::vector<VkExtensionProperties> availableExtensions( extensionCount );
	vkEnumerateDeviceExtensionProperties( m_physicalDevice, nullptr, &extensionCount, availableExtensions.data() );

	const char *const desiredExtensions[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

	m_enabledDeviceExtensions.reserve( ARRAY_COUNT( desiredExtensions ) );
	for ( const char *const desiredExtension : desiredExtensions )
	{
		auto it = std::find_if( availableExtensions.begin(), availableExtensions.end(), [&desiredExtension]( const VkExtensionProperties &ext )
		{
			return std::strcmp( ext.extensionName, desiredExtension ) == 0;
		} );

		Com_Assert_Warn( it != availableExtensions.end() );
		m_enabledDeviceExtensions.push_back( desiredExtension );
	}

	uint queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties2( m_physicalDevice, &queueFamilyCount, nullptr );

	std::vector<VkQueueFamilyProperties2> queueFamilies( queueFamilyCount );
	vkGetPhysicalDeviceQueueFamilyProperties2( m_physicalDevice, &queueFamilyCount, queueFamilies.data() );

	auto gfxQueueIt = std::find_if( queueFamilies.begin(), queueFamilies.end(), []( const VkQueueFamilyProperties2 &queueFamily )
	{
		const VkQueueFamilyProperties &props = queueFamily.queueFamilyProperties;
		return props.queueCount > 0 && ( props.queueFlags & VK_QUEUE_GRAPHICS_BIT );
	} );
	Com_Assert_Fatal( gfxQueueIt != queueFamilies.end() );
	const uint gfxIdx = static_cast<uint>( std::distance( queueFamilies.begin(), gfxQueueIt ) );

	auto computeQueueIt = std::find_if( queueFamilies.begin(), queueFamilies.end(), []( const VkQueueFamilyProperties2 &queueFamily )
	{
		const VkQueueFamilyProperties &props = queueFamily.queueFamilyProperties;
		return props.queueCount > 0 && ( props.queueFlags & VK_QUEUE_COMPUTE_BIT );
	} );
	Com_Assert_Fatal( computeQueueIt != queueFamilies.end() );
	const uint computeIdx = static_cast<uint>( std::distance( queueFamilies.begin(), computeQueueIt ) );

	constexpr float queuePriority = 1.f;

	BEGIN_INFO_STRUCT( VkDeviceQueueCreateInfo, gfxQueue )
		.queueFamilyIndex = gfxIdx,
		.queueCount = 1u,
		.pQueuePriorities = &queuePriority,
		END_INFO_STRUCT;

	BEGIN_INFO_STRUCT( VkDeviceQueueCreateInfo, computeQueue )
		.queueFamilyIndex = computeIdx,
		.queueCount = 1u,
		.pQueuePriorities = &queuePriority,
		END_INFO_STRUCT;

	VkDeviceQueueCreateInfo queueInfos[] =
	{
		gfxQueue,
		computeQueue,
	};

	BEGIN_INFO_STRUCT( VkDeviceCreateInfo, deviceInfo )
		.pNext = &features,
		.queueCreateInfoCount = ARRAY_COUNT( queueInfos ),
		.pQueueCreateInfos = queueInfos,
		.enabledExtensionCount = static_cast<uint>( m_enabledDeviceExtensions.size() ),
		.ppEnabledExtensionNames = m_enabledDeviceExtensions.data(),
		END_INFO_STRUCT;

	VK_CALL( vkCreateDevice( m_physicalDevice, &deviceInfo, nullptr, &m_device ) );
}


void GfxSystem::DestroyVulkanDevice()
{
	vkDestroyDevice( m_device, nullptr );
	m_device = VK_NULL_HANDLE;
}


void GfxSystem::ShutdownVulkanInstance()
{
	vkDestroyInstance( m_instance, nullptr );
	m_instance = VK_NULL_HANDLE;
}


GfxSystem::GfxSystem()
{
	LoadGlobalVulkanFuncs();
	InitVulkanInstance();
	LoadInstanceVulkanFuncs( m_instance, m_enabledInstanceExtensions );
	CreateVulkanDevice();
}


GfxSystem::~GfxSystem()
{
	DestroyVulkanDevice();
	ShutdownVulkanInstance();
	ReleaseVulkanLib();
}