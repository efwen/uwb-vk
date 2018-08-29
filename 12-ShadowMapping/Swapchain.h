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
#include "ImageManager.h"

/** @class Swapchain

	@brief A swapchain of images the Render System draws to the screen with

	@author Nicholas Carpenetti

	@date 4 July 2018
*/
class Swapchain 
{
public:
	/** @brief Constructor
		@param context		A pointer to the RenderSystem's VulkanContext
		@param imageManager	A pointer to the RenderSystem's ImageManager
	*/
	Swapchain(std::shared_ptr<VulkanContext> context, std::shared_ptr<ImageManager> imageManager);
	~Swapchain();

	/** @brief Initialize the Swapchain

		Creates a VkSwapchainKHR image, retrieves its VkImages and creates
		VkImageViews for them.

		@param surface A handle to the surface we are rendering to
		@param imageCount The number of images in the Swapchain
	*/
	void initialize(VkSurfaceKHR surface, uint32_t imageCount);

	/** @brief Cleanup any Vulkan resources the Swapchain has allocated */
	void cleanup();

	/** @brief Create framebuffers for each of the images in the swapchain
		@param renderPass The renderPass the frame buffer will attach to
	*/
	std::vector<VkFramebuffer> createFramebuffers(VkRenderPass renderPass);

	//--------------
	// Accessors
	//--------------

	VkSwapchainKHR getVkSwapchain() const;				///< Get the VkSwapchainKHR object
	std::vector<VkImageView> getImageViews() const;		///< Get a vector of the ImageViews for each swapchain image
	VkFormat getImageFormat() const;					///< Get the format of the swapchain images
	VkExtent2D getExtent() const;						///< Get the size of of the swapchain images (in pixels)
	uint32_t size() const;								///< Get the number of swapchain images
private:
	std::shared_ptr<VulkanContext> mContext;			///< A pointer to the render system's vulkan context
	std::shared_ptr<ImageManager> mImageManager;		///< A pointer to the render system's image manager
	
	VkSwapchainKHR mSwapchain;							///< The main VkSwapchainKHR object
	std::vector<VkImage> mImages;						///< The images composing the swapchain
	std::vector<VkImageView> mImageViews;				///< VkImageViews for each swapchain image
	VkFormat mImageFormat;								///< The format of the swapchain images
	VkExtent2D mExtent;									///< The size of the swapchain in pixels

private:

	/** @brief choose the best image format available to the given VkSurfaceKHR 
		
		Prefers { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR } as a format

		@param surface The VkSurfaceKHR object giving us a list of available formats
	*/
	VkSurfaceFormatKHR chooseSurfaceFormat(VkSurfaceKHR surface);

	/** @brief Choose the best present mode available 
		Order of preference: FIFO, MAILBOX, IMMEDIATE

		@param surface The VkSurfaceKHR object giving us a list of available present modes
	*/
	VkPresentModeKHR choosePresentMode(VkSurfaceKHR surface);

	/** @brief Choose extents (resolution) for the swapchain images
		@param capabilities the capabliities of the VkSurfaceKHR object
	*/
	VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	/** @brief Create ImageViews for each of the swapchain images */
	void createImageViews();
};