#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

/** @class Camera
	@brief A simple camera class

	@author Nicholas Carpenetti

	@date 9 August 2018
*/
class Camera
{
public:
	glm::vec3 position = glm::vec3(0.0f);				///< Camera position
	glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);	///< Camera's orientation
	glm::mat4 viewMat = glm::mat4(1.0f);				///< The View Matrix
	glm::mat4 projMat = glm::mat4(1.0f);				///<Projection Matrix

	glm::vec3 forward = glm::vec3(0.0f, 0.0f, 1.0f);	///< Forward Vector
	glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);		///< Right Vector
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);			///< Up Vector

	float fov = 45.0f;									///< The Camera's field of view (in degrees)
	float nearPlane = 0.1f;								///< The Camera's near plane distance (the near end of clip space)
	float farPlane = 1000.0f;							///< The Camera's far plane distance (the far end of clip space)

	/** @brief Constructor
		@param width The viewport width
		@param height The viewport height
	*/
	Camera(uint32_t width, uint32_t height)
	{
		updateViewMatrix();
		updateProjectionMatrix(width, height);
	}

	/** @brief Update the the camera's View matrix, 
			as well as the up, right and forward vectors
	*/
	void updateViewMatrix()
	{
		glm::mat4 rotMat = glm::toMat4(glm::inverse(rotation));
		glm::mat4 transMat = glm::translate(glm::mat4(1.0f), -position);

		viewMat = rotMat * transMat;

		up = rotation * glm::vec3(0.0f, 1.0f, 0.0f);
		right = rotation * glm::vec3(1.0f, 0.0f, 0.0f);
		forward = rotation * glm::vec3(0.0f, 0.0f, -1.0f);
	};

	/** @brief Update the Camera's Projection Matrix
		@param width Viewport width
		@param height Viewport height
	*/
	void updateProjectionMatrix(uint32_t width, uint32_t height)
	{
		//To correct clip space (vulkan has inverted y, 1/2 z)
		//which is incompatible with glm's default perspective method
		const glm::mat4 clipFix(1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.5f, 0.0f,
			0.0f, 0.0f, 0.5f, 1.0f);

		projMat = clipFix * glm::perspective(glm::radians(fov), (float)width / (float)height, nearPlane, farPlane);
	};
};