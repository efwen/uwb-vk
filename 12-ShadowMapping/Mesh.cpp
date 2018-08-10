#include "Mesh.h"

Mesh::Mesh(std::shared_ptr<VulkanContext> context, std::shared_ptr<BufferManager> bufferManager) :
	mContext(context)
,	mBufferManager(bufferManager)
{
}

Mesh::~Mesh() {}

void Mesh::load(std::vector<Vertex>& vertices, std::vector<uint32_t> &indices)
{
	//do we really need to keep a copy around?
	mVertices = vertices;
	mIndices = indices;

	mBufferManager->createVertexBuffer(mVertices, mVertexBuffer, mVertexBufferMemory);
	mBufferManager->createIndexBuffer(mIndices, mIndexBuffer, mIndexBufferMemory);
}

void Mesh::free()
{
	//cleanup index buffer
	vkDestroyBuffer(mContext->device, mIndexBuffer, nullptr);
	vkFreeMemory(mContext->device, mIndexBufferMemory, nullptr);

	//cleanup vertex buffer
	vkDestroyBuffer(mContext->device, mVertexBuffer, nullptr);
	vkFreeMemory(mContext->device, mVertexBufferMemory, nullptr);
}

uint32_t Mesh::getIndexCount()
{
	return static_cast<uint32_t>(mIndices.size());
}

VkBuffer Mesh::getVertexBuffer()
{
	return mVertexBuffer;
}

VkBuffer Mesh::getIndexBuffer()
{
	return mIndexBuffer;
}
