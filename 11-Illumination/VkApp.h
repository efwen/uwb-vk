#pragma once

#include <string>
#include <iostream>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/vec3.hpp>

#include "RenderSystem.h"
#include "InputSystem.h"
#include "Renderable.h"
#include "Shader.h"
#include "Texture.h"

const int WIDTH = 1280;
const int HEIGHT = 720;

//texture paths
const std::string WALL_TEXTURE_PATH = "textures/Brick_Wall_012/Brick_Wall_012_COLOR.jpg";

//model paths
const std::string WALL_MODEL_PATH = "models/wall.obj";

//shader paths
const std::string VERT_SHADER_PATH = "shaders/wall_vert.spv";
const std::string FRAG_SHADER_PATH = "shaders/wall_frag.spv";

//controls speeds
const float mCamTranslateSpeed = 10.0f;
const float mCamRotateSpeed = 100.0f;
const float mModelTranslateSpeed = 5.0f;
const float mModelRotateSpeed = 100.0f;
const float optionsModSpeed = 10.0f;

struct xform
{
	glm::vec3 scale = glm::vec3(1.0f);
	glm::vec3 pos = glm::vec3(0.0f);
	glm::vec3 rot = glm::vec3(0.0f);
};

struct MVPMatrices {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
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


	//UBOs
	std::shared_ptr<UBO> mWallMVPBuffer;
	xform mTestPlaneXform;
	
	//Lighting
	std::shared_ptr<UBO> mAmbientLightBuffer;
	glm::vec4 mAmbientColor = { 1.0, 1.0, 1.0, 1.0f };
	float ambientMagnitude = 0.1f;

	//"Camera"
	float mCamDist = 5.0f;
	glm::vec3 mCamRotate = glm::vec3(0.0f);

	std::shared_ptr<Renderable> mWall = nullptr;
public:
	VkApp();
	void run();

private:
	void initialize();
	void shutdown();
	void createWindow();
	void handleInput();
	void cameraControls();

	void createWall();
	void updateMVPMatrices(const std::shared_ptr<Renderable>& renderable, const xform& renderableXform);
};
