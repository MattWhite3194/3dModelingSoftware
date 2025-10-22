#ifndef MODEL_CLASS_H
#define MODEL_CLASS_H

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include "Texture.h"

class Model {
public:
	int ID;
	Texture* texture;
	glm::mat4 scale;
	glm::mat4 rotation;
	glm::mat4 translation;
	glm::vec4 position;
	glm::vec4 origin;

	GLfloat* transformed_vertices;

private:
	GLfloat* vertices;
	GLuint* inidices;
};

#endif
