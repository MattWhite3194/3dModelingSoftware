//Library includes
#include <iostream>
#include <stdio.h>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//Local includes
#include "shader_s.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "Model.h"
#include "Texture.h"
#include "Camera.h"

const int width = 1000;
const int height = 1000;
bool firstMouse = true;
double lastX, lastY, lastScrollY = 0.0f;
Camera camera;
glm::mat4 Projection = glm::perspective(glm::radians(51.0f), (float)(width / height), 0.1f, 100.0f);

GLfloat vertices[] = {
	//x		y		z		//Texture coordinates
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // 0
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // 1
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // 2
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // 3

    // Back face
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // 4
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // 5
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // 6
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // 7

    // Top face
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, // 8 (same as 3)
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // 9 (same as 2)
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // 10 (same as 6)
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // 11 (same as 7)
    
    // Bottom face
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // 12 (same as 0)
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // 13 (same as 1)
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, // 14 (same as 5)
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // 15 (same as 4)

    // Right face
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // 16 (same as 1)
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // 17 (same as 5)
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // 18 (same as 6)
     0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // 19 (same as 2)

    // Left face
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // 20 (same as 0)
    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // 21 (same as 4)
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // 22 (same as 7)
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f  // 23 (same as 3)
};

GLuint indices[] = {
	// Front face
    0, 1, 2,
    2, 3, 0,
    
    // Back face
    4, 5, 6,
    6, 7, 4,
    
    // Top face
    8, 9, 10,
    10, 11, 8,
    
    // Bottom face
    12, 13, 14,
    14, 15, 12,
    
    // Right face
    16, 17, 18,
    18, 19, 16,
    
    // Left face
    20, 21, 22,
    22, 23, 20

};

//TODO: implement primitive models like Cube.cpp, etc...
std::vector<Model> sceneModels = {};

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	Camera_Movement direction = FORWARD;

}


void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
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
		camera.ProcessMouseMovement(xoffset, yoffset);
	}
	else
		firstMouse = true;
}

void scroll_callback(GLFWwindow* window, double xpos, double ypos) {
	camera.ProcessMouseScroll(ypos);
}


int main() 
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//create a new glfw window
	GLFWwindow* window = glfwCreateWindow(1000, 1000, "Three cheers for the camera!!!!!!!!!!", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to initialize glfw window";
		glfwTerminate();
		return -1;
	}
	//set the glfw context to the new window
	glfwMakeContextCurrent(window);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	//initialize glad
	gladLoadGL();

	//set glWiewport
	glViewport(0, 0, width, height);

	VAO VAO1;
	VAO1.Bind();

	VBO VBO1(vertices, sizeof(vertices));
	EBO EBO1(indices, sizeof(indices));

	VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 5 * sizeof(float), (void*)0);
	VAO1.LinkAttrib(VBO1, 1, 2, GL_FLOAT, 5 * sizeof(float), (void*)(3*sizeof(float)));
	VAO1.Unbind();
	VBO1.Unbind();
	EBO1.Unbind();
	

	//Create shaders
	Shader basicShader("vert.vs", "frag.fs");


	//Create and load new texture
	Texture militaryMeme("MilitaryMeme.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);

	//bind a shader to the texture
	militaryMeme.texUnit(basicShader, "tex0", 0);

	glEnable(GL_DEPTH_TEST);

	//process loop
	glfwMakeContextCurrent(window);
	while (!glfwWindowShouldClose(window)) {
		//process calls
		processInput(window);

		//render calls
		//clear window to cornflower blue
		glClearColor(0.7f, 0.7f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUniformMatrix4fv(glGetUniformLocation(basicShader.ID, "camMatrix"), 1, GL_FALSE, glm::value_ptr(Projection * camera.GetViewMatrix() * glm::mat4(1.0f)));

		//bind the military man texture
		//TODO: for no shading, make a default texture object
		militaryMeme.Bind();

		//render triangles
		VAO1.Bind();
		glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(int), GL_UNSIGNED_INT, 0);

		//swap buffers (i.e. draw the color data to the screen) and poll events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	VAO1.Delete();
	VBO1.Delete();
	EBO1.Delete();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}