#include "Texture.h"
#include "RenderSystem.h"
#include <assert.h>

Texture::Texture(std::shared_ptr<VulkanContext> context, std::shared_ptr<BufferManager> bufferManager, std::shared_ptr<ImageManager> imageManager) :
	mWidth(0),
	mHeight(0),
	mChannels(0),
	mImageSize(0),
	mImage(VK_NULL_HANDLE),
	mImageMemory(VK_NULL_HANDLE),
	mContext(context),
	mBufferManager(bufferManager),
	mImageManager(imageManager)
{
}

Texture& Texture::load(unsigned char* pixelData, int width, int height, int channels) 
{
	assert(pixelData != nullptr);
	mWidth = width;
	mHeight = height;
	mChannels = channels;
	mImageSize = width * height * 4;

	createTextureImage(pixelData);
	createTextureImageView();
	createTextureSampler();

	return *this;
}

void Texture::free()
{
	//free up resources
	vkDestroySampler(mContext->device, mSampler, nullptr);
	vkDestroyImageView(mContext->device, mImageView, nullptr);
	vkDestroyImage(mContext->device, mImage, nullptr);
	vkFreeMemory(mContext->device, mImageMemory, nullptr);
	
	//reset all values in case we want to reuse this texture object
	mWidth = 0;
	mHeight = 0;
	mChannels = 0;
	mImageSize = 0;
	mSampler = VK_NULL_HANDLE;
	mImageView = VK_NULL_HANDLE;
	mImage = VK_NULL_HANDLE;
	mImageMemory = VK_NULL_HANDLE;
}

void Texture::createTextureImage(unsigned char* pixelData)
{
	//Create a staging buffer to store the pixel data
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	mBufferManager->createBuffer(mImageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(mContext->device, stagingBufferMemory, 0, mImageSize, 0, &data);
	memcpy(data, pixelData, static_cast<size_t>(mImageSize));
	vkUnmapMemory(mContext->device, stagingBufferMemory);

	mImageManager->createImage(mWidth, mHeight, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		mImage, mImageMemory);

	//copy the staging buffer to the texture image

	//transition from undefined layout to a layout conducive to being copied into
	mImageManager->transitionImageLayout(mImage,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	mBufferManager->copyBufferToImage(stagingBuffer,
		mImage,
		static_cast<uint32_t>(mWidth),
		static_cast<uint32_t>(mHeight));

	//Transition from transfer destination to shader reading
	mImageManager->transitionImageLayout(mImage,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(mContext->device, stagingBuffer, nullptr);
	vkFreeMemory(mContext->device, stagingBufferMemory, nullptr);
}

void Texture::createTextureImageView()
{
	mImageView = mImageManager->createImageView(mImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
}

void Texture::createTextureSampler()
{
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	//anisotropic filtering
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;

	//what color to return when sampling beyond image in clamp address mode
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

	//use texel values from [0, 1)
	samplerInfo.unnormalizedCoordinates = VK_FALSE;

	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	if (vkCreateSampler(mContext->device, &samplerInfo, nullptr, &mSampler) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create texture sampler!");
	}
}
