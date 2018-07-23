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
const std::string GROUND_TEXTURE_PATH = "textures/ground.jpg";
const std::string DISPLACEMENT_MAP_PATH = "textures/displacement_map.jpg";

//model paths
const std::string GROUND_MODEL_PATH = "models/tessGround.obj";

//shader paths
const std::string VERT_SHADER_PATH = "shaders/tessSquare_vert.spv";
const std::string FRAG_SHADER_PATH = "shaders/tessSquare_frag.spv";
const std::string TESS_CONTROL_SHADER_PATH = "shaders/tessSquare_tesc.spv";
const std::string TESS_EVAL_SHADER_PATH = "shaders/tessSquare_tese.spv";

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
	float dispMod = 1.0f;
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
	std::shared_ptr<UBO> tessControlBuffer;
	float tessLevel = 1.0f;
	std::shared_ptr<UBO> tessEvalBuffer;
	xform mTestPlaneXform;
	float displacementMultiplier = 1.0f;


	//"Camera"
	float mCamDist = 5.0f;
	glm::vec3 mCamRotate = glm::vec3(0.0f);
	std::shared_ptr<Renderable> mTestPlane = nullptr;
public:
	VkApp();
	void run();

private:
	void initialize();
	void shutdown();
	void createWindow();
	void handleInput();

	void createTesselatedPlane();
	void updateMVPMatrices(const std::shared_ptr<Renderable>& renderable, const xform& renderableXform);
	void updateDemoOptions();
};
