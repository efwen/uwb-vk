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
const bool WINDOWED = true;
const std::string WINDOW_TITLE = "12-MultipleLights";

//resource paths
const std::string BOX_DIFFUSE_PATH		= "Resources/Textures/UrbanTexturePack/Brick_OldDestroyed/Brick_OldDestroyed_1k_d.tga";
const std::string BOX_NORMAL_PATH		= "Resources/Textures/UrbanTexturePack/Brick_OldDestroyed/Brick_OldDestroyed_1k_n.tga";
const std::string BOX_SPECULAR_PATH		= "Resources/Textures/UrbanTexturePack/Brick_OldDestroyed/Brick_OldDestroyed_1k_s.tga";
const std::string BOX_MODEL_PATH		= "Resources/Meshes/cube.mesh";
const std::string BOX_VERT_SHADER_PATH  = "Resources/Shaders/multipleLights_vert.spv";
const std::string BOX_FRAG_SHADER_PATH  = "Resources/Shaders/multipleLights_frag.spv";

//ground
const std::string GROUND_DIFFUSE_PATH		= "Resources/Textures/UrbanTexturePack/Ground_Dirt/Ground_Dirt_1k_d.tga";
const std::string GROUND_NORMAL_PATH		= "Resources/Textures/UrbanTexturePack/Ground_Dirt/Ground_Dirt_1k_n.tga";
const std::string GROUND_SPECULAR_PATH		= "Resources/Textures/UrbanTexturePack/Ground_Dirt/Ground_Dirt_1k_s.tga";
const std::string GROUND_MESH_PATH			= "Resources/Meshes/ground.mesh";
const std::string GROUND_VERT_SHADER_PATH	= "Resources/Shaders/shadowReceive_vert.spv";
const std::string GROUND_FRAG_SHADER_PATH	= "Resources/Shaders/shadowReceive_frag.spv";

//light indicator
const std::string LIGHT_MODEL_PATH		 = "Resources/Meshes/cube.mesh";
const std::string LIGHT_VERT_SHADER_PATH = "Resources/Shaders/lightObj_vert.spv";
const std::string LIGHT_FRAG_SHADER_PATH = "Resources/Shaders/lightObj_frag.spv";

//controls speeds
const float cCamTranslateSpeed = 20.0f;
const float cCamRotateSpeed = 100.0f;
const float cLightTranslateSpeed = 5.0f;

/** @class Transform
	
	@brief An object combining position, rotation, and scale.
		Can be used to create a model matrix.

	@author Nicholas Carpenetti

	@date 22 July 2018
*/
class Transform {
public:
	glm::vec3 scale;		///< Scale of the object
	glm::vec3 position;		///< Position of the object
	glm::quat rotation;		///< Rotation of the object
public:
	/** @brief Transform Constructor 
		Sets scale to 1.0f in all axes,
		Sets position to origin,
		Sets 0 rotation
	*/
	Transform() :
		scale(glm::vec3(1.0f)),
		position(glm::vec3(0.0f)),
		rotation(glm::quat(0.0f, 0.0f, 0.0f, 1.0f)){}

	/** @brief Get a create a model matrix with the transform values */
	glm::mat4 getModelMatrix() const
	{
		glm::mat4 transMat = glm::translate(glm::mat4(1.0f), position);
		glm::mat4 rotMat = glm::toMat4(rotation);
		glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scale);

		return transMat * rotMat * scaleMat;
	}
};

/** @struct MVPMatrices
	
	@brief Struct of all the matrices sent to vertex shaders

	@author Nicholas Carpenetti

	@date 22 July 2018
*/
struct MVPMatrices {
	glm::mat4 model;		///< Model matrix
	glm::mat4 view;			///< View matrix
	glm::mat4 projection;	///< Projection matrix from the camera
	glm::mat4 normalMat;	///< Equivalent to transpose(inverse(modelview)). Used for light calculation
};



/** @class VkApp

	@brief Main Application class.

	Unique for every project, but shared among all of them are the creation of \
		the primary systems, creation of a scene, a main loop, and shutdown of all systems.
	
	Application 12 in particular demonstrates the use of a 

	@author Nicholas Carpenetti

	@date 21 June 2018
*/
class VkApp
{
private:
	GLFWwindow* mWindow;											///< The main window the application runs in
	RenderSystem mRenderSystem;										///< The System responsible for rendering
	InputSystem mInputSystem;										///< The System responsible for handling input

	//
	float mElapsedTime = 0.0;										///< Time elapsed since the beginning of the application (in seconds)
	float mPrevTime = 0.0;											///< Time elapsed in the previous frame (in seconds)
	float mFrameTime = 0.0;											///< Time elapsed this frame (in seconds)

	/// An array of clear colors (background colors) you can iterate through with the 'B' Key
	std::vector<VkClearValue> clearColors = { {0.176f, 0.11f,  0.114f, 1.0f},
											  {0.937f, 0.749f, 0.376f, 1.0f},
											  {0.788f, 0.2f,   0.125f, 1.0f},
											  {0.0f,   0.0f,   0.0f,   1.0f} };	
	int clearColorIndex = 0;										///< The index of the currently selected background color

	std::unique_ptr<Camera> mCamera;								///< A camera object used to view the scene
	
	//Lights UBO
	std::shared_ptr<UBO> mLightUBOBuffer;							///< A UBO for sending light information to shaders
	LightUBO mLightUBO;												///< Light source information

	//Cube renderable
	std::shared_ptr<Renderable> mCube;								///< A Cube Renderable in the center of the scene
	std::shared_ptr<UBO> mCubeMVPBuffer;							///< A UBO for sending the Cube's MVP matrices to the shaders
	Transform mCubeXForm;											///< The Cube's transform

	//Ground Renderable
	std::shared_ptr<Renderable> mGround;							///< A Ground Renderable to project shadows on
	std::shared_ptr<UBO> mGroundMVPBuffer;							///< A UBO for sending the ground's MVP matrices to the shaders
	Transform mGroundXForm;											///< The ground's transform

	//Shadow Info
	std::shared_ptr<UBO> mShadowVPBuffer;							///< A UBO for holding sending information about the shadow projection to make a shadow map
	glm::mat4 mShadowVP;											///< A View+Projection matrix for projecting shadows

	//Lights
	std::shared_ptr<Renderable> mLightIndicators[MAX_LIGHTS];		///< A Set of renderables indicating where light sources are
	std::shared_ptr<UBO> mLightIndicatorMVPBuffer[MAX_LIGHTS];		///< MVP matrices for each of the light indicator renderables
	Transform mLightIndicatorXForm[MAX_LIGHTS];						///< Transforms for each light indicator
	std::shared_ptr<UBO> mLightIndicatorLightBuffer[MAX_LIGHTS];	///< UBOs for each of the Light indicator MVP matrices

	bool mLightOrbit = true;										///< If the light source is in orbit mode
	uint32_t mSelectedLight = 0;									///< The index of the selected light
	uint32_t mTotalLights = 0;										///< The total number of lights used in the scene
public:
	/** @brief Main Application Constructor	*/
	VkApp();
	/** @brief Runs the application
		Initializes all systems, constructs the scene, runs the main loop,
		and shuts down all systems.
	*/
	void run();

private:

	//-----------------
	// Setup and Shutdown
	//-----------------

	/** @brief Creates a window and Initializes the primary systems 
		@param appName The name of the application
	*/
	void initialize(const std::string& appName);
	
	/** @brief Shuts down primary systems and cleans up resources */
	void shutdown();

	/** @brief Creates a GLFW window for the application */
	void createWindow();


	//-----------------
	// Input Handling & application logic
	//-----------------
	/** @brief Main Input handling Method
		Called once per frame
	*/
	void handleInput();

	/** @brief Moves the Camera
		This application implements simple FPS controls, but no up-down rotation
	*/
	void cameraControls();

	/** @brief Controls how the lights move in the scene
		There are two modes of light movement, indicated by mLightOrbit
		In orbit mode, all lights orbit around the origin. Otherwise,
		the user can select a specific light with the number keys and control
		the light with arrow keys and [/] (vertical axis)
	*/
	void lightControls();

	//---------------------
	// Scene Setup
	//---------------------
	/** @brief Create the camera and give it an initial state */
	void setupCamera();

	/** @brief Create lights for the scene and give them initial state */
	void setupLights();

	/** @brief Create a light indicator Renderable for a specific light 
		@param lightIndex the index of the light the indicator corresponds with
	*/
	void createLightIndicator(uint32_t lightIndex);

	/** @brief Create a cube to cast shadows of */
	void createCube();
	
	/** @brief Create a ground plane to cast shadows on */
	void createGround();

	//--------------------------
	// Buffer Updating
	//--------------------------

	/**	@brief Update the MVP buffer for a particular Renderable 
		@param mvpBuffer		The MVP Buffer UBO for the Renderable
		@param renderableXform	The Transform representing the Renderables state (provides the Model matrix)
		@param cam				The camera viewing the renderable (provides the View and Projection matrices)
	*/
	void updateMVPBuffer(const UBO& mvpBuffer, 
						const Transform& renderableXform, 
						const Camera& cam);

	/** @brief updates the MVP buffer for a light source and sets the VP buffer for the shadow it casts 
		@param light The light casting a shadow
	*/
	void updateShadowMVP(const Light& light);
};
