#ifndef CAMERA_CLASS_H
#define CAMERA_CLASS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "shader_s.h"

class Camera {
public:
	/// <summary>
	/// The camera's position in the world
	/// </summary>
	glm::vec3 cameraPos;
	/// <summary>
	/// What the camera is looking at
	/// </summary>
	glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	/// <summary>
	/// The direction the camera is pointing
	/// </summary>
	glm::vec3 cameraDirection;
	/// <summary>
	/// The global up direction
	/// </summary>
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	/// <summary>
	/// The positive X axis in camera space
	/// </summary>
	glm::vec3 cameraRight;
	/// <summary>
	/// The positive y axis of the camera space
	/// </summary>
	glm::vec3 cameraUp;
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);

	int width;
	int height;

	double lastX, lastY;
	bool firstMouse = true;
	float yaw = -90.0f, pitch = 0.0f;

	float cameraSpeed = 0.1f;
	float sensetivity = 100.0f;

	Camera(int width, int height, glm::vec3 position);

	void Matrix(float FOVdeg, float nearPlane, float farPlane, Shader& shader, const char* uniform);
	void Inputs(GLFWwindow* window);
	void MouseInputs(GLFWwindow* window);

};

#endif
