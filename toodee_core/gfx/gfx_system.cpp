#include "common/com_debug.h"
#include "gfx_command_buffer.h"
#include "gfx_globals.h"
#include "gfx_memory.h"
#include "gfx_sync.h"
#include "gfx_system.h"
#include "vk/gfx_vk_funcs.h"
#include "vk/gfx_vk_struct_factory.h"

#include "app/app_rgfw.h"
#include "app/app_window.h"
#include "common/com_logger.h"
#include "common/com_types.h"
#include "common/com_utils.h"

#include <algorithm>

#if DEBUG_BUILD
static VkBool32 DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT severity,
	VkDebugUtilsMessageTypeFlagsEXT types,
	const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
	void *userData )
{
	if ( severity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT )
	{
		return VK_FALSE;
	}

	ComLogLevel logLevel = severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT ? ComLogLevel::Verbose
		: severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT ? ComLogLevel::Info
		: severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT ? ComLogLevel::Warning
		: ComLogLevel::Error;

	COM_LOG( logLevel, "[VK Validation] {} ({}): {}\n", pCallbackData->pMessageIdName, pCallbackData->messageIdNumber, pCallbackData->pMessage );
	if ( pCallbackData->queueLabelCount > 0u )
	{
		COM_LOG( logLevel, "\tQueues:\n" );
		for ( uint idx = 0u; idx < pCallbackData->queueLabelCount; ++idx )
		{
			COM_LOG( logLevel, "\t\t{}\n", pCallbackData->pQueueLabels[idx].pLabelName );
		}
	}
	if ( pCallbackData->cmdBufLabelCount > 0u )
	{
		COM_LOG( logLevel, "\tCommand Buffers:\n" );
		for ( uint idx = 0u; idx < pCallbackData->cmdBufLabelCount; ++idx )
		{
			COM_LOG( logLevel, "\t\t{}\n", pCallbackData->pCmdBufLabels[idx].pLabelName );
		}
	}
	if ( pCallbackData->objectCount > 0u )
	{
		COM_LOG( logLevel, "\tObjects:\n" );
		for ( uint idx = 0u; idx < pCallbackData->objectCount; ++idx )
		{
			COM_LOG( logLevel, "\t\t{}\n", VulkanObjectTypeToString( pCallbackData->pObjects[idx].objectType ) );
		}
	}

	if ( severity > VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT )
	{
		Com_Assert_Warn( false );
	}

	return VK_FALSE;
}
#endif // #if DEBUG_BUILD


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

	std::vector<const char *> enabledInstanceExtensions;
	enabledInstanceExtensions.reserve( ARRAY_COUNT( desiredExtensions ) );
	for ( const char *const desiredExtension : desiredExtensions )
	{
		auto it = std::find_if( availableExtensions.begin(), availableExtensions.end(), [&desiredExtension]( const VkExtensionProperties &ext )
		{
			return std::strcmp( ext.extensionName, desiredExtension ) == 0;
		} );

		Com_Assert_Warn( it != availableExtensions.end() );
		enabledInstanceExtensions.push_back( desiredExtension );
	}

	std::vector<const char *> layers
	{
#if DEBUG_BUILD
		"VK_LAYER_KHRONOS_validation",
#endif // #if DEBUG_BUILD
	};

	BEGIN_INFO_STRUCT( VkApplicationInfo, appInfo )
		.pApplicationName = "TooDeeTesting",
		.applicationVersion = VK_MAKE_VERSION( 0, 0, 0 ),
		.pEngineName = "TooDeeEngine",
		.engineVersion = VK_MAKE_VERSION( 0, 0, 0 ),
		.apiVersion = VK_API_VERSION_1_4,
		END_INFO_STRUCT;

	BEGIN_INFO_STRUCT( VkInstanceCreateInfo, instInfo )
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<uint>( layers.size() ),
		.ppEnabledLayerNames = layers.data(),
		.enabledExtensionCount = static_cast<uint>( enabledInstanceExtensions.size() ),
		.ppEnabledExtensionNames = enabledInstanceExtensions.data(),
		END_INFO_STRUCT;

#if DEBUG_BUILD
	BEGIN_ASSIGN_INFO_STRUCT( VkDebugUtilsMessengerCreateInfoEXT, m_debugMsgrInfo )
		.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		.messageType = /*VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
		| */VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		.pfnUserCallback = DebugCallback,
		END_INFO_STRUCT;

	instInfo.pNext = &m_debugMsgrInfo;
#endif // #if DEBUG_BUILD

	VK_CALL( vkCreateInstance( &instInfo, nullptr, &m_instance ) );

	LoadInstanceVulkanFuncs( m_instance, enabledInstanceExtensions );
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

#if DEBUG_BUILD
	CREATE_INFO_STRUCT( VkPhysicalDeviceAddressBindingReportFeaturesEXT, bindingReportFeature );
	features14.pNext = &bindingReportFeature;
#endif // #if DEBUG_BUILD

	vkGetPhysicalDeviceProperties2( m_physicalDevice, &properties );
	vkGetPhysicalDeviceFeatures2( m_physicalDevice, &features );

	Com_Assert_Fatal( properties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU );
	Com_Assert_Fatal( features12.bufferDeviceAddress == VK_TRUE );

	uint extensionCount = 0;
	vkEnumerateDeviceExtensionProperties( m_physicalDevice, nullptr, &extensionCount, nullptr );

	std::vector<VkExtensionProperties> availableExtensions( extensionCount );
	vkEnumerateDeviceExtensionProperties( m_physicalDevice, nullptr, &extensionCount, availableExtensions.data() );

	const char *const desiredExtensions[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_AMD_DEVICE_COHERENT_MEMORY_EXTENSION_NAME,
#if DEBUG_BUILD
		VK_EXT_DEVICE_ADDRESS_BINDING_REPORT_EXTENSION_NAME,
#endif // #if DEBUG_BUILD
	};

	std::vector<const char *> enabledDeviceExtensions;
	enabledDeviceExtensions.reserve( ARRAY_COUNT( desiredExtensions ) );
	for ( const char *const desiredExtension : desiredExtensions )
	{
		auto it = std::find_if( availableExtensions.begin(), availableExtensions.end(), [&desiredExtension]( const VkExtensionProperties &ext )
		{
			return std::strcmp( ext.extensionName, desiredExtension ) == 0;
		} );

		Com_Assert_Warn( it != availableExtensions.end() );
		enabledDeviceExtensions.push_back( desiredExtension );
	}

	uint queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties2( m_physicalDevice, &queueFamilyCount, nullptr );

	std::vector<VkQueueFamilyProperties2> queueFamilies( queueFamilyCount );
	for ( VkQueueFamilyProperties2 &props : queueFamilies )
	{
		props.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
	}
	vkGetPhysicalDeviceQueueFamilyProperties2( m_physicalDevice, &queueFamilyCount, queueFamilies.data() );

	auto gfxQueueIt = std::find_if( queueFamilies.begin(), queueFamilies.end(), []( const VkQueueFamilyProperties2 &queueFamily )
	{
		const VkQueueFamilyProperties &props = queueFamily.queueFamilyProperties;
		return props.queueCount > 0 && ( props.queueFlags & VK_QUEUE_GRAPHICS_BIT );
	} );
	Com_Assert_Fatal( gfxQueueIt != queueFamilies.end() );
	m_queueFamilyIdxGfx = static_cast<uint>( std::distance( queueFamilies.begin(), gfxQueueIt ) );

	// for now we'll assume we need a separate queue family for async compute, since I previous tried to do two
	// queues from the graphics family and my card only supported one
	auto computeQueueIt = std::find_if( queueFamilies.begin(), queueFamilies.end(), [&gfxQueueIt]( const VkQueueFamilyProperties2 &queueFamily )
	{
		const VkQueueFamilyProperties &props = queueFamily.queueFamilyProperties;
		return props.queueCount > 0 && ( props.queueFlags & VK_QUEUE_COMPUTE_BIT ) && ( &queueFamily != &( *gfxQueueIt ) );
	} );
	Com_Assert_Fatal( computeQueueIt != queueFamilies.end() );
	m_queueFamilyIdxCompute = static_cast<uint>( std::distance( queueFamilies.begin(), computeQueueIt ) );

	constexpr float queuePriority = 1.f;

	BEGIN_INFO_STRUCT( VkDeviceQueueCreateInfo, gfxQueue )
		.queueFamilyIndex = m_queueFamilyIdxGfx,
		.queueCount = 1u,
		.pQueuePriorities = &queuePriority,
	END_INFO_STRUCT;

	BEGIN_INFO_STRUCT( VkDeviceQueueCreateInfo, computeQueue )
		.queueFamilyIndex = m_queueFamilyIdxCompute,
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
		.enabledExtensionCount = static_cast<uint>( enabledDeviceExtensions.size() ),
		.ppEnabledExtensionNames = enabledDeviceExtensions.data(),
	END_INFO_STRUCT;

	VK_CALL( vkCreateDevice( m_physicalDevice, &deviceInfo, nullptr, &m_device ) );

	LoadDeviceVulkanFuncs( m_device, enabledDeviceExtensions );

#if DEBUG_BUILD
	m_debugMsgrInfo.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT;
	VK_CALL( vkCreateDebugUtilsMessengerEXT( m_instance, &m_debugMsgrInfo, nullptr, &m_debugMsgr ) );
#endif // #if DEBUG_BUILD
}


void GfxSystem::DestroyVulkanDevice()
{
	VK_CALL( vkDeviceWaitIdle( m_device ) );
	vkDestroyDevice( m_device, nullptr );
	m_device = VK_NULL_HANDLE;
}


void GfxSystem::ShutdownVulkanInstance()
{
#if DEBUG_BUILD
	vkDestroyDebugUtilsMessengerEXT( m_instance, m_debugMsgr, nullptr );
	m_debugMsgr = VK_NULL_HANDLE;
#endif // #if DEBUG_BUILD

	vkDestroyInstance( m_instance, nullptr );
	m_instance = VK_NULL_HANDLE;
}


void GfxSystem::SetupQueues()
{
	vkGetDeviceQueue( m_device, m_queueFamilyIdxGfx, 0, &m_queueGfx );
	vkGetDeviceQueue( m_device, m_queueFamilyIdxCompute, 0, &m_queueCompute );

	Com_Assert_Fatal( m_queueGfx != VK_NULL_HANDLE );
	Com_Assert_Fatal( m_queueCompute != VK_NULL_HANDLE );
}


void GfxSystem::UnsetQueues()
{
	m_queueGfx = VK_NULL_HANDLE;
	m_queueCompute = VK_NULL_HANDLE;
}


void GfxSystem::InitPersistentResources()
{
	for ( GfxImage &img : m_sceneRT )
	{
		const uvec2 winDims = m_window->Dims();

		img.Init( winDims.x, winDims.y, 1u, 1u, 1u, VK_FORMAT_R8G8B8A8_UNORM, m_queueFamilyIdxGfx );
	}
}


void GfxSystem::ShutdownPersistentResources()
{
	for ( GfxImage &img : m_sceneRT )
	{
		img.Finalize();
	}
}


void GfxSystem::Init( AppWindow &window )
{
	Com_Assert_Fatal( m_window == nullptr );
	m_window = &window;

	LoadGlobalVulkanFuncs();
	InitVulkanInstance();
	CreateVulkanDevice();
	InitAllocator( m_instance, m_physicalDevice, m_device );
	SetupQueues();
	InitPersistentResources();

	m_swapchain.Init( m_instance, m_physicalDevice, m_device, m_queueGfx, m_queueFamilyIdxGfx, m_window->Handle() );
	GfxFence::InitPool( m_device );
}


void GfxSystem::Finalize()
{
	GfxFence::FinalizePool();
	m_swapchain.Finalize();
	GfxCmdBfr::FinalizePools();

	ShutdownPersistentResources();
	UnsetQueues();
	FinalizeAllocator();
	DestroyVulkanDevice();
	ShutdownVulkanInstance();
	ReleaseVulkanLib();
}


void GfxSystem::RenderFrame()
{
	static uint s_frame = 0u;

	GfxFence swapchainImgReadyFence;
	GfxFence cmdFinishedFence;
	GfxCmdBfr cmd( m_device, m_queueGfx, m_queueFamilyIdxGfx );

	cmd.BeginRecording();
	GfxImage &nextSwapchainImg = m_swapchain.AcquireNextImage( cmd, swapchainImgReadyFence );

	GfxImage &rt = m_sceneRT[s_frame % kBufferCount];
	cmd.TransitionImgForUsage( rt, GfxImgUsage::TransferDst );
	cmd.ClearColorImg( rt, g_glob.clearColor );
	cmd.TransitionImgForUsage( rt, GfxImgUsage::TransferSrc );
	cmd.BlitColor( rt, nextSwapchainImg );
	m_swapchain.PrepForPresent( cmd );

	swapchainImgReadyFence.Wait();
	cmd.EndRecordingAndSubmit( cmdFinishedFence );

	cmdFinishedFence.Wait();
	m_swapchain.Present();

	++s_frame;
}

