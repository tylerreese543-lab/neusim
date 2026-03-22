#pragma once
#include <GLEW_2.1/include/glew.h>
#include <GLFW_3.4/include/GLFW/glfw3.h>

struct color3 {
	float r;
	float g;
	float b;
	color3(float r = 1.0f, float g = 1.0f, float b = 1.0f) : r(r), g(g), b(b) {}

	color3 mix(color3 other, float factor) {
		return color3(r + (other.r - r) * factor, g + (other.g - g) * factor, b + (other.b - b) * factor);
	}
};
void fillCircle(float x, float y, float radius, int segments = 36, color3 color = color3()) {
	glBegin(GL_TRIANGLE_FAN);
	glColor3f(color.r, color.g, color.b);
	glVertex2f(x, y);
	for (int i = 0; i <= segments; i++) {
		float angle = i * 2.0f * 3.1415926f / segments;
		float dx = cosf(angle) * radius;
		float dy = sinf(angle) * radius;
		glVertex2f(x + dx, y + dy);
	}
	glEnd();
};

void drawCircle(float x, float y, float radius, int segments = 36, color3 color = color3()) {
	glBegin(GL_LINE_LOOP);
	glColor3f(color.r, color.g, color.b);
	for (int i = 0; i < segments; i++) {
		float angle = i * 2.0f * 3.1415926f / segments;
		float dx = cosf(angle) * radius;
		float dy = sinf(angle) * radius;
		glVertex2f(x + dx, y + dy);
	}
	glEnd();
};

void drawLine(float x1, float y1, float x2, float y2, color3 color = color3()) {
	glBegin(GL_LINES);
	glColor3f(color.r, color.g, color.b);
	glVertex2f(x1, y1);
	glVertex2f(x2, y2);
	glEnd();
};

void fillRect(float x, float y, float width, float height, color3 color = color3()) {
	glBegin(GL_QUADS);
	glColor3f(color.r, color.g, color.b);
	glVertex2f(x, y);
	glVertex2f(x + width, y);
	glVertex2f(x + width, y + height);
	glVertex2f(x, y + height);
	glEnd();
};

void drawRect(float x, float y, float width, float height, color3 color = color3()) {
	glBegin(GL_LINE_LOOP);
	glColor3f(color.r, color.g, color.b);
	glVertex2f(x, y);
	glVertex2f(x + width, y);
	glVertex2f(x + width, y + height);
	glVertex2f(x, y + height);
	glEnd();
};