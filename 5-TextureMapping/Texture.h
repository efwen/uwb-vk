#pragma once

//Vulkan
#include <vulkan/vulkan.h>

//STL
#include <string>

//Forward declare to avoid circular dependency
class RenderSystem;

class Texture
{
public:
	Texture(RenderSystem* renderSystem, VkDevice device);

	Texture& load(unsigned char* data, int width, int height, int channels);
protected:
	int mWidth;
	int mHeight;
	int mChannels;
	VkDeviceSize mImageSize;

	//Vulkan handles
	VkImage mImage;
	VkDeviceMemory mImageMemory;

	RenderSystem* mRenderSystem;
	VkDevice mDevice;

	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
};