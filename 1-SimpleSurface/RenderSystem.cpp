#include "RenderSystem.h"
#include <algorithm>


void RenderSystem::init(GLFWwindow * window)
{
	printExtensions();
	createInstance();
	if (enableValidationLayers) {
		createDebugCallback();
	}
	createSurface(window);
	createDevice();
	createSwapchain();
	createSwapchainImageViews();
}

void RenderSystem::update()
{
}

void RenderSystem::shutdown()
{
	std::cout << "Shutting down render system" << std::endl;

	for (auto imageView : mSwapchainImageViews){
		vkDestroyImageView(mDevice, imageView, nullptr);
	}

	vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
	vkDestroyDevice(mDevice, nullptr);

	if (enableValidationLayers) {
		destroyDebugCallback();
	}

	vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
	vkDestroyInstance(mInstance, nullptr);
}

void RenderSystem::createInstance()
{
	std::cout << "creating vulkan instance.." << std::endl;

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "1-SimpleSurface";
	appInfo.applicationVersion = 1;
	appInfo.pEngineName = "uwb-vk";
	appInfo.engineVersion = 1;
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);


	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	
	//instance extensions
	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();
	
	//validation layers
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateInstance(&createInfo, nullptr, &mInstance) != VK_SUCCESS) {
		std::cerr << "Failed to create instance!" << std::endl;
	}
}

void RenderSystem::createDevice()
{
	//get a list of physical devices
	uint32_t physicalDeviceCount;
	vkEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, nullptr);

	std::vector<VkPhysicalDevice> availableDevices(physicalDeviceCount);
	vkEnumeratePhysicalDevices(mInstance, &physicalDeviceCount, availableDevices.data());

	//select one of the devices
	std::cout << "Devices Found:" << physicalDeviceCount << std::endl;
	for (const auto& physicalDevice : availableDevices) {
		printPhysicalDeviceDetails(physicalDevice);
	}

	//just pick the first physical device for now
	//we'll need to do device suitability checks later
	mPhysicalDevice = availableDevices[0];

	mSelectedIndices = findQueueFamilies(mPhysicalDevice);

	//create a queue for both graphics and presentation
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	//create a graphics queue
	VkDeviceQueueCreateInfo graphicsQueueCreateInfo = {};
	graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	graphicsQueueCreateInfo.queueFamilyIndex = mSelectedIndices.graphicsFamily;
	graphicsQueueCreateInfo.queueCount = 1;
	float queuePriority = 1.0f;
	graphicsQueueCreateInfo.pQueuePriorities = &queuePriority;
	queueCreateInfos.push_back(graphicsQueueCreateInfo);

	//create a separate present queue if necessary (different family)
	if (mSelectedIndices.graphicsFamily != mSelectedIndices.presentFamily) {
		VkDeviceQueueCreateInfo presentQueueCreateInfo = {};
		presentQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		presentQueueCreateInfo.queueFamilyIndex = mSelectedIndices.presentFamily;
		presentQueueCreateInfo.queueCount = 1;
		float queuePriority = 1.0f;
		presentQueueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(presentQueueCreateInfo);
	}

	//specify the features of the device we'll be using
	VkPhysicalDeviceFeatures deviceFeatures = {};

	//main createInfo struct
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
	
	//extensions we want this device to use
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

	//create the device
	if (vkCreateDevice(mPhysicalDevice, &deviceCreateInfo, nullptr, &mDevice) != VK_SUCCESS) {
		std::cerr << "Device Creation Failed!" << std::endl;
	}

	//get the queue handles
	vkGetDeviceQueue(mDevice, mSelectedIndices.graphicsFamily, 0, &mGraphicsQueue);
	vkGetDeviceQueue(mDevice, mSelectedIndices.presentFamily, 0, &mPresentQueue);
}

QueueFamilyIndices RenderSystem::findQueueFamilies(VkPhysicalDevice physicalDevice)
{
	QueueFamilyIndices indices;

	//get the queue families for this device
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	std::cout << "--------------------------------" << std::endl;
	std::cout << queueFamilyCount << " queue families found" << std::endl;

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		std::cout << "Family " << i << ":" << std::endl;
		std::cout << queueFamily.queueCount << " queues" << std::endl;
		std::cout << "flags:" << std::endl;
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			std::cout << "-graphics" << std::endl;
			indices.graphicsFamily = i;
		}
		if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
			std::cout << "-compute" << std::endl;
		}
		if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) {
			std::cout << "-transfer" << std::endl;
		}
		if (queueFamily.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
			std::cout << "-sparse-binding" << std::endl;
		}
		if (queueFamily.queueFlags & VK_QUEUE_PROTECTED_BIT) {
			std::cout << "-protected" << std::endl;
		}

		//figure out if this queue family supports presentation
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(mPhysicalDevice, i, mSurface, &presentSupport);
		if (queueFamily.queueCount > 0 && presentSupport) {
			indices.presentFamily = i;
		}

		//if we have a present queue for both graphics and presentation we're done
		if (indices.isComplete())
			break;

		i++;
	}
	std::cout << "--------------------------------" << std::endl;
	return indices;
}

void RenderSystem::createSurface(GLFWwindow* window)
{
	if(glfwCreateWindowSurface(mInstance, window, nullptr, &mSurface) != VK_SUCCESS){
		throw std::runtime_error("Failed to create a window surface!");
	}
}

void RenderSystem::createSwapchain()
{
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, mSurface, &capabilities);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapchainSurfaceFormat();
	VkPresentModeKHR presentMode = chooseSwapchainPresentMode();
	VkExtent2D extent = chooseSwapchainExtent(capabilities);
	uint32_t imageCount = 2;

	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.pNext = nullptr;
	swapchainCreateInfo.flags = 0;
	swapchainCreateInfo.surface = mSurface;
	swapchainCreateInfo.minImageCount = imageCount;		//Double buffered
	swapchainCreateInfo.imageFormat = surfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent = extent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;	//rendering directly to these images

	//if the queue families are different, they need to be able to share the images
	//otherwise use exclusive mode
	if (mSelectedIndices.graphicsFamily != mSelectedIndices.presentFamily)
	{
		uint32_t queueFamilyIndices[] = { (uint32_t)mSelectedIndices.graphicsFamily,
										 (uint32_t)mSelectedIndices.presentFamily };
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

	if (vkCreateSwapchainKHR(mDevice, &swapchainCreateInfo, nullptr, &mSwapchain) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create swapchain!");
	}

	//get the swapchain images
	vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, nullptr);
	mSwapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, mSwapchainImages.data());

	//for use later
	mSwapchainImageFormat = surfaceFormat.format;
	mSwapchainExtent = extent;

}

VkSurfaceFormatKHR RenderSystem::chooseSwapchainSurfaceFormat()
{
	std::cout << "choosing swapchain format" << std::endl;
	//Choose a swapchain format
	uint32_t formatCount;
	std::vector<VkSurfaceFormatKHR> availableFormats;
	vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &formatCount, nullptr);

	if (formatCount != 0) {
		availableFormats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &formatCount, availableFormats.data());
	} else {
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

VkPresentModeKHR RenderSystem::chooseSwapchainPresentMode()
{
	//get all of the available modes
	uint32_t presentModeCount = 0;
	std::vector<VkPresentModeKHR> availablePresentModes;
	vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mSurface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		availablePresentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mSurface, &presentModeCount, availablePresentModes.data());
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

VkExtent2D RenderSystem::chooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		VkExtent2D actualExtent = { 800, 600 };
		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

void RenderSystem::createSwapchainImageViews()
{
	mSwapchainImageViews.resize(mSwapchainImages.size());

	for (size_t i = 0; i < mSwapchainImages.size(); i++) {
		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = mSwapchainImages[i];
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = mSwapchainImageFormat;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(mDevice, &imageViewCreateInfo, nullptr, &mSwapchainImageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create swapchain image views!");
		}

	}
}

void RenderSystem::createGraphicsPipeline()
{
	auto vertShaderCode = readShaderFile("shaders/vert.spv");
	auto fragShaderCode = readShaderFile("shaders/frag.spv");







	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.pNext = nullptr;

}

std::vector<const char*> RenderSystem::getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	return extensions;
}

void RenderSystem::createDebugCallback()
{
	//Get a function pointer to call the function
	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT =
		reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>
		(vkGetInstanceProcAddr(mInstance, "vkCreateDebugReportCallbackEXT"));

	if (vkCreateDebugReportCallbackEXT == nullptr) {
		throw std::runtime_error("Unable to get function pointer for vkCreateDebugReportCallbackEXT!");
	}

	VkDebugReportCallbackCreateInfoEXT callbackCreateInfo = {};
	callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	callbackCreateInfo.pNext = nullptr;
	callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
								VK_DEBUG_REPORT_WARNING_BIT_EXT |
								VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	callbackCreateInfo.pfnCallback = debugCallback;
	callbackCreateInfo.pUserData = nullptr;

	if (vkCreateDebugReportCallbackEXT(mInstance, &callbackCreateInfo, nullptr, &callback) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create debug callback!");
	}
}

void RenderSystem::destroyDebugCallback()
{
	//Get a function pointer to call the function
	PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT =
		reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>
		(vkGetInstanceProcAddr(mInstance, "vkDestroyDebugReportCallbackEXT"));

	vkDestroyDebugReportCallbackEXT(mInstance, callback, nullptr);
}

void RenderSystem::printExtensions()
{
	std::cout << "--------------------------------" << std::endl;
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::cout << extensionCount << " vulkan extensions supported: " << std::endl << std::endl;

	std::vector<VkExtensionProperties> extensionProperties;
	extensionProperties.resize(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionProperties.data());

	
	for (auto properties : extensionProperties) {
		std::cout << properties.extensionName << std::endl;
	}
	std::cout << "--------------------------------" << std::endl;
}

void RenderSystem::printPhysicalDeviceDetails(VkPhysicalDevice physicalDevice)
{
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);

	std::cout << properties.deviceName << std::endl;
	std::cout << "API Version: " << properties.apiVersion << std::endl;
}
