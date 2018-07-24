#pragma once

//STL
#include <memory>
#include <stdexcept>


#include "VulkanContext.h"
#include "CommandPool.h"

class ImageManager
{
public:
	ImageManager(std::shared_ptr<VulkanContext> context, std::shared_ptr<CommandPool> commandPool);
	~ImageManager();

	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage & image, VkDeviceMemory & imageMemory);
	VkImageView createImageView(VkImage image, VkFormat imageFormat, VkImageAspectFlags aspectFlags);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	bool hasStencilComponent(VkFormat format);
private:
	std::shared_ptr<VulkanContext> mContext;
	std::shared_ptr<CommandPool> mCommandPool;
};