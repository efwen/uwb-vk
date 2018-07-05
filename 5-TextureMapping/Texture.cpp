#include "Texture.h"
#include "RenderSystem.h"
#include <assert.h>

Texture::Texture(RenderSystem* renderSystem, VkDevice device) :
	mWidth(0),
	mHeight(0),
	mChannels(0),
	mImageSize(0),
	mImage(VK_NULL_HANDLE),
	mImageMemory(VK_NULL_HANDLE),
	mRenderSystem(renderSystem),
	mDevice(device)
{
}

Texture& Texture::load(unsigned char* pixelData, int width, int height, int channels) 
{
	assert(pixelData != nullptr);
	mWidth = width;
	mHeight = height;
	mChannels = channels;
	mImageSize = width * height * 4;

	//Create a staging buffer to store the pixel data
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	mRenderSystem->createBuffer(mImageSize, 
								VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
								VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
								stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(mDevice, stagingBufferMemory, 0, mImageSize, 0, &data);
		memcpy(data, pixelData, static_cast<size_t>(mImageSize));
	vkUnmapMemory(mDevice, stagingBufferMemory);

	createImage(mWidth, mHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, 
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		mImage, mImageMemory);

	//copy the staging buffer to the texture image
	mRenderSystem->transitionImageLayout(mImage,
									VK_FORMAT_R8G8B8A8_UNORM,
									VK_IMAGE_LAYOUT_UNDEFINED,
									VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	mRenderSystem->copyBufferToImage(stagingBuffer,
									mImage,
									static_cast<uint32_t>(mWidth),
									static_cast<uint32_t>(mHeight));

	mRenderSystem->transitionImageLayout(mImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(mDevice, stagingBuffer, nullptr);
	vkFreeMemory(mDevice, stagingBufferMemory, nullptr);

	return *this;
}

void Texture::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage & image, VkDeviceMemory & imageMemory)
{
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(mDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(mDevice, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = mRenderSystem->findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(mDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(mDevice, image, imageMemory, 0);
}
