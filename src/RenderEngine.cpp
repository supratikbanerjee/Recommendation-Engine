#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\glm.hpp>

#include "stb_image.h"
#include "OBJ_Loader.h"
#include "Camera.h"
#include "ObjectHandler.h"

#ifdef _WIN32

extern "C" {

	_declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;

}

#endif

unsigned int width = 1920;
unsigned int height = 1080;

ObjectHandler OH;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
float lastX = width / 2.0f;
float lastY = height / 2.0f;
bool firstMouse = true;

std::vector < glm::mat4 > local;
std::vector < glm::mat4 > global;

int objs;
float rotatez = 0.0;

std::vector < float > vertex_count;
std::vector <unsigned int> vertex_position;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
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

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}



void manageScene()
{
	for (int i = 0; i < objs; i++)
		global[i] = local[i];
	global[1] = glm::rotate(local[1], glm::radians(5*rotatez), glm::vec3(0.0f, 1.0f, 0.0f));
}

void display(Shader shader, Shader skybox_shader)
{
	//glDepthFunc(GL_LESS);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
	glBindVertexArray(OH.getVAO());
	float rough = 1.0;

	for (int i = 0; i < objs; i++)
	{
		manageScene();
		// draw in wireframe polygons.
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		//glBindTexture(GL_TEXTURE_2D, texture);
		/*if (i == 0)
			rough = 200.0f;
		else if (i == 1)
			rough = 200.0f;
		else if (i == 2)
			rough = 200.0f;
		else if (i == 3)
			rough = 200.0f;*/

		//shader.setVec3("position", glm::vec3(1.0f, 1.0f, 1.0f));
	
		shader.use();

		shader.setVec3("camView", camera.GetCameraPosition());
		shader.setVec3("objColor", glm::vec3(0.95, 0.45, 0.25));
		//shader.setFloat("shine_rough", rough);
		//shader.setVec3("objColor", glm::vec3(1.0, 0.8431372549019608, 0.0));
		shader.setMat4("proj", projection);
		shader.setMat4("view", view);
		shader.setMat4("model", global[i]);
		//std::cout << vertex_count[i] << std::endl;
		glDrawArrays(GL_TRIANGLES, vertex_position[i], vertex_count[i]);
		//glDrawArrays(GL_TRIANGLES, 0, 3);
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	// draw skybox as last
	glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
	skybox_shader.use();
	view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
	skybox_shader.setMat4("view", view);
	skybox_shader.setMat4("projection", projection);
	// skybox cube
	glBindVertexArray(OH.getSkyboxVAO());
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, OH.getCubemapTexture());
	glDrawArrays(GL_TRIANGLES, 0, 36);
	//glBindVertexArray(0);
	glDepthFunc(GL_LESS); // set depth function back to default
}

void init(Shader shader)
{
	const char *obj[] = { "../Assets/models/dot.obj","../Assets/models/cube.obj"};
	objs = (sizeof(obj) / sizeof(obj[0]));

	std::vector < std::vector< glm::vec3 >> vertices;
	std::vector < std::vector< glm::vec2 >> uv;
	std::vector < std::vector< glm::vec3 >> normals;
	std::vector < unsigned int > indices_vec_temp;
	std::vector < unsigned int > indices_norm_temp;


	std::vector< glm::vec3 > vert_temp;
	std::vector< glm::vec2 > uv_temp;
	std::vector< glm::vec3 > norm_temp;
	vertex_position.push_back(0);

	for (int i = 0; i < objs; i++)
	{
		global.push_back(glm::mat4(1.0f));
		local.push_back(glm::mat4(1.0f));
		local[i] = glm::scale(local[i], glm::vec3(0.5f, 0.5f, 0.5f));
		bool res = loadOBJ(obj[i], vert_temp, uv_temp, norm_temp, indices_vec_temp, indices_norm_temp);
		vertices.push_back(vert_temp);
		uv.push_back(uv_temp);
		normals.push_back(norm_temp);
		//std::cout << vert_temp.size() << std::endl;
		vert_temp.clear();
		uv_temp.clear();
		norm_temp.clear();
		//std::cout << vert_temp.size() << std::endl;
		vertex_count.push_back(vertices[i].size());
		vertex_position.push_back(vertex_position[i] + vertex_count[i]);
		//printf("count %d\n", vertices[i].size());
	}
	OH.genVAO(objs, vertices, normals, vertex_count, vertex_position, shader);
	OH.skyboxTex();
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow* window = glfwCreateWindow(width, height, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

	Shader shader("../Shaders/bp.vert", "../Shaders/on.frag");
	//Shader shader("../Shaders/cubemap.vs", "../Shaders/cubemap.fs");
	Shader skybox_shader("../Shaders/skybox.vs", "../Shaders/skybox.fs");
	OH = ObjectHandler();
	init(shader);
	skybox_shader.use();
	skybox_shader.setInt("skybox", 0);
	

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);
		rotatez += 0.1f;
		display(shader, skybox_shader);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}



