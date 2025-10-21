#pragma once

struct Color {
	float r, g, b, a;
	Color(int r, int g, int b, float a) {
		this->r = r / 255.0f;
		this->g = g / 255.0f;
		this->b = b / 255.0f;
		this->a = a;
	}
	Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {

	}

};