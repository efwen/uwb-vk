#pragma once

#include <string>
#include <iostream>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/vec3.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "RenderSystem.h"
#include "InputSystem.h"
#include "Renderable.h"
#include "Shader.h"
#include "Texture.h"

const int WIDTH = 1280;
const int HEIGHT = 720;

//resource paths
const std::string WALL_TEXTURE_PATH = "textures/brickWall/Brick_Wall_012_COLOR.jpg";
const std::string WALL_NORM_MAP_PATH = "textures/brickWall/Brick_Wall_012_NORM.jpg";
const std::string WALL_MODEL_PATH = "models/oldCube.mesh";
const std::string WALL_VERT_SHADER_PATH = "shaders/phong_vert.spv";
const std::string WALL_FRAG_SHADER_PATH = "shaders/phong_frag.spv";

//light indicator
const std::string LIGHT_MODEL_PATH = "models/oldCube.mesh";
const std::string LIGHT_VERT_SHADER_PATH = "shaders/lightObj_vert.spv";
const std::string LIGHT_FRAG_SHADER_PATH = "shaders/lightObj_frag.spv";

//controls speeds
const float cCamTranslateSpeed = 10.0f;
const float cCamRotateSpeed = 100.0f;
const float cModelTranslateSpeed = 5.0f;
const float cModelRotateSpeed = 1.0f;
const float cLightTranslateSpeed = 40.0f;
const float cOptionsModSpeed = 10.0f;

struct Transform {
	glm::vec3 scale = glm::vec3(1.0f);
	glm::vec3 position = glm::vec3(0.0f);
	glm::quat rotation = glm::quat(0.0f, 0.0f, 0.0f, 1.0f);

	glm::mat4 getModelMatrix() const
	{
		glm::mat4 transMat = glm::translate(glm::mat4(1.0f), position);
		glm::mat4 rotMat = glm::toMat4(rotation);
		glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scale);

		return transMat * rotMat * scaleMat;
	}
};

struct MVPMatrices {
	glm::mat4 projection;	//projection matrix from the camera
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 normalMat;	//equivalent to transpose(inverse(modelview))
};

struct Light {
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
};

class Camera
{
public:
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::quat rotation = glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
	glm::mat4 viewMat = glm::mat4(1.0f);
	glm::mat4 projMat = glm::mat4(1.0f);

	glm::vec3 forward = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	float fov = 45.0f;
	float nearPlane = 0.1f;
	float farPlane = 100.0f;

	Camera(uint32_t width, uint32_t height)
	{
		updateViewMat();
		updateProjectionMat(width, height);
	}

	void updateViewMat() 
	{		
		glm::mat4 rotMat = glm::toMat4(rotation);
		glm::mat4 transMat = glm::translate(glm::mat4(1.0f), -position);

		viewMat = rotMat * transMat;

		//update front, right, up
		forward = rotation * glm::vec3(0.0f, 0.0f, -1.0f);
		right = rotation * glm::vec3(1.0f, 0.0f, 0.0f);
		up = glm::cross(forward, right);
	};

	void updateProjectionMat(uint32_t width, uint32_t height)
	{
		projMat = glm::perspective(glm::radians(fov), (float)width / (float)height, nearPlane, farPlane);
	    projMat[1][1] *= -1;
	};
};
 
class VkApp
{
private:
	GLFWwindow* mWindow;
	RenderSystem mRenderSystem;
	InputSystem mInputSystem;
	double mTime = 0.0;
	double mPrevTime = 0.0;
	double mFrameTime = 0.0;

	std::vector<VkClearValue> clearColors = {   {0.0f,   0.0f,   0.0f,   1.0f },
												{0.937f, 0.749f, 0.376f, 1.0f},
												{0.788f, 0.2f,   0.125f, 1.0f},
												{0.176f, 0.11f,  0.114f, 1.0f} };		
	int clearColorIndex = 0;

	std::shared_ptr<Renderable> mWall = nullptr;
	std::shared_ptr<Renderable> mLightIndicator = nullptr;

	//UBOs
	std::shared_ptr<UBO> mWallMVPBuffer;
	Transform mWallXForm;

	std::shared_ptr<UBO> mLightIndicatorMVPBuffer;
	Transform mLightIndicatorXForm;
	
	//Lighting
	std::shared_ptr<UBO> mLightBuffer;
	std::shared_ptr<UBO> mLightPosBuffer;
	Light mLight;
	glm::vec3 mLightPos;

	std::unique_ptr<Camera> mCamera;

	bool mLightOrbit = true;
public:
	VkApp();
	void run();

private:
	void initialize();
	void shutdown();
	void createWindow();
	void handleInput();
	void cameraControls();
	void lightControls();

	void setupCamera();
	void setupLight();

	void createLightIndicator();
	void createWall();
	void updateMVPBuffer(const UBO& mvpBuffer, 
						const Renderable& renderable, 
						const Transform& renderableXform, 
						const Camera& cam);
};
