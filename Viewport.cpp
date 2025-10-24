#include "Viewport.h"

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

void Viewport::Draw(Shader& shader) {
	shader.use();
	glBindVertexArray(gridVao);
	glLineWidth(2.0f);
	glDrawArrays(GL_LINES, 0, (GRID_SIZE + 1) * 4);
	glBindVertexArray(0);
}