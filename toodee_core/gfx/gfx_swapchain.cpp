#include "gfx_command_buffer.h"
#include "gfx_sync.h"
#include "gfx_swapchain.h"
#include "vk/gfx_vk_funcs.h"
#include "vk/gfx_vk_struct_factory.h"

#include "app/app_rgfw.h"
#include "app/app_window.h"
#include "common/com_debug.h"
#include "common/com_utils.h"


void GfxSwapchain::Init( VkInstance instance,
						 VkPhysicalDevice physicalDevice,
						 VkDevice device,
						 VkQueue submitQueue,
						 uint submitQueueFamilyIndex,
						 RGFW_window *window )
{
	Com_Assert_Fatal( m_instance == VK_NULL_HANDLE );
	Com_Assert_Fatal( m_device == VK_NULL_HANDLE );
	Com_Assert_Fatal( m_surface == VK_NULL_HANDLE );
	Com_Assert_Fatal( m_swapchain == VK_NULL_HANDLE );
	Com_Assert_Fatal( m_submitQueue == VK_NULL_HANDLE );

	Com_Assert_Fatal( instance != VK_NULL_HANDLE );
	Com_Assert_Fatal( physicalDevice != VK_NULL_HANDLE );
	Com_Assert_Fatal( device != VK_NULL_HANDLE );
	Com_Assert_Fatal( submitQueue != VK_NULL_HANDLE );

	m_instance = instance;
	m_device = device;
	m_submitQueue = submitQueue;

	VK_CALL( RGFW_window_createSurface_Vulkan( window, m_instance, &m_surface ) );
	Com_Assert_Fatal( m_surface != VK_NULL_HANDLE );

	VkBool32 supported = VK_FALSE;
	VK_CALL( vkGetPhysicalDeviceSurfaceSupportKHR( physicalDevice, submitQueueFamilyIndex, m_surface, &supported ) );

	Com_Assert_Fatal( supported == VK_TRUE );

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR; // guaranteed available
	Com_Assert_Fatal( m_surface != VK_NULL_HANDLE );

	uint presentModeCount = 0;
	VK_CALL( vkGetPhysicalDeviceSurfacePresentModesKHR( physicalDevice, m_surface, &presentModeCount, nullptr ) );
	Com_Assert_Fatal( presentModeCount > 0 );

	std::vector<VkPresentModeKHR> modesAvailable;
	modesAvailable.resize( presentModeCount );

	VK_CALL( vkGetPhysicalDeviceSurfacePresentModesKHR( physicalDevice, m_surface, &presentModeCount, modesAvailable.data() ) );

	// Keeping it simple: mailbox if available, FIFO otherwise
	for ( const VkPresentModeKHR &mode : modesAvailable )
	{
		if ( mode == VK_PRESENT_MODE_MAILBOX_KHR )
		{
			presentMode = mode;
			return;
		}
	}

	BEGIN_INFO_STRUCT( VkPhysicalDeviceSurfaceInfo2KHR, surfaceInfo )
		.surface = m_surface,
	END_INFO_STRUCT;

#if DEBUG_BUILD
	CREATE_INFO_STRUCT( VkSurfaceCapabilities2KHR, surfaceCapabilities );
	VK_CALL( vkGetPhysicalDeviceSurfaceCapabilities2KHR( physicalDevice, &surfaceInfo, &surfaceCapabilities ) );

	// Confirm that we can use our hard-coded swapchain image count
	Com_Assert_Fatal( surfaceCapabilities.surfaceCapabilities.maxImageCount >= kSwapchainImgCount );
	Com_Assert_Fatal( surfaceCapabilities.surfaceCapabilities.minImageCount <= kSwapchainImgCount );

	// Confirm that the window governs the swapchain image size. If this isn't true, the window relies on the image for sizing
	Com_Assert_Fatal( surfaceCapabilities.surfaceCapabilities.currentExtent.width != 0xFFFFFFFF );
#endif // #if DEBUG_BUILD

	uint formatCount = 0;
	VK_CALL( vkGetPhysicalDeviceSurfaceFormats2KHR( physicalDevice, &surfaceInfo, &formatCount, nullptr ) );
	Com_Assert_Fatal( formatCount > 0 );

	std::vector<VkSurfaceFormat2KHR> formats;
	CREATE_INFO_STRUCT( VkSurfaceFormat2KHR, emptyFormat );
	formats.resize( formatCount );
	std::fill( formats.begin(), formats.end(), emptyFormat );

	// for simplicity, we'll take the first supported format until there's a reason not to.
	VK_CALL( vkGetPhysicalDeviceSurfaceFormats2KHR( physicalDevice, &surfaceInfo, &formatCount, formats.data() ) );

	BEGIN_INFO_STRUCT( VkSwapchainCreateInfoKHR, swapchainInfo )
		.surface = m_surface,
		.minImageCount = kSwapchainImgCount,
		.imageFormat = formats[0].surfaceFormat.format,
		.imageColorSpace = formats[0].surfaceFormat.colorSpace,
		.imageExtent = surfaceCapabilities.surfaceCapabilities.currentExtent,
		.imageArrayLayers = 1u,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = presentMode,
		.clipped = VK_TRUE,
		END_INFO_STRUCT;
	VK_CALL( vkCreateSwapchainKHR( m_device, &swapchainInfo, nullptr, &m_swapchain ) );

	Com_Assert_Fatal( m_swapchain != VK_NULL_HANDLE );

#if DEBUG_BUILD
	uint swapchainImgCountActual = 0;
	VK_CALL( vkGetSwapchainImagesKHR( m_device, m_swapchain, &swapchainImgCountActual, nullptr ) );
	Com_Assert_Fatal( kSwapchainImgCount == swapchainImgCountActual );
#endif // #if DEBUG_BUILD
	VkImage swapchainImages[kSwapchainImgCount];
	VK_CALL( vkGetSwapchainImagesKHR( m_device, m_swapchain, &swapchainImgCountActual, swapchainImages ) );

	for ( uint i = 0; i < kSwapchainImgCount; ++i )
	{
		m_swapchainImages[i].Init( swapchainImages[i],
								   swapchainInfo.imageExtent.width,
								   swapchainInfo.imageExtent.height,
								   1u,
								   1u,
								   1u,
								   swapchainInfo.imageFormat,
								   submitQueueFamilyIndex );
	}
}


void GfxSwapchain::Finalize()
{
	vkDeviceWaitIdle( m_device );

	for ( GfxImage &img : m_swapchainImages )
	{
		img.Finalize();
	}

	vkDestroySwapchainKHR( m_device, m_swapchain, nullptr );
	m_swapchain = VK_NULL_HANDLE;

	vkDestroySurfaceKHR( m_instance, m_surface, nullptr );
	m_surface = VK_NULL_HANDLE;
}


GfxImage& GfxSwapchain::AcquireNextImage( GfxCmdBfr &cmd, GfxFence &fence )
{
	Com_AssertMsg_Fatal( m_currentIndex == kUnsetIndex, "Acquired a swapchain image without presenting the previous one.\n" );

	BEGIN_INFO_STRUCT( VkAcquireNextImageInfoKHR, acquireInfo )
		.swapchain = m_swapchain,
		.timeout = kFenceWaitTimeout,
		.fence = fence,
		.deviceMask = 1, // docs aren't great on deviceMask but it has to do with using multiple physical devices. I'm guessing 1 is fine for a single-device setup.
	END_INFO_STRUCT;

	VK_CALL( vkAcquireNextImage2KHR( m_device, &acquireInfo, &m_currentIndex ) );
	Com_Assert_Fatal( m_currentIndex < ARRAY_COUNT( m_swapchainImages ) );

	GfxImage &nextImage = m_swapchainImages[m_currentIndex];
	cmd.TransitionImgForUsage( nextImage, GfxImgUsage::TransferDst );

	return nextImage;
}


void GfxSwapchain::PrepForPresent( GfxCmdBfr &cmd )
{
	Com_AssertMsg_Fatal( m_currentIndex != kUnsetIndex, "Attempted to prep for present without first acquiring a swapchain image.\n" );
	GfxImage &nextImage = m_swapchainImages[m_currentIndex];
	cmd.TransitionImgForUsage( nextImage, GfxImgUsage::PresentSrc );
}


void GfxSwapchain::Present()
{
	Com_AssertMsg_Fatal( m_currentIndex != kUnsetIndex, "Attempted to present without first acquiring a swapchain image.\n" );

	VkResult presentResult = VK_RESULT_MAX_ENUM;
	BEGIN_INFO_STRUCT( VkPresentInfoKHR, presentInfo )
		.swapchainCount = 1,
		.pSwapchains = &m_swapchain,
		.pImageIndices = &m_currentIndex,
	END_INFO_STRUCT;

	VK_CALL( vkQueuePresentKHR( m_submitQueue, &presentInfo ) );

	m_currentIndex = kUnsetIndex;
}
