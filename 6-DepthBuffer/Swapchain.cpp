#include "Swapchain.h"

Swapchain::Swapchain(std::shared_ptr<VulkanContext> context, std::shared_ptr<ImageManager> imageManager) :
	mSwapchain(VK_NULL_HANDLE),
	mImageFormat(VK_FORMAT_UNDEFINED),
	mExtent{ 0, 0 },
	mContext(context),
	mImageManager(imageManager)
{}

Swapchain::~Swapchain() {}

void Swapchain::initialize(VkSurfaceKHR surface, uint32_t imageCount) 
{
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mContext->physicalDevice, surface, &capabilities);

	VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(surface);
	VkPresentModeKHR presentMode = choosePresentMode(surface);
	VkExtent2D extent = chooseExtent(capabilities);

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
	if (mContext->selectedIndices.graphicsFamily != mContext->selectedIndices.presentFamily)
	{
		uint32_t queueFamilyIndices[] = { (uint32_t)mContext->selectedIndices.graphicsFamily,
			(uint32_t)mContext->selectedIndices.presentFamily };
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

	if (vkCreateSwapchainKHR(mContext->device, &swapchainCreateInfo, nullptr, &mSwapchain) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create swapchain!");
	}

	//get the swapchain images
	vkGetSwapchainImagesKHR(mContext->device, mSwapchain, &imageCount, nullptr);
	mImages.resize(imageCount);
	vkGetSwapchainImagesKHR(mContext->device, mSwapchain, &imageCount, mImages.data());

	//for use later
	mImageFormat = surfaceFormat.format;
	mExtent = extent;

	//createImageViews
	createImageViews();
}

void Swapchain::cleanup()
{
	for (auto imageView : mImageViews) {
		vkDestroyImageView(mContext->device, imageView, nullptr);
	}

	//Images automatically cleaned up

	vkDestroySwapchainKHR(mContext->device, mSwapchain, nullptr);
}

VkSurfaceFormatKHR Swapchain::chooseSurfaceFormat(VkSurfaceKHR surface)
{
	//std::cout << "choosing swapchain format" << std::endl;
	//Choose a swapchain format
	uint32_t formatCount;
	std::vector<VkSurfaceFormatKHR> availableFormats;
	vkGetPhysicalDeviceSurfaceFormatsKHR(mContext->physicalDevice, surface, &formatCount, nullptr);

	if (formatCount != 0) {
		availableFormats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(mContext->physicalDevice, surface, &formatCount, availableFormats.data());
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

VkPresentModeKHR Swapchain::choosePresentMode(VkSurfaceKHR surface)
{
	//get all of the available modes
	uint32_t presentModeCount = 0;
	std::vector<VkPresentModeKHR> availablePresentModes;
	vkGetPhysicalDeviceSurfacePresentModesKHR(mContext->physicalDevice, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		availablePresentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(mContext->physicalDevice, surface, &presentModeCount, availablePresentModes.data());
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

VkExtent2D Swapchain::chooseExtent(const VkSurfaceCapabilitiesKHR & capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(mContext->window, &width, &height);
		std::cout << "window size: (" << width << ", " << height << ")" << std::endl;

		VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

void Swapchain::createImageViews()
{
	std::cout << "Creating swapchain image views" << std::endl;
	mImageViews.resize(mImages.size());
	for (size_t i = 0; i < mImages.size(); i++) {
		mImageViews[i] = mImageManager->createImageView(mImages[i], mImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

std::vector<VkFramebuffer> Swapchain::createFramebuffers(VkRenderPass renderPass)
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

		if (vkCreateFramebuffer(mContext->device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}

	return framebuffers;
}

VkSwapchainKHR Swapchain::getVkSwapchain() const
{
	return mSwapchain;
}

std::vector<VkImageView> Swapchain::getImageViews() const
{
	return mImageViews;
}

VkFormat Swapchain::getImageFormat() const
{
	return mImageFormat;
}

VkExtent2D Swapchain::getExtent() const
{
	return mExtent;
}

uint32_t Swapchain::size() const
{
	return static_cast<uint32_t>(mImages.size());
}
