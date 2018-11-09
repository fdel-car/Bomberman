#pragma once

#include "header.hpp"

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

#include "nuklear/nuklear.h"
#include "nuklear_glfw_gl3.h"

// Custom Defines
#define WHITE_SHADER 1
#define GREEN_SHADER 2
#define RED_SHADER 3
#define CYAN_SHADER 4
#define YELLOW_SHADER 5
#define GRAY_SHADER 6

class GameLogic;

class GameRenderer
{
  public:
	static GameLogic *mainGame;

	GameRenderer(GameLogic *mainGame);
	~GameRenderer(void);

	void getUserInput(void);
	void refreshWindow(void);
	void closeWindow(void);

	bool active;

  private:
	GameRenderer(void);
	static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
	static void error_callback(int error, const char *description);

	GameRenderer(GameRenderer const &src);

	GameRenderer &operator=(GameRenderer const &rhs);

	void init_shaders(int type);
	void init_program(void);
	void create_border(void);
	void create_grid(void);
	void draw_gui(void);
	void draw_player(std::tuple<int, int> &player_pos);
	void make_vao(GLuint &vbo);

	// General vars
	GLFWwindow *window;
	int width = 0;
	int height = 0;
	int x_offset;
	int y_offset;
	int square_size;
	float start_x;
	float start_y;
	float square_percent_y;
	float square_percent_x;

	// Nuklear vars
	struct nk_context *ctx;
	struct nk_colorf bg;

	// Rendering vars
	GLuint vbo;
	GLuint vao;
	const char *vertex_shader;
	const char *fragment_shader;
	GLuint vs;
	GLuint fs;
	GLuint shader_program;
};
