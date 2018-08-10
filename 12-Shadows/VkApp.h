#pragma once

#include <string>
#include <iostream>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/vec3.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "RenderSystem.h"
#include "InputSystem.h"
#include "Renderable.h"
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "Lighting.h"

const int WIDTH = 1280;
const int HEIGHT = 720;

//resource paths
const std::string BOX_DIFFUSE_PATH		= "textures/Brick_OldDestroyed_1k_d.tga";
const std::string BOX_NORMAL_PATH		= "textures/Brick_OldDestroyed_1k_n.tga";
const std::string BOX_SPECULAR_PATH		= "textures/Brick_OldDestroyed_1k_s.tga";
const std::string BOX_MODEL_PATH		= "models/oldCube.mesh";
const std::string BOX_VERT_SHADER_PATH  = "shaders/multipleLights_vert.spv";
const std::string BOX_FRAG_SHADER_PATH  = "shaders/multipleLights_frag.spv";

//light indicator
const std::string LIGHT_MODEL_PATH		 = "models/oldCube.mesh";
const std::string LIGHT_VERT_SHADER_PATH = "shaders/lightObj_vert.spv";
const std::string LIGHT_FRAG_SHADER_PATH = "shaders/lightObj_frag.spv";

//controls speeds
const float cCamTranslateSpeed = 20.0f;
const float cCamRotateSpeed = 100.0f;
const float cLightTranslateSpeed = 5.0f;

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

struct Material
{

	float shininess;
};
 
class VkApp
{
private:
	GLFWwindow* mWindow;
	RenderSystem mRenderSystem;
	InputSystem mInputSystem;
	float mElapsedTime = 0.0;
	float mPrevTime = 0.0;
	float mFrameTime = 0.0;

	std::vector<VkClearValue> clearColors = { {0.176f, 0.11f,  0.114f, 1.0f},
											  {0.937f, 0.749f, 0.376f, 1.0f},
											  {0.788f, 0.2f,   0.125f, 1.0f},
											  {0.0f,   0.0f,   0.0f,   1.0f} };	
	int clearColorIndex = 0;

	std::unique_ptr<Camera> mCamera;
	
	//Lights UBO
	std::shared_ptr<UBO> mLightUBOBuffer;
	LightUBO mLightUBO;

	std::shared_ptr<Renderable> mWall;
	std::shared_ptr<UBO> mWallMVPBuffer;
	Transform mWallXForm;

	std::shared_ptr<Renderable> mLightIndicators[MAX_LIGHTS];
	std::shared_ptr<UBO> mLightIndicatorMVPBuffer[MAX_LIGHTS];
	Transform mLightIndicatorXForm[MAX_LIGHTS];
	std::shared_ptr<UBO> mLightIndicatorLightBuffer[MAX_LIGHTS];
	//implied mLightUBO.lights[index]


	bool mLightOrbit = true;
	uint32_t mSelectedLight = 0;
	uint32_t mTotalLights = 0;
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
	void setupLights();

	void createLightIndicator(uint32_t lightIndex);
	void createWall();
	void updateMVPBuffer(const UBO& mvpBuffer, 
						const Renderable& renderable, 
						const Transform& renderableXform, 
						const Camera& cam);
};
