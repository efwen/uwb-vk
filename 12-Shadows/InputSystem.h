#pragma once
#include <GLFW/glfw3.h>
	
#include <glm/glm.hpp>

class InputSystem 
{
private:
	static InputSystem * mEventHandleInstance;
	GLFWwindow *mWindow = nullptr;

	bool mKeyDown[GLFW_KEY_LAST];
	bool mKeyPrev[GLFW_KEY_LAST];
	bool mKeyPressed[GLFW_KEY_LAST];
	bool mKeyReleased[GLFW_KEY_LAST];

	bool mMouseDown[GLFW_MOUSE_BUTTON_LAST];
	bool mMousePrev[GLFW_MOUSE_BUTTON_LAST];
	bool mMousePressed[GLFW_MOUSE_BUTTON_LAST];
	bool mMouseReleased[GLFW_MOUSE_BUTTON_LAST];

	glm::vec3 mCursorPos;
	glm::vec3 mPrevCursorPos;
	glm::vec3 mDeltaCursorPos;
	bool cursorSet = false;
public:
	InputSystem();
	
	void initialize(GLFWwindow* window);
	void update();

	//status check
	bool isKeyDown(int key);
	bool isKeyPressed(int key);
	bool isKeyReleased(int key);
	bool isMouseDown(int button);
	bool isMousePressed(int button);
	bool isMouseReleased(int button);
	
	glm::vec3 getMouseDelta();
	glm::vec3 getMousePos();
	
private:
	void setEventHandling() { mEventHandleInstance = this; }
	static void dispatchKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (mEventHandleInstance)
			mEventHandleInstance->keyCallback(window, key, scancode, action, mods);
	};
	static void dispatchMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
		if (mEventHandleInstance)
			mEventHandleInstance->mouseButtonCallback(window, button, action, mods);
	}
	static void dispatchMousePosCallback(GLFWwindow* window, double xpos, double ypos) {
		if (mEventHandleInstance)
			mEventHandleInstance->mouseCursorCallback(window, xpos, ypos);
	}
	static void dispatchMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset){
		if (mEventHandleInstance)
			mEventHandleInstance->mouseScrollCallback(window, xoffset, yoffset);
	}

	//input callback methods
	void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	void mouseCursorCallback(GLFWwindow* window, double xpos, double ypos);
	void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
};