#include "Viewport.h"
#include "ObjectPrimitives.h"
#include <glm/gtx/string_cast.hpp>
#include "MeshUtilities.h"

void Viewport::InitGrid() {
	std::vector<glm::vec3> gridVertices;
	int half = GRID_SIZE / 2;

	for (int i = -half; i <= half; i++) {
		//line size of GRID_SIZE parallel to X-axis
		gridVertices.push_back(glm::vec3(i, 0, -half));
		gridVertices.push_back(glm::vec3(i, 0, half));

		//line size of GRID_SIZE parallel to Z-axis
		gridVertices.push_back(glm::vec3(-half, 0, i));
		gridVertices.push_back(glm::vec3(half, 0, i));
	}

	glGenVertexArrays(1, &gridVao);
	glGenBuffers(1, &gridVbo);

	glBindVertexArray(gridVao);
	glBindBuffer(GL_ARRAY_BUFFER, gridVbo);
	glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(glm::vec3), gridVertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
}

void Viewport::InitShaders() {
	sceneMeshes.push_back(CreateCube());
	viewportCamera = new Camera();
	std::cout << glm::to_string(viewportCamera->ZoomPosition) << " " << glm::to_string(viewportCamera->Front) << std::endl;
	objectShader = new Shader("objectVert.vert", "objectFrag.frag");
	viewportShader = new Shader("viewportVert.vert", "viewportFrag.frag");
}

void Viewport::ResizeViewportFramebuffer(int width, int height) {
	if (width <= 0 || height <= 0) return;
	if (width == viewportWidth && height == viewportHeight) return;

	viewportWidth = width;
	viewportHeight = height;
	CreateViewportFramebuffer();

	// Update projection matrix for new size
	Projection = glm::perspective(glm::radians(51.0f),
		(float)width / (float)height,
		0.1f, 1000.0f);
}

void Viewport::CreateViewportFramebuffer() {
	std::cout << "creating viewport buffer" << std::endl;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// Color texture
	glGenTextures(1, &fboTexture);
	glBindTexture(GL_TEXTURE_2D, fboTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewportWidth, viewportHeight,
		0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, fboTexture, 0);

	// Depth buffer
	glGenRenderbuffers(1, &fboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, fboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
		viewportWidth, viewportHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
		GL_RENDERBUFFER, fboDepth);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer incomplete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Viewport::Draw() {
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, viewportWidth, viewportHeight);

	glEnable(GL_DEPTH_TEST);

	glClearColor(0.2f, 0.2f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	viewportShader->use();
	viewportShader->setMat4("projection", Projection);
	viewportShader->setMat4("view", viewportCamera->GetViewMatrix());
	glBindVertexArray(gridVao);
	glLineWidth(2.0f);
	glDrawArrays(GL_LINES, 0, (GRID_SIZE + 1) * 4);
	glBindVertexArray(0);

	objectShader->use();
	objectShader->setVec3("lightPos", glm::vec3(glm::vec4(viewportCamera->ZoomPosition, 1.0f) * glm::rotate(glm::mat4(1.0f), glm::radians<float>(45.0f), glm::vec3(0.0f, 1.0f, 0.0f))));
	objectShader->setMat4("projection", Projection);
	objectShader->setMat4("view", viewportCamera->GetViewMatrix());
	

	for (std::vector<Mesh>::iterator Mesh = sceneMeshes.begin(); Mesh != sceneMeshes.end(); ++Mesh) {
		Mesh->Draw(*objectShader);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Viewport::cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
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
			viewportCamera->ProcessMousePanning(xoffset, yoffset);
		}
		else {
			viewportCamera->ProcessMouseMovement(xoffset, yoffset);
		}
	}
	else
		firstMouse = true;
}

void Viewport::scroll_callback(GLFWwindow* window, double xpos, double ypos) {
	viewportCamera->ProcessMouseScroll(ypos);
}

void Viewport::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
		float x = (2.0f * lastX) / viewportWidth - 1.0f;
		float y = 1.0f - (2.0f * lastY) / viewportHeight;

		glm::vec4 rayClip(x, y, -1.0f, 1.0f);

		glm::vec4 rayEye = glm::inverse(Projection) * rayClip;
		rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

		glm::vec3 rayDir = glm::normalize(
			glm::vec3(glm::inverse(viewportCamera->GetViewMatrix()) * rayEye)
		);
		glm::vec3 origin = viewportCamera->ZoomPosition;

		Mesh* selected = nullptr;
		Face* selectedFace = nullptr;
		float closestDistance = FLT_MAX;
		//Iterate through all the meshes and select the mesh that was intersected closest to the camera
		for (std::vector<Mesh>::iterator Mesh = sceneMeshes.begin(); Mesh != sceneMeshes.end(); ++Mesh) {
			float dist;
			Face* face;
			if (PickMesh(*Mesh, origin, rayDir, dist, face)) {
				std::cout << dist << std::endl;
				if (dist < closestDistance) {
					selected = &*Mesh;
					closestDistance = dist;
					selectedFace = face;
				}
			}
		}
		if (currentSelectedMesh) {
			currentSelectedMesh->selected = false;
		}
		if (selected) {
			selected->selected = true;
			currentSelectedMesh = selected;
		}
		else {
			currentSelectedMesh = nullptr;
		}
	}
}