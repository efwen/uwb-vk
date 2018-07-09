#pragma once

//Vulkan
#include <vulkan/vulkan.h>

//STL
#include <string>
#include <memory>

//Forward declare to avoid circular dependency
class RenderSystem;
#include "BufferManager.h"

class Texture
{
public:
	Texture(RenderSystem* renderSystem, std::shared_ptr<DeviceContext> context, std::shared_ptr<BufferManager> bufferManager);

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

	RenderSystem* mRenderSystem;
	std::shared_ptr<DeviceContext> mContext;
	std::shared_ptr<BufferManager> mBufferManager;

	void createTextureImage(unsigned char* pixelData);
	void createTextureImageView();
	void createTextureSampler();
};