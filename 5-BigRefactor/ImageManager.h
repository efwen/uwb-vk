#pragma once

//STL
#include <memory>
#include <stdexcept>


#include "DeviceContext.h"
#include "CommandPool.h"

class ImageManager
{
public:
	ImageManager(std::shared_ptr<DeviceContext> context, std::shared_ptr<CommandPool> commandPool);
	~ImageManager();

	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage & image, VkDeviceMemory & imageMemory);
	VkImageView createImageView(VkImage image, VkFormat imageFormat);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
private:
	std::shared_ptr<DeviceContext> mContext;
	std::shared_ptr<CommandPool> mCommandPool;
};