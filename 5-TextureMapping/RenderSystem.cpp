#include "RenderSystem.h"
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


void RenderSystem::init(GLFWwindow * window)
{
	printExtensions();
	createInstance();
	if (enableValidationLayers) {
		setupDebugCallback();
	}

	mWindow = window;
	createSurface(window);
	createDevice();
	createSwapchain();
	createSwapchainImageViews();
	createRenderPass();
	createDescriptorSetLayout();
	createGraphicsPipeline();

	createFramebuffers();
	createCommandPool();

	createTexture();
	createVertexBuffer();
	createIndexBuffer();
	createUniformBuffer();
	createDescriptorPool();
	createDescriptorSet();

	createCommandBuffers();
	createSyncObjects();
}

void RenderSystem::recreateSwapchain()
{
	vkDeviceWaitIdle(mDevice);

	cleanupSwapchain();

	createSwapchain();
	createSwapchainImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createFramebuffers();
	createCommandBuffers();
}

void RenderSystem::cleanupSwapchain()
{
	for (auto framebuffer : mSwapchainFramebuffers) {
		vkDestroyFramebuffer(mDevice, framebuffer, nullptr);
	}

	vkFreeCommandBuffers(mDevice, mCommandPool, static_cast<uint32_t>(mCommandBuffers.size()), mCommandBuffers.data());

	vkDestroyPipeline(mDevice, mPipeline, nullptr);
	vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
	vkDestroyRenderPass(mDevice, mRenderPass, nullptr);

	for (auto imageView : mSwapchainImageViews) {
		vkDestroyImageView(mDevice, imageView, nullptr);
	}

	vkDestroySwapchainKHR(mDevice, mSwapchain, nullptr);
}

void RenderSystem::shutdown()
{
	std::cout << "Shutting down render system" << std::endl;
	
	//make sure the queue is idle before destroying its sync objects
	vkQueueWaitIdle(mPresentQueue);

	cleanupSwapchain();

	vkDestroyDescriptorPool(mDevice, mDescriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayout, nullptr);
	vkDestroyBuffer(mDevice, mUniformBuffer, nullptr);
	vkFreeMemory(mDevice, mUniformBufferMemory, nullptr);

	vkDestroyBuffer(mDevice, mIndexBuffer, nullptr);
	vkFreeMemory(mDevice, mIndexBufferMemory, nullptr);

	vkDestroyBuffer(mDevice, mVertexBuffer, nullptr);
	vkFreeMemory(mDevice, mVertexBufferMemory, nullptr);

	for (size_t i = 0; i < MAX_CONCURRENT_FRAMES; i++) {
		vkDestroySemaphore(mDevice, mRenderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(mDevice, mImageAvailableSemaphores[i], nullptr);
		vkDestroyFence(mDevice, mFrameFences[i], nullptr);
	}

	vkDestroyCommandPool(mDevice, mCommandPool, nullptr);

	vkDestroyDevice(mDevice, nullptr);
	if (enableValidationLayers) {
		//DestroyDebugReportCallbackEXT(mInstance, mCallback, nullptr);
	}

	vkDestroySurfaceKHR(mInstance, mSurface, nullptr);
	vkDestroyInstance(mInstance, nullptr);
}

void RenderSystem::drawFrame()
{
	updateUniformBuffer();

	vkWaitForFences(mDevice, 1, &mFrameFences[mCurrentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
	vkResetFences(mDevice, 1, &mFrameFences[mCurrentFrame]);

	//Get the next available image
	uint32_t imageIndex;
	vkAcquireNextImageKHR(mDevice, mSwapchain, std::numeric_limits<uint64_t>::max(), mImageAvailableSemaphores[mCurrentFrame], VK_NULL_HANDLE, &imageIndex);


	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	//Semaphores for waiting to submit
	VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[mCurrentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &mCommandBuffers[imageIndex];

	//Semaphore for when the submission is done
	VkSemaphore signalSemaphores[] = { mRenderFinishedSemaphores[mCurrentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mFrameFences[mCurrentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	//Present the image
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { mSwapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;

	presentInfo.pImageIndices = &imageIndex;

	VkResult result = vkQueuePresentKHR(mPresentQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapchain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("Failed to acquire swapchain image!");
	}

	mCurrentFrame = (mCurrentFrame + 1) % MAX_CONCURRENT_FRAMES;
}



void RenderSystem::createInstance()
{
	std::cout << "creating vulkan instance.." << std::endl;

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "3-MVP";
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

void RenderSystem::createDevice()
{
	std::cout << "Creating Device" << std::endl;

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

	mSelectedIndices = findQueueFamilies(mPhysicalDevice, mSurface);

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

void RenderSystem::createSurface(GLFWwindow* window)
{
	std::cout << "Creating Surface" << std::endl;

	if(glfwCreateWindowSurface(mInstance, window, nullptr, &mSurface) != VK_SUCCESS){
		throw std::runtime_error("Failed to create a window surface!");
	}
}

void RenderSystem::createSwapchain()
{
	std::cout << "Creating Swapchain" << std::endl;

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
		int width, height;
		glfwGetFramebufferSize(mWindow, &width, &height);
		std::cout << "window size: (" << width << ", " << height << ")" << std::endl;

		VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

void RenderSystem::createSwapchainImageViews()
{
	std::cout << "Creating swapchain image views" << std::endl;

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
	std::cout << "Creating Graphics pipeline" << std::endl;

	//Set up the programmable stages of the pipeline
	auto vertShaderCode = readShaderFile("shaders/square_vert.spv");
	auto fragShaderCode = readShaderFile("shaders/square_frag.spv");

	VkShaderModule vertShaderModule;
	VkShaderModule fragShaderModule;
	vertShaderModule = createShaderModule(vertShaderCode);
	fragShaderModule = createShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	//
	//Fixed Function Stages
	//


	//Vertex Input:
	//Determine the format of vertex data passed into the vertex shader
	//no input for now
	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	//What kind of geometry primitives will be drawn from the vertices
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	//ViewportState
	//Where on the surface this pipeline will render to

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)mSwapchainExtent.width;
	viewport.height = (float)mSwapchainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = mSwapchainExtent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;



	//Rasterizer
	//Turns geometry into fragments
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;


	//Multisampling - won't be using this, but useful for things like anti-aliasing
	//requires enabling a gpu feature
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	//Depth & Stencil testing
	//not using for now, so just pass in nullptr


	//Color blending
	//For now, no blending
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;


	//Dynamic State
	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;

	//Pipeline Layout
	//This is where you pass in uniform values
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1; 
	pipelineLayoutInfo.pSetLayouts = &mDescriptorSetLayout; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

	if (vkCreatePipelineLayout(mDevice, &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}


	//Finally create the pipeline
	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.pNext = nullptr;
	pipelineCreateInfo.stageCount = 2;
	pipelineCreateInfo.pStages = shaderStages;
	pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pRasterizationState = &rasterizer;
	pipelineCreateInfo.pMultisampleState = &multisampling;
	pipelineCreateInfo.pDepthStencilState = nullptr;
	pipelineCreateInfo.pColorBlendState = &colorBlending;
	pipelineCreateInfo.pDynamicState = nullptr;

	pipelineCreateInfo.layout = mPipelineLayout;
	pipelineCreateInfo.renderPass = mRenderPass;
	pipelineCreateInfo.subpass = 0;

	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(mDevice, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mPipeline) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create a graphics pipeline!");
	}

	//No longer need the shader modules, destroy them
	vkDestroyShaderModule(mDevice, vertShaderModule, nullptr);
	vkDestroyShaderModule(mDevice, fragShaderModule, nullptr);
}

VkShaderModule RenderSystem::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(mDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create shader module!");
	}

	return shaderModule;
}

void RenderSystem::createRenderPass()
{
	std::cout << "Creating render pass" << std::endl;

	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = mSwapchainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;	//clear values to constant at start
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;	//rendered contents stored in memory for later
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	if (vkCreateRenderPass(mDevice, &renderPassInfo, nullptr, &mRenderPass) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create render pass!");
	}
}

void RenderSystem::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr;		//only relevant for image sampling descriptors

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &uboLayoutBinding;

	if (vkCreateDescriptorSetLayout(mDevice, &layoutInfo, nullptr, &mDescriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor set layout!");
	}
}

void RenderSystem::createDescriptorPool()
{
	VkDescriptorPoolSize poolSize = {};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	poolInfo.maxSets = 1;

	if (vkCreateDescriptorPool(mDevice, &poolInfo, nullptr, &mDescriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor pool!");
	}
}

void RenderSystem::createDescriptorSet()
{
	VkDescriptorSetLayout layouts[] = { mDescriptorSetLayout };
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = mDescriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	if (vkAllocateDescriptorSets(mDevice, &allocInfo, &mDescriptorSet) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate descriptor set!");
	}

	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = mUniformBuffer;
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(UniformBufferObject);

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = mDescriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &bufferInfo;
	descriptorWrite.pImageInfo = nullptr;
	descriptorWrite.pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(mDevice, 1, &descriptorWrite, 0, nullptr);
}

void RenderSystem::createFramebuffers()
{
	std::cout << "Creating Framebuffers" << std::endl;

	mSwapchainFramebuffers.resize(mSwapchainImageViews.size());

	for (size_t i = 0; i < mSwapchainImageViews.size(); i++) {
		VkImageView attachments[] = {
			mSwapchainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = mRenderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = mSwapchainExtent.width;
		framebufferInfo.height = mSwapchainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(mDevice, &framebufferInfo, nullptr, &mSwapchainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void RenderSystem::createCommandPool()
{
	std::cout << "Creating command pool" << std::endl;

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = mSelectedIndices.graphicsFamily;
	poolInfo.flags = 0; // Optional

	if (vkCreateCommandPool(mDevice, &poolInfo, nullptr, &mCommandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}

void RenderSystem::createCommandBuffers()
{
	std::cout << "Creating command buffers" << std::endl;

	mCommandBuffers.resize(mSwapchainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = mCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)mCommandBuffers.size();

	if (vkAllocateCommandBuffers(mDevice, &allocInfo, mCommandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	//record into the command buffers
	for (size_t i = 0; i < mCommandBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		if (vkBeginCommandBuffer(mCommandBuffers[i], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = mRenderPass;
		renderPassInfo.framebuffer = mSwapchainFramebuffers[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = mSwapchainExtent;
		
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &mClearColor;

		vkCmdBeginRenderPass(mCommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline);


		VkBuffer vertexBuffers = { mVertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(mCommandBuffers[i], 0, 1, &vertexBuffers, offsets);

		vkCmdBindIndexBuffer(mCommandBuffers[i], mIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

		vkCmdBindDescriptorSets(mCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &mDescriptorSet, 0, nullptr);
		vkCmdDrawIndexed(mCommandBuffers[i], static_cast<uint32_t>(squareIndices.size()), 1, 0, 0, 0);

		vkCmdEndRenderPass(mCommandBuffers[i]);

		if (vkEndCommandBuffer(mCommandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

void RenderSystem::createSyncObjects()
{
	std::cout << "Creating semaphores" << std::endl;
	mImageAvailableSemaphores.resize(MAX_CONCURRENT_FRAMES);
	mRenderFinishedSemaphores.resize(MAX_CONCURRENT_FRAMES);
	mFrameFences.resize(MAX_CONCURRENT_FRAMES);
	
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_CONCURRENT_FRAMES; i++)
	{
		if(vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mImageAvailableSemaphores[i]) != VK_SUCCESS ||
		   vkCreateSemaphore(mDevice, &semaphoreInfo, nullptr, &mRenderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(mDevice, &fenceInfo, nullptr, &mFrameFences[i]) != VK_SUCCESS) 
		{
			throw std::runtime_error("Failed to create frame sync objects!");
		}
	}
}

/*
Utility Functions
*/

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

VkCommandBuffer RenderSystem::beginSingleCmdBuffer()
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = mCommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(mDevice, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void RenderSystem::endSingleCmdBuffer(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(mGraphicsQueue);

	vkFreeCommandBuffers(mDevice, mCommandPool, 1, &commandBuffer);
}

uint32_t RenderSystem::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("Failed to find a suitable memory type!");
}

void RenderSystem::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer & buffer, VkDeviceMemory & bufferMemory)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(mDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(mDevice, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate buffer memory!");
	}

	vkBindBufferMemory(mDevice, buffer, bufferMemory, 0);


}

void RenderSystem::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkCommandBuffer commandBuffer = beginSingleCmdBuffer();

	std::cout << "TransitionImageLayout();" << std::endl;

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;	//used for transitioning family ownership
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;									//the image to transition
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = 0;	//todo
	barrier.dstAccessMask = 0;  //todo

	vkCmdPipelineBarrier(
		commandBuffer,
		0, 0,	//pipeline stages to use before the barrier (todo)
		0,	//dependency flags
		0, nullptr,	//memory barriers	
		0, nullptr,	//buffer memory barriers
		1, &barrier //image barriers
	);


	endSingleCmdBuffer(commandBuffer);
}

void RenderSystem::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = beginSingleCmdBuffer();

	std::cout << "Copy Buffer to image" << std::endl;

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width, height, 1
	};

	vkCmdCopyBufferToImage(
		commandBuffer,
		buffer,									//source buffer
		image,									//destination image
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,	//layout the dest image is using
		1,										//region count
		&region									//array address
	);

	endSingleCmdBuffer(commandBuffer);
}

void RenderSystem::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = beginSingleCmdBuffer();

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = 0; // Optional
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	endSingleCmdBuffer(commandBuffer);
}

void RenderSystem::createVertexBuffer()
{
	std::cout << "Creating Verrtex Buffer..." << std::endl;

	VkDeviceSize bufferSize = sizeof(squareVertices[0]) * squareVertices.size();

	//create a staging buffer and load the data into it
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(bufferSize, 
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
				stagingBuffer, 
				stagingBufferMemory);

	//Copy the data into the staging buffer
	void* data;
	vkMapMemory(mDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, squareVertices.data(), (size_t)bufferSize);
	vkUnmapMemory(mDevice, stagingBufferMemory);

	//create the vertex buffer
	createBuffer(bufferSize, 
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
				mVertexBuffer, 
				mVertexBufferMemory);

	//move the data from the staging buffer to the vertex buffer
	copyBuffer(stagingBuffer, mVertexBuffer, bufferSize);

	vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
	vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
}

void RenderSystem::createIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(squareIndices[0]) * squareIndices.size();
	
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(bufferSize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				stagingBuffer,
				stagingBufferMemory);

	//Copy the data into the staging buffer
	void* data;
	vkMapMemory(mDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, squareIndices.data(), (size_t)bufferSize);
	vkUnmapMemory(mDevice, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		mIndexBuffer,
		mIndexBufferMemory);

	//move the data from the staging buffer to the vertex buffer
	copyBuffer(stagingBuffer, mIndexBuffer, bufferSize);

	vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
	vkFreeMemory(mDevice, stagingBufferMemory, nullptr);
}

void RenderSystem::createUniformBuffer()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);
	createBuffer(bufferSize,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		mUniformBuffer,
		mUniformBufferMemory);

	//no need for a stahging buffer
}

void RenderSystem::updateUniformBuffer()
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	UniformBufferObject ubo = {};
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), mSwapchainExtent.width / (float)mSwapchainExtent.height, 0.1f, 10.0f);
	ubo.proj[1][1] *= -1;

	void* data;
	vkMapMemory(mDevice, mUniformBufferMemory, 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(mDevice, mUniformBufferMemory);
}

void RenderSystem::createTexture()
{
	std::cout << "Creating texture..." << std::endl;
	int width, height, channels;
	stbi_uc* pixels = stbi_load("textures/texture.jpg", &width, &height, &channels, STBI_rgb_alpha);

	if (!pixels) {
		throw std::runtime_error("Failed to load texture image");
	}

	mTexture = new Texture(this, mDevice);
	mTexture->load(pixels, width, height, channels);

	stbi_image_free(pixels);

}

void RenderSystem::setClearColor(VkClearValue clearColor)
{
	vkDeviceWaitIdle(mDevice);
	mClearColor = clearColor;
	createCommandBuffers();
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

void RenderSystem::setupDebugCallback()
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
