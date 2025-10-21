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

GLfloat vertices[] = {
	//x		y		z		//Texture coordinates
	-0.5f, -0.5f, 0.0f,		0.0f, 0.0f,
	-0.5f, 0.5f, 0.0f,		0.0f, 1.0f,
	0.5f, 0.5f, 0.0f,		1.0f, 1.0f,
	0.5f, -0.5f, 0.0f,		1.0f, 0.0f,
};

GLuint indices[] = {
	0, 2, 1,
	0, 3, 2

};

//TODO: implement primitive models like Cube.cpp, etc...
std::vector<Model> sceneModels = {};

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}


int main() 
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//create a new glfw window
	GLFWwindow* window = glfwCreateWindow(1000, 1000, "ive made it textured....", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to initialize glfw window";
		glfwTerminate();
		return -1;
	}
	//set the glfw context to the new window
	glfwMakeContextCurrent(window);

	//initialize glad
	gladLoadGL();

	//set glWiewport
	glViewport(0, 0, 1000, 1000);

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


	//Test texture
	int widthImg, heightImg, numColCh;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* bytes = stbi_load("MilitaryMeme.png", &widthImg, &heightImg, &numColCh, 0);

	GLuint texture;
	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, widthImg, heightImg, 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(bytes);
	glBindTexture(GL_TEXTURE_2D, 0);

	GLuint tex0Uni = glGetUniformLocation(basicShader.ID, "tex0");
	basicShader.use();
	glUniform1i(tex0Uni, 0);

	//process loop
	glfwMakeContextCurrent(window);
	while (!glfwWindowShouldClose(window)) {
		//process calls
		processInput(window);

		//render calls
		//clear window to cornflower blue
		glClearColor(100 / 255.0f, 149 / 255.0f, 237 / 255.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//use the compiled shader program
		basicShader.use();
		float timeValue = glfwGetTime();
		float greenValue = sin(timeValue) / 2.0f + 0.5f;
		float redValue = 1.0f - (sin(timeValue) / 2.0f + 0.5f);
		float blueValue = cos(timeValue) / 2.0f + 0.5f;
		basicShader.setVec4("ourColor", redValue, greenValue, blueValue, 1.0f);
		glBindTexture(GL_TEXTURE_2D, texture);

		//render triangles
		VAO1.Bind();
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

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