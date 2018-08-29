#pragma once

//STL
#include <memory>
#include <stdexcept>

#include "VulkanContext.h"
#include "CommandPool.h"

/**
	@class ImageManager

	@brief Allocation and Management of VkImage objects

	@author Nicholas Carpenetti

	@date 9 July 2018
*/
class ImageManager
{
public:
	/** @brief Constructor
		@param context The Vulkan Context to operate within
		@param commandPool The Command Pool
	*/
	ImageManager(std::shared_ptr<VulkanContext> context, std::shared_ptr<CommandPool> commandPool);
	~ImageManager();

	/** @brief Create a new VkImage object 
		@param width The width of the image
		@param height The height of the image
		@param format The image format
		@param tiling The type of tiling this image uses (either optimal or linear). i.e. how data is laid out in memory
		@param usage Flags for how the image will be used
		@param properties Required properties of the created image
		@param image The VkImage object to be created
		@param imageMemory The memory associated with the created VkImage
	*/
	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage & image, VkDeviceMemory & imageMemory);
	
	/** @brief Create a new VkImageView object
		@param image A handle to the associated VkImage
		@param imageFormat The texel format of the image
		@param aspectFlags indicating which aspect of the image you want to view (i.e. color or depth)
	*/
	VkImageView createImageView(VkImage image, VkFormat imageFormat, VkImageAspectFlags aspectFlags);
	
	/** @brief Transition an image from one layout to another
		@param image A handle to the associated VkImage
		@param format The texel format of the image
		@param oldLayout The old Layout of the image
		@param newLayout The target layout
	*/
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);


	/** @brief From a list of candidates, pick the best image format supported by the device
		@param candidates A non-zero sized list of candidates for possible image formats
		@param tiling How the image is tiled
		@param features Features required by the chosen image format
	*/
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	
	/** @brief Determine if a particular image format has a stencil component
		@param format The format being evaluated
	*/
	bool hasStencilComponent(VkFormat format);
private:
	std::shared_ptr<VulkanContext> mContext;		///< A reference to the Vulkan Context
	std::shared_ptr<CommandPool> mCommandPool;		///< The Command Pool
};