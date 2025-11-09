#pragma once

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
#include "imgui_internal.h"

enum TransformTool {
	None = 0,
	Rotate = 1,
	Translate = 2,
	Scale = 3
};
class Viewport {
public:
	GLuint fbo = 0, fboTexture = 0, fboDepth = 0, gridVao = 0, gridVbo = 0;
	Shader* objectShader, * edgeShader, * gridShader;
	Camera* viewportCamera;
	Mesh* selectedMesh;
	/// <summary>
	/// For use when applying transforms with the mouse. Need to save the meshes original transform so it can be undone.
	/// </summary>
	glm::vec3 selectedTransform = glm::vec3(1.0f);
	float scaleStartDistance = 0.0f;
	std::unordered_map<int, TransformTool> transformKeyMappings;
	std::vector<std::unique_ptr<Mesh>> sceneMeshes;
	int viewportWidth = 1000, viewportHeight = 1000;
	glm::vec2 localCursorPos;
	glm::mat4 Projection;
	glm::vec3 viewportLight = glm::vec3(1.2f, 1.0f, 10.0f);
	glm::vec2 rotationStartPos;
	ImVec2 imguiWinPos = ImVec2(0.0f, 0.0f);
	ImVec2 imguiCurPos = ImVec2(0.0f, 0.0f);
	glm::vec3 transformAxis = glm::vec3(0.0f);
	const int GRID_SIZE = 20;
	double lastX, lastY;
	bool firstMouse = true;
	bool IsActive = false;
	bool forceMeshTab = false;
	bool firstScaleUpdate = false;
	bool firstRotationUpdate = false;
	bool ignoreNextMouseDelta = false;
	float accumulatedRotation = 0.0;
	TransformTool ActiveTool = None;
	std::string transformVisualText;
	glm::vec3 cursor3D = glm::vec3(0.0f);

	Viewport() {
		InitGrid();
		Init();
	}

	void InitGrid();

	void Init();

	void CreateViewportFramebuffer();


	void ResizeViewportFramebuffer(int width, int height);

	void Draw();

	void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);

	void scroll_callback(GLFWwindow* window, double xpos, double ypos);

	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

	void AddMesh(std::unique_ptr<Mesh> mesh);

	void DeleteMesh(Mesh* mesh);

	void DuplicateMesh(Mesh* mesh);

	void SetSelected(Mesh* mesh);

	void SetActiveTool(GLFWwindow* window, TransformTool activeTool, bool undoCurrent = true);

	void UndoTransform();

	glm::vec3 ScreenDeltaToWorldDelta(
		float mouseX, float mouseY,
		float dx, float dy,
		const glm::mat4& view,
		const glm::mat4& proj,
		const glm::vec4& viewport,
		const glm::vec3& objectPosWorld
	);

	bool PickMesh(Mesh& mesh,
		glm::vec3 rayOrigin,
		glm::vec3 rayDir,
		float& outDist,
		Face*& outFace
	);

	bool RayTriangle(const glm::vec3& orig, const glm::vec3& dir,
		const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
		float& tOut
	);
};
