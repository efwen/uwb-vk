#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
	glm::vec3 position = glm::vec3(0.0f);
	glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	glm::mat4 viewMat = glm::mat4(1.0f);
	glm::mat4 projMat = glm::mat4(1.0f);

	glm::vec3 forward = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	float fov = 45.0f;
	float nearPlane = 0.1f;
	float farPlane = 1000.0f;

	Camera(uint32_t width, uint32_t height)
	{
		updateViewMatrix();
		updateProjectionMat(width, height);
	}

	void updateViewMatrix()
	{
		glm::mat4 rotMat = glm::toMat4(glm::inverse(rotation));
		glm::mat4 transMat = glm::translate(glm::mat4(1.0f), -position);

		viewMat = rotMat * transMat;

		up = rotation * glm::vec3(0.0f, 1.0f, 0.0f);
		right = rotation * glm::vec3(1.0f, 0.0f, 0.0f);
		forward = rotation * glm::vec3(0.0f, 0.0f, -1.0f);
	};

	void updateProjectionMat(uint32_t width, uint32_t height)
	{
		//To correct clip space (vulkan has inverted y, 1/2 z)
		const glm::mat4 clipFix(1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.5f, 0.0f,
			0.0f, 0.0f, 0.5f, 1.0f);

		projMat = clipFix * glm::perspective(glm::radians(fov), (float)width / (float)height, nearPlane, farPlane);
	};
};