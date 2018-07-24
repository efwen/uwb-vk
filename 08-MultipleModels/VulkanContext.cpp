#include "VulkanContext.h"

VulkanContext::VulkanContext() : 
	window(nullptr),
	mCallback(nullptr),
	mInstance(VK_NULL_HANDLE),
	device(VK_NULL_HANDLE),
	graphicsQueue(VK_NULL_HANDLE),
	presentQueue(VK_NULL_HANDLE),
	surface(VK_NULL_HANDLE)
{}

void VulkanContext::initialize(GLFWwindow *window)
{
	std::cout << "Initializing Vulkan Context" << std::endl;
	createInstance();
	
	if (enableValidationLayers) {
		setupDebugCallback();
	}

	this->window = window;
	createSurface(mInstance, window);
	createDevice(mInstance);
}

void VulkanContext::cleanup()
{
	std::cout << "Destroying Vulkan Context" << std::endl;

	vkDestroyDevice(device, nullptr);
	vkDestroySurfaceKHR(mInstance, surface, nullptr);

	if (enableValidationLayers) {
		DestroyDebugReportCallbackEXT(mInstance, mCallback, nullptr);
	}

	vkDestroyInstance(mInstance, nullptr);
}

uint32_t VulkanContext::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("Failed to find a suitable memory type!");
}

void VulkanContext::createInstance()
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "8-MultipleModels";
	appInfo.applicationVersion = 1;
	appInfo.pEngineName = "uwb-vk";
	appInfo.engineVersion = 1;
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);


	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	//instance extensionsv
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

void VulkanContext::createDevice(VkInstance instance)
{
	std::cout << "Creating Device" << std::endl;

	//get a list of physical devices
	uint32_t physicalDeviceCount;
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

	std::vector<VkPhysicalDevice> availableDevices(physicalDeviceCount);
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, availableDevices.data());

	//select one of the devices
	std::cout << "Devices Found:" << physicalDeviceCount << std::endl;
	for (const auto& device : availableDevices) {
		printPhysicalDeviceDetails(device);
	}

	//just pick the first physical device for now
	//we'll need to do device suitability checks later
	physicalDevice = availableDevices[0];

	selectedIndices = findQueueFamilies(physicalDevice, surface);

	//create a queue for both graphics and presentation
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	//create a graphics queue
	VkDeviceQueueCreateInfo graphicsQueueCreateInfo = {};
	graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	graphicsQueueCreateInfo.queueFamilyIndex = selectedIndices.graphicsFamily;
	graphicsQueueCreateInfo.queueCount = 1;
	float queuePriority = 1.0f;
	graphicsQueueCreateInfo.pQueuePriorities = &queuePriority;
	queueCreateInfos.push_back(graphicsQueueCreateInfo);

	//create a separate present queue if necessary (different family)
	if (selectedIndices.graphicsFamily != selectedIndices.presentFamily) {
		VkDeviceQueueCreateInfo presentQueueCreateInfo = {};
		presentQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		presentQueueCreateInfo.queueFamilyIndex = selectedIndices.presentFamily;
		presentQueueCreateInfo.queueCount = 1;
		float queuePriority = 1.0f;
		presentQueueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(presentQueueCreateInfo);
	}

	//specify the features of the device we'll be using
	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

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
	if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS) {
		std::runtime_error("Device Creation Failed");
	}

	//get the queue handles
	vkGetDeviceQueue(device, selectedIndices.graphicsFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(device, selectedIndices.presentFamily, 0, &presentQueue);
}

void VulkanContext::createSurface(VkInstance instance, GLFWwindow* window)
{
	std::cout << "Creating Surface" << std::endl;

	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a window surface!");
	}
}

void VulkanContext::printPhysicalDeviceDetails(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(device, &properties);

	std::cout << properties.deviceName << std::endl;
	std::cout << "API Version: " << properties.apiVersion << std::endl;
}

std::vector<const char*> VulkanContext::getRequiredExtensions()
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

void VulkanContext::setupDebugCallback()
{
	if (!enableValidationLayers) return;

	std::cout << "Setting up debug callback" << std::endl;

	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createInfo.pfnCallback = debugCallback;

	if (CreateDebugReportCallbackEXT(mInstance, &createInfo, nullptr, &mCallback) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug callback!");
	}
}
