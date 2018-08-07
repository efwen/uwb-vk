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
const int MAX_LIGHTS = 1;


//resource paths
const std::string WALL_TEXTURE_PATH = "textures/brickWall/Brick_Wall_012_COLOR.jpg";
const std::string WALL_NORM_MAP_PATH = "textures/brickWall/Brick_Wall_012_NORM.jpg";
const std::string WALL_MODEL_PATH = "models/oldCube.mesh";
const std::string WALL_VERT_SHADER_PATH = "shaders/point_vert.spv";
const std::string WALL_FRAG_SHADER_PATH = "shaders/point_frag.spv";


//light indicator
const std::string LIGHT_MODEL_PATH = "models/oldCube.mesh";
const std::string LIGHT_VERT_SHADER_PATH = "shaders/lightObj_vert.spv";
const std::string LIGHT_FRAG_SHADER_PATH = "shaders/lightObj_frag.spv";

//controls speeds
const float cCamTranslateSpeed = 20.0f;
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


struct Light
{	
	uint32_t isEnabled = false;		//uint32_t so it will align properly when passed to the UBO
	uint32_t isLocal = false;		//too lazy to make a method that aligns it for you
	uint32_t isSpot = false;	
	float spotCosCutoff = 45.0f;
	float spotExponent = 1.0f;
	float constAtten = 0.0f;
	float linearAtten = 0.0f;
	float quadAtten = 0.0f;
	glm::vec4 ambient = glm::vec4(0.0);
	glm::vec4 diffuse = glm::vec4(0.0);
	glm::vec4 specular = glm::vec4(0.0);
};

struct LightTransform
{
	glm::vec4 position = glm::vec4(0.0);
	glm::vec4 direction = glm::vec4(0.0);
};

struct Material
{
	glm::vec4 emission;
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
	float shininess;
};

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
	float farPlane = 100.0f;

	Camera(uint32_t width, uint32_t height)
	{
		updateViewMatrix();
		updateProjectionMat(width, height);
	}

	void updateViewMatrix() 
	{		
		glm::mat4 rotMat = glm::toMat4(glm::inverse(rotation));
		glm::mat4 transMat = glm::translate(glm::mat4(1.0f), -position);

		viewMat =  rotMat * transMat;

		up = rotation * glm::vec3(0.0f, 1.0f, 0.0f);
		right = rotation * glm::vec3(1.0f, 0.0f, 0.0f);
		forward = glm::cross(up, right);
	};

	void updateProjectionMat(uint32_t width, uint32_t height)
	{
		//To correct clip space (vulkan has inverted y, 1/2 z)
		const glm::mat4 clip(1.0f,  0.0f,  0.0f,  0.0f,
							 0.0f, -1.0f,  0.0f,  0.0f,
							 0.0f,  0.0f,  0.5f,  0.0f,
							 0.0f,  0.0f,  0.5f,  1.0f);

		projMat = clip * glm::perspective(glm::radians(fov), (float)width / (float)height, nearPlane, farPlane);
	};
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
	Light mLight;
	std::shared_ptr<UBO> mLightXFormBuffer;
	LightTransform mLightXForm;

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
	void setupLights();

	void createLightIndicator();
	void createWall();
	void updateMVPBuffer(const UBO& mvpBuffer, 
						const Renderable& renderable, 
						const Transform& renderableXform, 
						const Camera& cam);
};
