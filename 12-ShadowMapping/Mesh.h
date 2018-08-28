#pragma once

//Vulkan
#include <vulkan/vulkan.h>

//STL
#include <vector>
#include <memory>

//uwb-vk
#include "VulkanContext.h"
#include "BufferManager.h"
#include "Vertex.h"

/** @class Mesh
	
	@brief A Mesh composed of vertex and index buffers

	@author Nicholas Carpenetti

	@date 20 July 2018
*/
class Mesh
{
public:
	/** @brief Constructor
		@param context The VulkanContext object
		@param bufferManager The BufferManager, 
			used for making vertex and index buffers
	*/
	Mesh(std::shared_ptr<VulkanContext> context, std::shared_ptr<BufferManager> bufferManager);
	~Mesh();
	
	/** @brief Create Vertex and index buffers from vectors of vertices and indices
		@param vertices A vector of vertices
		@param indices  A vector of indices
	*/
	void load(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	/** @brief Free all resources
	*/
	void free();

	/** @brief get the number of indices
		@return The number of indices
	*/
	uint32_t getIndexCount();
	/** @brief Get a handle to the Vertex Buffer
		@return The vertex buffer
	*/
	VkBuffer getVertexBuffer();
	/** @brief Get a handle to the Index Buffer
		@return The index buffer
	*/
	VkBuffer getIndexBuffer();
protected:
	std::shared_ptr<VulkanContext> mContext;			///< The RenderSystem's VulkanContext
	std::shared_ptr<BufferManager> mBufferManager;		///< The RenderSystem's BufferManager

	//vertex buffer
	std::vector<Vertex> mVertices;						///< The vertices in the mesh
	VkBuffer mVertexBuffer;								///< The VkBuffer object for holding vertices
	VkDeviceMemory mVertexBufferMemory;					///< The device memory for the vertices

	//index buffer
	std::vector<uint32_t> mIndices;						///< The indices in the mesh
	VkBuffer mIndexBuffer;								///< The VkBuffer object for the indices
	VkDeviceMemory mIndexBufferMemory;					///< The device memory for the indices
};