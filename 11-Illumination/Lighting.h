#pragma once

const int MAX_LIGHTS = 8;

//Enum for lightType, forced to use uint32_t
enum class LightType : uint32_t
{
	None = 0,
	Directional,
	Point,
	Spot
};

//A struct containing the info necessary for any particular light
//Note: member order is specific, as it is copied byte-by-byte
//into a buffer, and the shaders also expect the same order
struct Light
{
	//position and direction are both in world space
	glm::vec4 position = glm::vec4(0.0);
	glm::vec4 direction = glm::vec4(0.0, 0.0, -1.0, 1.0);

	//light colors (default magent for debugging)
	glm::vec4 ambient = glm::vec4(1.0, 0.0, 1.0, 1.0);
	glm::vec4 diffuse = glm::vec4(1.0, 0.0, 1.0, 1.0);
	glm::vec4 specular = glm::vec4(1.0, 0.0, 1.0, 1.0);

	uint32_t isEnabled = false;

	//attenuation parameters
	float constant = 0.0f;
	float linear = 0.0f;
	float quadratic = 0.0f;

	//cutoff values for spotlights (dot product values)
	//this created to make the circle shape
	float cutOff = 0.0f;
	float outerCutOff = 0.0f;
	LightType lightType = LightType::None;
	
	//bring the array up to a multiple of 32 bytes
	char padding[4];							
};

//A UBO containing the information necessary to calculate lighting
struct LightUBO
{
	glm::vec4 viewPos;			//the position we are viewing from (in world space)
	Light lights[MAX_LIGHTS];	//array of lights we are using
};