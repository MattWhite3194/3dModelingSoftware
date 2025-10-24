//Library includes
#include <iostream>
#include <stdio.h>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

//UI Framework
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

//Local includes
#include "shader_s.h"
#include "Camera.h"
#include "Mesh.h"
#include "ObjectPrimitives.h"
#include "MeshUtilities.h"
#include "Viewport.h"

const int width = 1000;
const int height = 1000;
bool firstMouse = true;
double lastX, lastY, lastScrollY = 0.0f;
Camera camera;
Viewport* viewport;
glm::mat4 Projection = glm::perspective(glm::radians(51.0f), (float)(width / height), 0.1f, 100.0f);
std::vector<Mesh> sceneMeshes = {};
Mesh* currentSelectedMesh = nullptr;

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse) {
		return;
	}
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
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse) {
		return;
	}
	if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
		float x = (2.0f * lastX) / width - 1.0f;
		float y = 1.0f - (2.0f * lastY) / height;

		glm::vec4 rayClip(x, y, -1.0f, 1.0f);

		glm::vec4 rayEye = glm::inverse(Projection) * rayClip;
		rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

		glm::vec3 rayDir = glm::normalize(
			glm::vec3(glm::inverse(camera.GetViewMatrix()) * rayEye)
		);
		glm::vec3 origin = camera.ZoomPosition;

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

void DisplaySelectedMeshWindow() {
	ImGui::Begin("Mesh");                          // Create a window called "Hello, world!" and append into it.

	if (ImGui::DragFloat3("Translation", glm::value_ptr(currentSelectedMesh->Translation), 0.1f)) {
		currentSelectedMesh->transformDirty = true;
	}
	if (ImGui::DragFloat3("Rotation", glm::value_ptr(currentSelectedMesh->Rotation), 0.1f)) {
		currentSelectedMesh->transformDirty = true;
	}
	if (ImGui::DragFloat3("Scale", glm::value_ptr(currentSelectedMesh->Scale), 0.1f)) {
		currentSelectedMesh->transformDirty = true;
	}
	if (ImGui::Checkbox("Flat Shading", &currentSelectedMesh->flatShading)) {
		currentSelectedMesh->gpuDirty = !currentSelectedMesh->gpuDirty;
	}
	ImGui::ColorEdit4("Object Color", glm::value_ptr(currentSelectedMesh->ObjectColor));
	
	ImGui::End();
}


int main() 
{
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

	//Set up Imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init("#version 130");

	//set glWiewport
	glViewport(0, 0, width, height);
	

	//Create shaders
	Shader objectShader("objectVert.vert", "objectFrag.frag");
	Shader viewportShader("viewportVert.vert", "viewportFrag.frag");
	glm::vec3 lightPos(1.2f, 1.0f, 10.0f);

	//process loop
	glfwMakeContextCurrent(window);

	//Add default cube
	sceneMeshes.push_back(CreateCube());

	//Add Light Source (for visualization)
	sceneMeshes.push_back(CreateCube());
	sceneMeshes[1].ScaleBy(glm::vec3(0.1f, 0.1f, 0.1f));
	sceneMeshes[1].ObjectColor = glm::vec4(0.7f, 0.7f, 0.0f, 1.0f);
	//sceneMeshes[1].Translate(lightPos);
	static float f = 0.0f;
	static int counter = 0;

	glEnable(GL_DEPTH_TEST);
	//initiate viewport
	viewport = new Viewport();
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();


		//TEST: Imgui testing
		
		if (currentSelectedMesh) {
			DisplaySelectedMeshWindow();
		}
		


		//Viewport Rendering

		//clear viewport
		glClearColor(0.2f, 0.2f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//set shader transforms
		
		//ENSURE THE SHADER IS ACTIVE BEFORE SETTING VARIABLES, 30 MINUTES OF HEADACHES AND WASTED TIME
		viewportShader.use();
		viewportShader.setMat4("projection", Projection);
		viewportShader.setMat4("view", camera.GetViewMatrix());

		//Draw viewport
		viewport->Draw(viewportShader);

		objectShader.use();
		objectShader.setVec3("lightPos", glm::vec3(glm::vec4(camera.ZoomPosition, 1.0f) * glm::rotate(glm::mat4(1.0f), glm::radians<float>(45.0f), glm::vec3(0.0f, 1.0f, 0.0f))));
		objectShader.setMat4("projection", Projection);
		objectShader.setMat4("view", camera.GetViewMatrix());
		//draw meshes
		//sceneMeshes[1].Rotate(glm::vec3(0.0f, 0.001f, 0.0f));
		for (std::vector<Mesh>::iterator Mesh = sceneMeshes.begin(); Mesh != sceneMeshes.end(); ++Mesh) {
			Mesh->Draw(objectShader);
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}