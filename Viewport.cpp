#include "Viewport.h"
#include "ObjectPrimitives.h"
#include <glm/gtx/string_cast.hpp>
#include "MeshUtilities.h"

void Viewport::InitGrid() {
	float gridPlane[12] = {
		-1, -1, 0,
		1,	-1, 0,
		1,	1, 0, 
		-1, 1, 0
	};

	glGenVertexArrays(1, &gridVao);
	glGenBuffers(1, &gridVbo);

	glBindVertexArray(gridVao);
	glBindBuffer(GL_ARRAY_BUFFER, gridVbo);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), gridPlane, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
}

void Viewport::Init() {
	//Default Cube
	sceneMeshes.push_back(CreateCube(1.0f));
	viewportCamera = new Camera();
	objectShader = new Shader("objectVert.vert", "objectFrag.frag");
	edgeShader = new Shader("edgeVert.vert", "edgeFrag.frag", "edgeGeom.geom");
	gridShader = new Shader("gridVert.vert", "gridFrag.frag");
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
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// Viewport as image texture
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
	glDepthMask(GL_TRUE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	glClearColor(0.2f, 0.2f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Set transforms on shaders
	objectShader->use();
	objectShader->setVec3("lightPos", glm::vec3(glm::vec4(viewportCamera->ZoomPosition, 1.0f) * glm::rotate(glm::mat4(1.0f), glm::radians<float>(45.0f), glm::vec3(0.0f, 1.0f, 0.0f))));
	objectShader->setMat4("projection", Projection);
	objectShader->setMat4("view", viewportCamera->GetViewMatrix());
	edgeShader->use();
	edgeShader->setMat4("projection", Projection);
	edgeShader->setMat4("view", viewportCamera->GetViewMatrix());
	edgeShader->setMat4("model", glm::mat4(1.0f));
	edgeShader->setVec2("viewportSize", glm::vec2(viewportWidth, viewportHeight));

	for (const auto& mesh : sceneMeshes) {
		mesh->Draw(*objectShader);
		mesh->DrawEdges(*edgeShader);
	}

	gridShader->use();
	gridShader->setMat4("projection", Projection);
	gridShader->setMat4("view", viewportCamera->GetViewMatrix());
	gridShader->setMat4("model", glm::mat4(1.0f));
	gridShader->setVec4("gridColor", glm::vec4(0.7f));
	gridShader->setVec3("cameraPos", viewportCamera->Position);
	glBindVertexArray(gridVao);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

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
		for (const auto& mesh : sceneMeshes) {
			float dist;
			Face* face;
			if (PickMesh(*mesh, origin, rayDir, dist, face)) {
				if (dist < closestDistance) {
					selected = mesh.get();
					closestDistance = dist;
					selectedFace = face;
				}
			}
		}
		SetSelected(selected);
	}
}

void Viewport::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_F && action == GLFW_PRESS) {
		if (currentSelectedMesh) {
			viewportCamera->SetFocus(currentSelectedMesh->Translation, 10.0f);
		}
	}
	else if (key == GLFW_KEY_DELETE && action == GLFW_PRESS) {
		if (currentSelectedMesh) {
			DeleteMesh(currentSelectedMesh);
		}
	}
}

void Viewport::AddMesh(std::unique_ptr<Mesh> mesh) {
	Mesh* newSelected = mesh.get();
	sceneMeshes.push_back(std::move(mesh));
	SetSelected(newSelected);
}

void Viewport::DeleteMesh(Mesh* mesh) {
	auto it = std::find_if(sceneMeshes.begin(), sceneMeshes.end(),
		[&](const std::unique_ptr<Mesh>& m) {
			return m.get() == mesh;
		});

	if (it != sceneMeshes.end()) {
		sceneMeshes.erase(it);
	}
	currentSelectedMesh = nullptr;
}

void Viewport::SetSelected(Mesh* mesh) {
	if (currentSelectedMesh) {
		currentSelectedMesh->selected = false;
	}
	currentSelectedMesh = mesh;
	if (mesh) {
		mesh->selected = true;
		forceMeshTab = true;
	}
}