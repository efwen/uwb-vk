#pragma once

const int MAX_LIGHTS = 8;			///< The maximum number of lights available

/** @brief The type of light being used
*/
enum class LightType : uint32_t
{
	None = 0,
	Directional,
	Point,
	Spot
};


/** @brief A Light Source

	A struct containing the info necessary for any particular light
	Note: member order is specific, as it is copied byte-by-byte
	into a buffer, and the shaders also expect the same order

	@author Nicholas Carpenetti

	@date 9 Aug 2018
*/
struct Light
{
	glm::vec4 position = glm::vec4(0.0);						///< Position of the light in world space
	glm::vec4 direction = glm::vec4(0.0, 0.0, -1.0, 1.0);		///< Normalized forward vector for the light source

	glm::vec4 ambient = glm::vec4(1.0, 0.0, 1.0, 1.0);			///< Ambient color of the light source
	glm::vec4 diffuse = glm::vec4(1.0, 0.0, 1.0, 1.0);			///< Diffuse color of the light source
	glm::vec4 specular = glm::vec4(1.0, 0.0, 1.0, 1.0);			///< Specular color of the light source

	uint32_t isEnabled = false;									///< Is the light source enabled?

	float constant = 0.0f;										///< Constant light attenuation
	float linear = 0.0f;										///< Linear light attenuation
	float quadratic = 0.0f;										///< Quadratic light attenuation

	float cutOff = 0.0f;										///< Inner Dot product cutoff value for spotlights
	float outerCutOff = 0.0f;									///< Outer Dot product cutoff value for spotlights
	LightType lightType = LightType::None;						///< The light type (dirctional, point, or spot)
	
	char padding[4];											///< Padding values to fix alignment issues passing to the shader
};


//A UBO containing the information necessary to calculate lighting
/** @brief Buffer information necessary to calculate lighting

	@author Nicholas Carpenetti

	@date 9 Aug 2018
*/
struct LightUBO
{
	glm::vec4 viewPos;			///< The position we are viewing from (in world space)
	Light lights[MAX_LIGHTS];	///< The array of lights we are using
};