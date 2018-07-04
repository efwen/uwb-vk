#pragma once

//vulkan, glfw
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//STL
#include <vector>
#include <algorithm>
#include <iostream>

//uwb-vk
#include "QueueFamilies.h"

class Swapchain 
{
public:
	Swapchain();
	~Swapchain();

	void initialize(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, QueueFamilyIndices selectedIndices, VkExtent2D extent);
	void cleanup(VkDevice device);

	std::vector<VkFramebuffer> createFramebuffers(VkDevice device, VkRenderPass renderPass);

	VkFormat getImageFormat();
	VkExtent2D getExtent();
private:
	VkSurfaceFormatKHR chooseSurfaceFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	VkPresentModeKHR choosePresentMode(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	void createImageViews(VkDevice device);

private:
	bool initialized = false;
	VkSwapchainKHR mSwapchain;
	std::vector<VkImage> mImages;
	std::vector<VkImageView> mImageViews;
	VkFormat mImageFormat;
	VkExtent2D mExtent;
};