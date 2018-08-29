#pragma once

#include <vulkan/vulkan.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <array>

/** @struct Vertex
	
	@brief Vertex used in a Mesh. Holds positions, colors, normals, tangents and texture coordinates.

	@author Nicholas Carpenetti

	@date 28 June 2018
*/
struct Vertex {
	glm::vec4 pos;			///< Position of the vertex
	glm::vec4 color;		///< Color of the
	glm::vec3 normal;		///< Normal of the vertex
	glm::vec3 tangent;		///< Optional tangent to the normal
	glm::vec2 texCoord;		///< Texture Coordinate

	/** @brief Get a Binding Description for pipeline creation */
	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	/** @brief Get a description of all the Vertex attributes for pipeline creation 
		Describes each attribute, binding number, locations of each attribute,
		attribute formats, and offsets.
	*/
	static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions = {};

		//position attribute
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		//color attribute
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		//normal attribute
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, normal);

		//tangent attribute
		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex, tangent);

		//texture coordinate attribute
		attributeDescriptions[4].binding = 0;
		attributeDescriptions[4].location = 4;
		attributeDescriptions[4].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[4].offset = offsetof(Vertex, texCoord);

		return attributeDescriptions;
	}

	/** @brief equality operator used by std::unordered_map
		@param other A vertex to compare to
	*/
	bool operator== (const Vertex& other) const
	{
		return pos == other.pos &&
			   color == other.color &&
			   normal == other.normal &&
			   tangent == other.tangent &&
			   texCoord == other.texCoord;
	}

	friend std::ostream& operator<<(std::ostream& os, const Vertex& v);
};

/** @brief << print operator for Vertices
	@param os Stream object to print to
	@param v  The vertex to print
*/
inline std::ostream& operator<< (std::ostream& os, const Vertex& v)
{
	os << "[p(" << v.pos.x		 << ", " << v.pos.y		  << ", " << v.pos.z	 << v.pos.w	  << ")" << 
		 " c("  << v.color.r     << ", " << v.color.g	  << ", " << v.color.b	 << v.color.a <<")" << 
		 " n("  << v.normal.x    << ", " << v.normal.y    << ", " << v.normal.z  << ")" <<
		 " t("  << v.tangent.x   << ", " << v.tangent.y   << ", " << v.tangent.z << ")" <<
		 " uv(" << v.texCoord.x	 << ", " << v.texCoord.y  << ")]";
	return os;
}

namespace std {

	template<> struct hash<Vertex>
	{	
		/** @brief Get a hash value for a vertex 
		Needs improvement to include more attributes
		*/
		size_t operator()(Vertex const& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.pos) ^ 
					(hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^ 
					(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}
