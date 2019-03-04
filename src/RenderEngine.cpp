#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <glm\glm.hpp>

#include "utils/stb_image.h"
#include "application/camera.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"
//#include "global_variables.h"
#include "application/renderer.h"
#include "events/keyboard_events.h"
#include "events/system_events.h"
//#include "events/mouse_events.h"
#include "ui/ui_manager.h"
#include "utils/gizmo.h"
#include "RenderEngine.h"
#include "utils/gizmo_manager.h"
#include "application/scene_manager.h"


/*

TODO

NEED TO CREATE A CLASS FOR COMMON OBJECT SHARING!

VERY VERY IMPORTANT

*/

#ifdef _WIN32
extern "C" {
	_declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}
#endif

Camera camera(glm::vec3(0.0f,0.0f, 3.0f));
SystemEvents sysEvent = SystemEvents();
vfGizmo3DClass getGizmo;
GizmoManager gizmo;
SceneManager scene;


int width = 1920;
int height = 1080;

float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

bool firstMouse = true;
float lastX = width / 2.0f;
float lastY = height / 2.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}


void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	if (sysEvent.getProcessMouseMovementStatus())
		camera.ProcessMouseMovement(xoffset, yoffset);
	getGizmo.motion(xpos, ypos);
	gizmo.TrackMotion(xpos, ypos);

}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	getGizmo.mouse((vgButtons)(button), mods, action == GLFW_PRESS, lastX, lastY);
	printf("clicked \n");
}

void init(GLFWwindow *window)
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
	ImGui_ImplGlfwGL3_Init(window, true);
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow* window = glfwCreateWindow(width, height, "Xtreme Render Engine", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Xtreme Render Engine Error" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);


	getGizmo.setGizmoRotControl((vgButtons)GLFW_MOUSE_BUTTON_LEFT, (vgModifiers)0 /* evNoModifier */);

	//for pan and zoom/dolly
	getGizmo.setDollyControl((vgButtons)GLFW_MOUSE_BUTTON_RIGHT, (vgModifiers)GLFW_MOD_CONTROL | GLFW_MOD_SHIFT);
	getGizmo.setPanControl((vgButtons)GLFW_MOUSE_BUTTON_RIGHT, (vgModifiers)0);

	//If you need, for rotations around a single axis
	getGizmo.setGizmoRotXControl((vgButtons)GLFW_MOUSE_BUTTON_LEFT, (vgModifiers)GLFW_MOD_SHIFT);
	getGizmo.setGizmoRotYControl((vgButtons)GLFW_MOUSE_BUTTON_LEFT, (vgModifiers)GLFW_MOD_CONTROL);
	getGizmo.setGizmoRotZControl((vgButtons)GLFW_MOUSE_BUTTON_LEFT, (vgModifiers)GLFW_MOD_ALT | GLFW_MOD_SUPER);


	// Now call viewportSize with the dimension of window/screen
	// It is need to set mouse sensitivity for rotation
	// You need to call it also in your "reshape" function: when resize the window (look below)

	getGizmo.viewportSize(width, height);



	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	//Shader shader("../Shaders/cubemap.vs", "../Shaders/cubemap.fs");
	Shader shader("../Shaders/PBR/PBR.vs.glsl", "../Shaders/PBR/PBR3.fs");
	Shader skybox_shader("../Shaders/skybox.vs", "../Shaders/skybox.fs");
	init(window);
	Renderer renderer = Renderer();
	KeyboardEvents keyboard = KeyboardEvents();
	UIManager ui = UIManager();
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		keyboard.processInput(window, camera, deltaTime, sysEvent);
	
		renderer.Render(&shader, skybox_shader, camera, getGizmo);

		if (sysEvent.getRenderEngineUIStatus())
		{
			ui.DrawUI();
		}

		/*if (sysEvent.getFirstPersonCameraStatus())
		{
			scene.getFirstPersonCamPosition(&fpCamPos);
			camera.setCamPosition(&fpCamPos);
		}
		else if (!sysEvent.getFirstPersonCameraStatus())
		{
			scene.getThirdPersonCamPosition(&tpCamPos);
			camera.setCamPosition(&tpCamPos);
		}*/

		glfwSwapInterval(0);
		glfwSwapBuffers(window);
	}

	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();

	return 0;
}