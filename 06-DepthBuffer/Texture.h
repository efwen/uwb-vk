#pragma once

//Vulkan
#include <vulkan/vulkan.h>

//STL
#include <string>
#include <memory>

#include "BufferManager.h"
#include "ImageManager.h"

class Texture
{
public:
	Texture(std::shared_ptr<VulkanContext> context, std::shared_ptr<BufferManager> bufferManager, std::shared_ptr<ImageManager> imageManager);

	Texture& load(unsigned char* data, int width, int height, int channels);
	void free();

	VkImageView getImageView() const { return mImageView; };
	VkSampler getSampler() const { return mSampler; };
protected:
	int mWidth;
	int mHeight;
	int mChannels;
	VkDeviceSize mImageSize;

	//Vulkan handles
	VkImage mImage;
	VkDeviceMemory mImageMemory;
	VkImageView mImageView;
	VkSampler mSampler;

	std::shared_ptr<VulkanContext> mContext;
	std::shared_ptr<BufferManager> mBufferManager;
	std::shared_ptr<ImageManager> mImageManager;

	void createTextureImage(unsigned char* pixelData);
	void createTextureImageView();
	void createTextureSampler();
};