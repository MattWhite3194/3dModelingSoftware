#ifndef VIEWPORT_CLASS_H
#define VIEWPORT_CLASS_H

#include <glad/glad.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "shader_s.h"
#include <string>
#include <vector>

class Viewport {
public:
	GLuint gridVao = 0, gridVbo = 0;
	const int GRID_SIZE = 20;
	Viewport() {
		InitGrid();
	}
	void InitGrid();
	void Draw(Shader& shader);
};

#endif
