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

Viewport* viewport;
ImVec2 viewportCursorPos = ImVec2(0, 0);

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
	if (ImGui::GetIO().WantCaptureMouse && !viewport->IsActive) {
		return;
	}
	viewport->cursor_pos_callback(window, viewportCursorPos.x, viewportCursorPos.y);
}

void scroll_callback(GLFWwindow* window, double xpos, double ypos) {
	ImGui_ImplGlfw_ScrollCallback(window, xpos, ypos);
	if (ImGui::GetIO().WantCaptureMouse && !viewport->IsActive) {
		return;
	}
	viewport->scroll_callback(window, xpos, ypos);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
	if (ImGui::GetIO().WantCaptureMouse && !viewport->IsActive) {
		return;
	}
	viewport->mouse_button_callback(window, button, action, mods);
}

void DisplaySelectedMeshWindow() {
	ImGui::Begin("Mesh");                          // Create a window called "Hello, world!" and append into it.
	if (viewport->currentSelectedMesh) {
		if (ImGui::DragFloat3("Translation", glm::value_ptr(viewport->currentSelectedMesh->Translation), 0.1f)) {
			viewport->currentSelectedMesh->transformDirty = true;
		}
		if (ImGui::DragFloat3("Rotation", glm::value_ptr(viewport->currentSelectedMesh->Rotation), 0.1f)) {
			viewport->currentSelectedMesh->transformDirty = true;
		}
		if (ImGui::DragFloat3("Scale", glm::value_ptr(viewport->currentSelectedMesh->Scale), 0.1f)) {
			viewport->currentSelectedMesh->transformDirty = true;
		}
		if (ImGui::Checkbox("Flat Shading", &viewport->currentSelectedMesh->flatShading)) {
			viewport->currentSelectedMesh->gpuDirty = !viewport->currentSelectedMesh->gpuDirty;
		}
		ImGui::ColorEdit4("Object Color", glm::value_ptr(viewport->currentSelectedMesh->ObjectColor));
	}
	ImGui::End();
}

int main() 
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//create a new glfw window
	GLFWwindow* window = glfwCreateWindow(1400, 1000, "Doing a frame swap.", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to initialize glfw window";
		glfwTerminate();
		return -1;
	}

	//set the glfw context to the new window
	glfwMakeContextCurrent(window);
	//
	//

	//initialize glad
	gladLoadGL();

	//Set up Imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init("#version 130");

	//process loop
	glfwMakeContextCurrent(window);

	//initiate viewport
	viewport = new Viewport();
	viewport->CreateViewportFramebuffer();
	glfwSetCursorPosCallback(window, cursor_pos_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::DockSpaceOverViewport();

		//TEST: Imgui testing
		
		DisplaySelectedMeshWindow();
		
		//TODO: Viewport Rendering
		ImGui::Begin("Viewport");
		ImVec2 mousePos = ImGui::GetMousePos();
		ImVec2 winPos = ImGui::GetWindowPos();
		ImVec2 curPos = ImGui::GetCursorPos(); 
		viewportCursorPos = ImVec2(
			mousePos.x - winPos.x - curPos.x,
			mousePos.y - winPos.y - curPos.y
		);
		bool isViewportHovered = ImGui::IsWindowHovered();
		bool isViewportFocused = ImGui::IsWindowFocused();
		viewport->IsActive = isViewportHovered;
		ImVec2 size = ImGui::GetContentRegionAvail();
		viewport->ResizeViewportFramebuffer(size.x, size.y);
		viewport->Draw();
		ImGui::Image((ImTextureID)(intptr_t)viewport->fboTexture, size,
			ImVec2(0, 1), ImVec2(1, 0)); // Flip vertically
		ImGui::End();
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(0.2f, 0.2f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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