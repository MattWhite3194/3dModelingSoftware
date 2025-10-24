#ifndef VIEWPORT_CLASS_H
#define VIEWPORT_CLASS_H

#include <glad/glad.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "shader_s.h"
#include <string>
#include <vector>
#include "Mesh.h"
#include "Face.h"
#include "Camera.h"
#include <GLFW/glfw3.h>

class Viewport {
public:
	GLuint fbo = 0, fboTexture = 0, fboDepth = 0, gridVao = 0, gridVbo = 0;
	Shader* objectShader;
	Shader* viewportShader;
	Camera* viewportCamera;
	Mesh* currentSelectedMesh;
	Face* currentSelectedFace;
	std::vector<Mesh> sceneMeshes;
	int viewportWidth = 1000, viewportHeight = 1000;
	glm::mat4 Projection;
	glm::vec3 viewportLight = glm::vec3(1.2f, 1.0f, 10.0f);
	const int GRID_SIZE = 20;
	double lastX, lastY;
	bool firstMouse = true;
	bool IsActive = false;
	Viewport() {
		InitGrid();
		InitShaders();
	}
	void InitGrid();
	void InitShaders();
	void CreateViewportFramebuffer();
	void ResizeViewportFramebuffer(int width, int height);
	void Draw();
	void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
	void scroll_callback(GLFWwindow* window, double xpos, double ypos);
	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
};

#endif
