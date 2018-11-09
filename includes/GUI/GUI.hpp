#pragma once

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

#include "header.hpp"
#include "nuklear/nuklear.h"
#include "nuklear_glfw_gl3.h"

// #define NK_INCLUDE_FIXED_TYPES
// #define NK_INCLUDE_STANDARD_IO
// #define NK_INCLUDE_STANDARD_VARARGS
// #define NK_INCLUDE_DEFAULT_ALLOCATOR
// #define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
// #define NK_INCLUDE_FONT_BAKING
// #define NK_INCLUDE_DEFAULT_FONT
// #define NK_KEYSTATE_BASED_INPUT
// #define NK_IMPLEMENTATION
// #define NK_GLFW_GL3_IMPLEMENTATION

class GUI {
public:
	GUI();
	GUI(GLFWwindow *window);
	~GUI();

	GUI &operator=(GUI const &rhs);

	void drawGUI();

	// Nuklear vars
	struct nk_context *ctx;
	struct nk_colorf bg;

};