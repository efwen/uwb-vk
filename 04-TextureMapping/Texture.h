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
	VkDevice mDevice;

	void createTextureImage(unsigned char* pixelData);
	void createTextureImageView();
	void createTextureSampler();
};