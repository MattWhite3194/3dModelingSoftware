//Library includes
#include <iostream>
#include <stdio.h>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>
#include <glm/gtc/matrix_transform.hpp>

//Local includes
#include "shader_s.h"
#include "Camera.h"
#include "Mesh.h"
#include "ObjectPrimitives.h"
#include "MeshUtilities.h"

const int width = 1000;
const int height = 1000;
bool firstMouse = true;
double lastX, lastY, lastScrollY = 0.0f;
Camera camera;
glm::mat4 Projection = glm::perspective(glm::radians(51.0f), (float)(width / height), 0.1f, 100.0f);
std::vector<Mesh> sceneMeshes = {};

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_3) == GLFW_PRESS) {
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
			camera.ProcessMousePanning(xoffset, yoffset);
		}
		else {
			camera.ProcessMouseMovement(xoffset, yoffset);
		}
	}
	else
		firstMouse = true;
}

void scroll_callback(GLFWwindow* window, double xpos, double ypos) {
	camera.ProcessMouseScroll(ypos);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
		double flippedY = height - lastY;
		glm::vec3 nearPoint = glm::unProject(glm::vec3(lastX, flippedY, 0.0f), camera.GetViewMatrix(), Projection, glm::vec4(0.0f, 0.0f, width, height));
		glm::vec3 farPoint = glm::unProject(glm::vec3(lastX, flippedY, 1.0f), camera.GetViewMatrix(), Projection, glm::vec4(0.0f, 0.0f, width, height));

		glm::vec3 dir = glm::normalize(nearPoint - farPoint);
		glm::vec3 origin = camera.ZoomPosition;

		Mesh* selected = nullptr;
		for (std::vector<Mesh>::iterator Mesh = sceneMeshes.begin(); Mesh != sceneMeshes.end(); ++Mesh) {
			
		}
	}
}


int main() 
{
	//TODO: VIEWPORT LIGHT POSITION IS CAMERA POSITION
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//create a new glfw window
	GLFWwindow* window = glfwCreateWindow(1000, 1000, "Doing a frame swap.", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to initialize glfw window";
		glfwTerminate();
		return -1;
	}
	//set the glfw context to the new window
	glfwMakeContextCurrent(window);
	glfwSetCursorPosCallback(window, cursor_pos_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);

	//initialize glad
	gladLoadGL();

	//set glWiewport
	glViewport(0, 0, width, height);
	

	//Create shaders
	Shader basicShader("basicVert.vert", "basicFrag.frag");
	glm::vec3 lightPos(1.2f, 1.0f, 10.0f);
	glEnable(GL_DEPTH_TEST);

	//process loop
	glfwMakeContextCurrent(window);

	//Add default cube
	sceneMeshes.push_back(CreateCube());

	//Add Light Source (for visualization)
	sceneMeshes.push_back(CreateCube());
	sceneMeshes[1].Scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));
	sceneMeshes[1].ObjectColor = glm::vec4(0.7f, 0.7f, 0.0f, 1.0f);

	while (!glfwWindowShouldClose(window)) {

		//render calls
		//clear window to cornflower blue
		glClearColor(0.2f, 0.2f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		basicShader.use();
		float time = glfwGetTime(); // or your own time variable
		float angle = time * glm::radians(30.0f); // 30 degrees per second
		glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec3 rotatedLightPos = glm::vec3(rotation * glm::vec4(lightPos, 1.0f));
		basicShader.setVec3("lightPos", glm::vec3(glm::vec4(camera.ZoomPosition, 1.0f) * glm::rotate(glm::mat4(1.0f), glm::radians<float>(45.0f), glm::vec3(0.0f, 1.0f, 0.0f))));
		basicShader.setMat4("projection", Projection);
		basicShader.setMat4("view", camera.GetViewMatrix());

		sceneMeshes[1].Translation = glm::translate(glm::mat4(1.0f), rotatedLightPos);
		for (std::vector<Mesh>::iterator Mesh = sceneMeshes.begin(); Mesh != sceneMeshes.end(); ++Mesh) {
			Mesh->Draw(basicShader);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}