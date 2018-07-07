#include "Swapchain.h"

Swapchain::Swapchain() : mSwapchain(VK_NULL_HANDLE)
{}

Swapchain::~Swapchain()
{
}

void Swapchain::initialize(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, QueueFamilyIndices selectedIndices, VkExtent2D extent)
{
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

	VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(physicalDevice, surface);
	VkPresentModeKHR presentMode = choosePresentMode(physicalDevice, surface);
	uint32_t imageCount = 2;

	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.pNext = nullptr;
	swapchainCreateInfo.flags = 0;
	swapchainCreateInfo.surface = surface;
	swapchainCreateInfo.minImageCount = imageCount;		//Double buffered
	swapchainCreateInfo.imageFormat = surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent = extent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;	//rendering directly to these images

																			//if the queue families are different, they need to be able to share the images
																			//otherwise use exclusive mode
	if (selectedIndices.graphicsFamily != selectedIndices.presentFamily)
	{
		uint32_t queueFamilyIndices[] = { (uint32_t)selectedIndices.graphicsFamily,
			(uint32_t)selectedIndices.presentFamily };
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCreateInfo.queueFamilyIndexCount = 2;
		swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCreateInfo.queueFamilyIndexCount = 0;
		swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	}

	swapchainCreateInfo.preTransform = capabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = presentMode;
	swapchainCreateInfo.clipped = VK_TRUE;	//we don't care about the colors of obscured pixels (i.e. hidden by another window)
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;	//used for recreating the swapchain

	if (vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &mSwapchain) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create swapchain!");
	}

	//get the swapchain images
	vkGetSwapchainImagesKHR(device, mSwapchain, &imageCount, nullptr);
	mImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, mSwapchain, &imageCount, mImages.data());

	//for use later
	mImageFormat = surfaceFormat.format;
	mExtent = extent;

	initialized = true;
}

void Swapchain::cleanup(VkDevice device)
{
	for (auto imageView : mImageViews) {
		vkDestroyImageView(device, imageView, nullptr);
	}

	//Images automatically cleaned up

	vkDestroySwapchainKHR(device, mSwapchain, nullptr);
}

VkSurfaceFormatKHR Swapchain::chooseSurfaceFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	//std::cout << "choosing swapchain format" << std::endl;
	//Choose a swapchain format
	uint32_t formatCount;
	std::vector<VkSurfaceFormatKHR> availableFormats;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

	if (formatCount != 0) {
		availableFormats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, availableFormats.data());
	}
	else {
		throw std::runtime_error("no available formats for swapchain!");
	}

	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {

		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	//otherwise just use the first
	return availableFormats[0];
}

VkPresentModeKHR Swapchain::choosePresentMode(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	//get all of the available modes
	uint32_t presentModeCount = 0;
	std::vector<VkPresentModeKHR> availablePresentModes;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		availablePresentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, availablePresentModes.data());
	}
	else {
		throw std::runtime_error("No available swapchain present modes!");
	}

	//select the best one
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
		else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			bestMode = availablePresentMode;
		}
	}

	return bestMode;
}

void Swapchain::createImageViews(VkDevice device)
{
	std::cout << "Creating swapchain image views" << std::endl;

	mImageViews.resize(mImages.size());

	for (size_t i = 0; i < mImages.size(); i++) {
		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = mImages[i];
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = mImageFormat;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &imageViewCreateInfo, nullptr, &mImageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create swapchain image views!");
		}

	}
}

std::vector<VkFramebuffer> Swapchain::createFramebuffers(VkDevice device, VkRenderPass renderPass)
{
	std::vector<VkFramebuffer> framebuffers;
	framebuffers.resize(mImageViews.size());

	for (size_t i = 0; i < mImageViews.size(); i++) {
		VkImageView attachments[] = {
			mImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = mExtent.width;
		framebufferInfo.height = mExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
	return std::vector<VkFramebuffer>();
}

VkSwapchainKHR Swapchain::getVkSwapchain() const
{
	return mSwapchain;
}

std::vector<VkImageView> Swapchain::getImageViews()
{
	return mImageViews;
}

VkFormat Swapchain::getImageFormat() const
{
	return mImageFormat;
}

VkExtent2D Swapchain::getExtent()
{
	return mExtent;
}
