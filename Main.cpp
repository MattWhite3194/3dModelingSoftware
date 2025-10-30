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
#include "imgui_theme.h"
#include "imgui_internal.h"

//Local includes
#include "ObjectPrimitives.h"
#include "Viewport.h"

Viewport* viewport;
ImGuiWindowFlags host_flags =
ImGuiWindowFlags_NoTitleBar |
ImGuiWindowFlags_NoCollapse |
ImGuiWindowFlags_NoResize |
ImGuiWindowFlags_NoMove |
ImGuiWindowFlags_NoBringToFrontOnFocus |
ImGuiWindowFlags_NoNavFocus |
ImGuiWindowFlags_NoBackground | // remove gray background
ImGuiWindowFlags_NoScrollWithMouse | // stop scroll detection inside dockspace
ImGuiWindowFlags_NoScrollbar;

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
	if (ImGui::GetIO().WantCaptureMouse && !viewport->IsActive) {
		return;
	}
	viewport->cursor_pos_callback(window, xpos, ypos);
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

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	if (ImGui::GetIO().WantCaptureKeyboard && !viewport->IsActive) {
		return;
	}
	viewport->key_callback(window, key, scancode, action, mods);
}

void DrawToolWindow() {
	ImGui::Begin("Tools");

	//export drawing of tabs to viewport
	if (ImGui::BeginTabBar("Mesh Tools"))
	{
		if (viewport->selectedMesh) {
			//Mesh transformations tab
			ImGuiTabItemFlags meshFlags = viewport->forceMeshTab ? ImGuiTabItemFlags_SetSelected : 0;
			if (ImGui::BeginTabItem("Mesh", nullptr, meshFlags)) {
				viewport->forceMeshTab = false;
				bool changed = false;
				//GLobal transformations
				ImGui::Text("Translation:");
				ImGui::PushItemWidth(-1);
				changed |= ImGui::DragFloat("##01", &viewport->selectedMesh->Translation.x, 0.1f, 0.0f, 0.0f, "X:\t%.4f");
				changed |= ImGui::DragFloat("##02", &viewport->selectedMesh->Translation.y, 0.1f, 0.0f, 0.0f, "Y:\t%.4f");
				changed |= ImGui::DragFloat("##03", &viewport->selectedMesh->Translation.z, 0.1f, 0.0f, 0.0f, "Z:\t%.4f");
				ImGui::PopItemWidth();
				ImGui::Text("Rotation:");
				ImGui::PushItemWidth(-1);
				changed |= ImGui::DragFloat("##04", &viewport->selectedMesh->Rotation.x, 0.1f, 0.0f, 0.0f, "X:\t%.4f");
				changed |= ImGui::DragFloat("##05", &viewport->selectedMesh->Rotation.y, 0.1f, 0.0f, 0.0f, "Y:\t%.4f");
				changed |= ImGui::DragFloat("##06", &viewport->selectedMesh->Rotation.z, 0.1f, 0.0f, 0.0f, "Z:\t%.4f");
				ImGui::PopItemWidth();
				ImGui::Text("Scale:");
				ImGui::PushItemWidth(-1);
				changed |= ImGui::DragFloat("##07", &viewport->selectedMesh->Scale.x, 0.1f, 0.0f, 0.0f, "X:\t%.4f");
				changed |= ImGui::DragFloat("##08", &viewport->selectedMesh->Scale.y, 0.1f, 0.0f, 0.0f, "Y:\t%.4f");
				changed |= ImGui::DragFloat("##09", &viewport->selectedMesh->Scale.z, 0.1f, 0.0f, 0.0f, "Z:\t%.4f");
				ImGui::PopItemWidth();
				if (changed) {
					viewport->selectedMesh->transformDirty = true;
				}

				if (ImGui::Checkbox("Flat Shading", &viewport->selectedMesh->flatShading)) {
					viewport->selectedMesh->gpuDirty = !viewport->selectedMesh->gpuDirty;
				}
				ImGui::ColorEdit4("Object Color", glm::value_ptr(viewport->selectedMesh->ObjectColor), ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_DisplayHex);
				ImGui::EndTabItem();
			}
			//mesh modifiers tab
			if (ImGui::BeginTabItem("Modify")) {
				ImGui::Button("Flip Normals");
				ImGui::Button("Subdivide");
				ImGui::Button("Duplicate");
				ImGui::Button("Delete");
				ImGui::EndTabItem();
			}
		}
		if (ImGui::BeginTabItem("Create")) {
			if (ImGui::Button("New Cube")) {
				viewport->AddMesh(CreateCube(1.0f));
			}
			if (ImGui::Button("New Cylinder")) {
				viewport->AddMesh(CreateCylinder(16, 1.0f, 2.0f));
			}
			if (ImGui::Button("New Cone")) {
				viewport->AddMesh(CreateCone(16, 1.0f, 2.0f));
			}
			if (ImGui::Button("New Circle")) {
				viewport->AddMesh(CreateCircle(16, 1.0f));
			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
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

	//initialize glad
	gladLoadGL();

	//Set up Imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

	//Load ImGui font
	io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/Arial.ttf", 8.0f);
	io.FontDefault = io.Fonts->Fonts.back();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init("#version 130");

	//process loop
	glfwMakeContextCurrent(window);

	//initiate viewport
	viewport = new Viewport();
	viewport->CreateViewportFramebuffer();
	//set button callbacks
	glfwSetCursorPosCallback(window, cursor_pos_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetKeyCallback(window, key_callback);

	SetupImGuiStyle();
	bool first_time = true;
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuiID dockspace_id = ImGui::GetID("RootDockspace");

		if (first_time)
		{
			first_time = false;

			ImGui::DockBuilderRemoveNode(dockspace_id);
			ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_None);

			// Split: bottom gets 25% height
			ImGuiID dock_main = dockspace_id;
			ImGuiID dock_right;
			ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Right, 0.25f, &dock_right, &dock_main);

			// Assign windows
			ImGui::DockBuilderDockWindow("Viewport", dock_main);
			ImGui::DockBuilderDockWindow("Tools", dock_right);

			ImGui::DockBuilderFinish(dockspace_id);
		}
		const ImGuiViewport* viewport_imgui = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport_imgui->WorkPos);
		ImGui::SetNextWindowSize(viewport_imgui->WorkSize);
		ImGui::SetNextWindowViewport(viewport_imgui->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("RootDockspaceWindow", nullptr, host_flags);
		ImGui::PopStyleVar(3);

		ImGui::DockSpace(dockspace_id, ImVec2(0, 0),
			ImGuiDockNodeFlags_AutoHideTabBar | ImGuiDockNodeFlags_NoDockingInCentralNode | ImGuiDockNodeFlags_NoTabBar);

		ImGui::BeginMainMenuBar();
		ImGui::MenuItem("File");
		ImGui::EndMainMenuBar();
		ImGui::End();

		ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
		
		ImVec2 size = ImGui::GetContentRegionAvail();
		viewport->ResizeViewportFramebuffer(size.x, size.y);
		viewport->Draw();
		ImGui::Image((ImTextureID)(intptr_t)viewport->fboTexture, size,
			ImVec2(0, 1), ImVec2(1, 0)); // Flip vertically

		//END MAIN WINDOW DRAW
		ImGui::End();

		DrawToolWindow();

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