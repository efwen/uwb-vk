#pragma once

//Vulkan
#include <vulkan/vulkan.h>

//STL
#include <string>
#include <memory>

#include "BufferManager.h"
#include "ImageManager.h"

/** @class Texture

	@brief A class for holding the information loaded from texture files

	@author Nicholas Carpenetti

	@date 5 July 2018
*/
class Texture
{
public:
	/** Texture Constructor
		@param context			A pointer to the RenderSystem's VulkanContext
		@param bufferManager	A pointer to the RenderSystem's buffer manager
		@param imageManager		A pointer to the RenderSystem's image manager
	*/
	Texture(std::shared_ptr<VulkanContext> context, std::shared_ptr<BufferManager> bufferManager, std::shared_ptr<ImageManager> imageManager);

	/** @brief load raw image data into this texture
		@param data		A byte array of image data
		@param width	The width of the image source in pixels
		@param height	The height of the image source in pixels
		@param channels The number of color channels in the image source
	*/
	Texture& load(unsigned char* data, int width, int height, int channels);

	/** @brief Free the Vulkan resources allocated for this texture */
	void free();

	//-------------
	// Accessors
	//-------------
	
	/** Get the VkImageView object for this texture */
	VkImageView getImageView() const { return mImageView; };	
	
	/** Get the VkSampler object for this texture */
	VkSampler getSampler() const { return mSampler; };			
protected:
	//Primary system pointers
	std::shared_ptr<VulkanContext> mContext;			///< A shared pointer to the RenderSystem's VulkanContext
	std::shared_ptr<BufferManager> mBufferManager;		///< A shared pointer to the RenderSystem's BufferManager
	std::shared_ptr<ImageManager> mImageManager;		///< A shared pointer to the RenderSystem's ImageManager

	int mWidth;											///< The resolution width of the texture
	int mHeight;										///< The resolution height of the texture
	int mChannels;										///< The number of color channels
	VkDeviceSize mImageSize;							///< The size of the image data in bytes
		
	//Vulkan handles
	VkImage mImage;										///< The VkImageHandle
	VkDeviceMemory mImageMemory;						///< A handle to the device memory holding the image
	VkImageView mImageView;								///< A Vulkan ImageView for the texture
	VkSampler mSampler;									///< A sampler so the image can be used in a shader

protected:
	/** @brief create a VkImage object from the pixel data
		@param pixelData A byte array of pixel data
	*/
	void createTextureImage(unsigned char* pixelData);

	/** @brief Create a VkImageView object for accessing the VkImage */
	void createTextureImageView();
	
	/** @brief Create a VkSampler object for access inside shaders */
	void createTextureSampler();
};