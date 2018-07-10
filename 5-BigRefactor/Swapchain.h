#pragma once

//vulkan, glfw
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//STL
#include <vector>
#include <algorithm>
#include <iostream>
#include <memory>

//uwb-vk
#include "VulkanContext.h"
#include "QueueFamilies.h"

class Swapchain 
{
public:
	Swapchain(std::shared_ptr<VulkanContext> context);
	~Swapchain();

	void initialize(VkSurfaceKHR surface, uint32_t imageCount);
	void cleanup();

	void createImageViews();
	std::vector<VkFramebuffer> createFramebuffers(VkRenderPass renderPass);

	VkSwapchainKHR getVkSwapchain() const;
	std::vector<VkImageView> getImageViews() const;
	VkFormat getImageFormat() const;
	VkExtent2D getExtent() const;
	uint32_t size() const;
private:
	std::shared_ptr<VulkanContext> mContext;
	
	VkSwapchainKHR mSwapchain;
	std::vector<VkImage> mImages;
	std::vector<VkImageView> mImageViews;
	VkFormat mImageFormat;
	VkExtent2D mExtent;


	VkSurfaceFormatKHR chooseSurfaceFormat(VkSurfaceKHR surface);
	VkPresentModeKHR choosePresentMode(VkSurfaceKHR surface);
	VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);
};