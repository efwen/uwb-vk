#pragma once
#include <GLFW/glfw3.h>
	
#include <glm/glm.hpp>

/** @class InputSystem
	
	@brief Input handling using GLFW

	This class handles all of the inputs recieved from windows through GLFW.
	Down states, pressed states, and release states for both mouse and keyboard
	can be queried. Note: in addition to the standard GLFW callbacks, this class
	uses dispatch methods as GLFW can only use static methods for the callback.

	@author Nicholas Carpenetti

	@date 24 Jun 2018
*/
class InputSystem 
{
private:
	static InputSystem * mEventHandleInstance;		///< A handle to the
	GLFWwindow *mWindow = nullptr;					///< The GLFW window we are receiving input events from

	bool mKeyDown[GLFW_KEY_LAST];					///< Current key states
	bool mKeyPrev[GLFW_KEY_LAST];					///< Previous key states
	bool mKeyPressed[GLFW_KEY_LAST];				///< Key pressed states (i.e. previously up, now down)
	bool mKeyReleased[GLFW_KEY_LAST];				///< Key released states (i.e. previously down, now up)

	bool mMouseDown[GLFW_MOUSE_BUTTON_LAST];		///< Current Mouse button states
	bool mMousePrev[GLFW_MOUSE_BUTTON_LAST];		///< Previous mouse button states
	bool mMousePressed[GLFW_MOUSE_BUTTON_LAST];		///< Moused button pressed states (i.e. previously up, now down)
	bool mMouseReleased[GLFW_MOUSE_BUTTON_LAST];	///< Moused button released states (i.e. previously down, now up)

	glm::vec3 mCursorPos;							///< Cursor position (x and y are screen coordinates, z is scroll position)
	glm::vec3 mPrevCursorPos;						///< Previous cursor position (x and y are screen coordinates, z is scroll position)
	glm::vec3 mDeltaCursorPos;						///< Delta curor position (x and y are screen coordinates, z is scroll position)
	bool cursorSet = false;							///< Whether the cursor position has already been set. This prevents issues with the delta cursor position
public:
	/** @brief Constructor
	*/
	InputSystem();
	/** @brief Initialize the Input System
		@param window An initialized GLFW window
	*/
	void initialize(GLFWwindow* window);
	/** @brief Update the input states
	*/
	void update();

	//------------------------
	//Input Status Checking
	//------------------------
	
	/** @brief Check if a particular keyboard key is down
		@param key to query (glfw key name convention is GLFW_KEY_<key name>)
	*/
	bool isKeyDown(int key);
	/** @brief Check if a particular key has been pressed
		@param key Key to query (glfw key name convention is GLFW_KEY_<key name>)
	*/
	bool isKeyPressed(int key);
	/** @brief Check if a particular key has been released
		@param key Key to query (glfw key name convention is GLFW_KEY_<key name>)
	*/
	bool isKeyReleased(int key);
	/** @brief Check if a particular mouse button is down
		@param button Button to query (glfw key name convention is GLFW_KEY_<key name>)
	*/
	bool isMouseDown(int button);
	/** @brief Check if a particular mouse button has been presssed
		@param button Button to query (glfw key name convention is GLFW_KEY_<key name>)
	*/
	bool isMousePressed(int button);
	/** @brief Check if a particular mouse button has been released
		@param button Button to query (glfw key name convention is GLFW_KEY_<key name>)
	*/
	bool isMouseReleased(int button);
	
	/** @brief Get the Delta position of the mouse in a vec3
	*/
	glm::vec3 getMouseDelta();
	/** @brief Get the Current position of the mouse in a vec3
	*/
	glm::vec3 getMousePos();
	
private:
	/**	@brief set the static variable instance used by the dispatch methods
	*/
	void setEventHandling() { mEventHandleInstance = this; }

	//-----------------------------------
	//Input Callback Dispatching Methods
	//-----------------------------------
	/** @brief callback dispatching for the keyboard
		@param window The GLFW window triggering the event
		@param key The key associated with the event
		@param scancode A system specific scancode of the key
		@param action The key action asssociated with the event (i.e. GLFW_PRESS, GLFW_RELEASE, etc.)
		@param mods A bitfield describing which modifier keys are held down
	*/
	static void dispatchKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (mEventHandleInstance)
			mEventHandleInstance->keyCallback(window, key, scancode, action, mods);
	};
	/** @brief callback dispatching for the mouse buttons
		@param window The GLFW window triggering the event
		@param button The button associated with the event
		@param action The mouse button action asssociated with the event (i.e. GLFW_PRESS, GLFW_RELEASE, etc.)
		@param mods A bitfield describing which modifier buttons are held down
	*/
	static void dispatchMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
		if (mEventHandleInstance)
			mEventHandleInstance->mouseButtonCallback(window, button, action, mods);
	}
	/** @brief callback dispatching for mouse movement (x and y only)
		@param window The GLFW window triggering the event
		@param xpos The x-coordinate of the mouse position
		@param ypos The y-coordinate of the mouse position
	*/
	static void dispatchMousePosCallback(GLFWwindow* window, double xpos, double ypos) {
		if (mEventHandleInstance)
			mEventHandleInstance->mouseCursorCallback(window, xpos, ypos);
	}
	/** @brief callback dispatching for mouse scroll
		@param window The GLFW window triggering the event
		@param xoffset The x-offset of the mouse scroll button
		@param yoffset The y-offfset of the mouse scroll button
	*/
	static void dispatchMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset){
		if (mEventHandleInstance)
			mEventHandleInstance->mouseScrollCallback(window, xoffset, yoffset);
	}

	//-------------------------
	//input callback methods
	//-------------------------
	/** @brief callback for the keyboard
		@param window The GLFW window triggering the event
		@param key The key associated with the event
		@param scancode A system specific scancode of the key
		@param action The key action asssociated with the event (i.e. GLFW_PRESS, GLFW_RELEASE, etc.)
		@param mods A bitfield describing which modifier keys are held down
	*/
	void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	/** @brief callback for the mouse buttons
		@param window The GLFW window triggering the event
		@param button The button associated with the event
		@param action The mouse button action asssociated with the event (i.e. GLFW_PRESS, GLFW_RELEASE, etc.)
		@param mods A bitfield describing which modifier buttons are held down
	*/
	void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	/** @brief callback for mouse movement (x and y only)
		@param window The GLFW window triggering the event
		@param xpos The x-coordinate of the mouse position
		@param ypos The y-coordinate of the mouse position
	*/
	void mouseCursorCallback(GLFWwindow* window, double xpos, double ypos);
	/** @brief callback for mouse scroll
		@param window The GLFW window triggering the event
		@param xoffset The x-offset of the mouse scroll button
		@param yoffset The y-offfset of the mouse scroll button
	*/
	void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
};