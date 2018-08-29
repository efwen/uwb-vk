#pragma once

//Vulkan
#include <vulkan/vulkan.h>

//STL
#include <string>
#include <memory>

#include "BufferManager.h"
#include "ImageManager.h"

/** @class Texture

	@brief

	@author Nicholas Carpenetti

	@date 
*/
class Texture
{
public:
	Texture(std::shared_ptr<VulkanContext> context, std::shared_ptr<BufferManager> bufferManager, std::shared_ptr<ImageManager> imageManager);

	Texture& load(unsigned char* data, int width, int height, int channels);
	void free();

	VkImageView getImageView() const { return mImageView; };
	VkSampler getSampler() const { return mSampler; };
protected:
	int mWidth;						///< The resolution width of the texture
	int mHeight;					///< The resolution height of the texture
	int mChannels;					///< The number of color channels
	VkDeviceSize mImageSize;

	//Vulkan handles
	VkImage mImage;
	VkDeviceMemory mImageMemory;
	VkImageView mImageView;
	VkSampler mSampler;

	std::shared_ptr<VulkanContext> mContext;
	std::shared_ptr<BufferManager> mBufferManager;
	std::shared_ptr<ImageManager> mImageManager;

protected:
	void createTextureImage(unsigned char* pixelData);
	void createTextureImageView();
	void createTextureSampler();
};