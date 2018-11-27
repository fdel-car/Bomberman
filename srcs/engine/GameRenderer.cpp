#include "engine/GameRenderer.hpp"
#include "engine/AGame.hpp"
#include "engine/Entity.hpp"
#include "engine/GameEngine.hpp"
#include "engine/Model.hpp"

extern std::string _srcsDir;
extern std::string _assetsDir;

GameRenderer::GameRenderer(GameEngine *gameEngine, AGame *game) {
	_gameEngine = gameEngine;
	glfwSetErrorCallback(errorCallback);
	if (!glfwInit()) throw new std::runtime_error("Failed to initialize GLFW");
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_SAMPLES, 4);
	_window = glfwCreateWindow(WINDOW_W, WINDOW_H, "Bomberman", NULL,
							   NULL);  // Size of screen will change
	if (!_window) {
		glfwTerminate();
		throw new std::runtime_error("Failed to create windows GLFW");
	}
	const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwSetWindowPos(_window, (mode->width / 2) - (WINDOW_W / 2),
					 (mode->height / 2) - (WINDOW_H / 2));
	glfwMakeContextCurrent(_window);
	glfwGetWindowSize(_window, &_width, &_height);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		throw new std::runtime_error("Failed to initialize GLAD");
	glViewport(0, 0, WINDOW_W, WINDOW_H);
	glfwSetKeyCallback(_window, keyCallback);
	glfwSetCursorPosCallback(_window, mouseCallback);

	_initGUI(game);
	_initModels();
	_initDepthMap();  // TODO Check if the Framebuffer was create correctly
	_initShader();
}

GameRenderer::~GameRenderer(void) {
	delete graphicUI;
	for (auto model : _models) delete model.second;
	delete _shaderProgram;
	_models.clear();
	if (_window) glfwDestroyWindow(_window);
	glfwTerminate();
	return;
}

void GameRenderer::_initGUI(AGame *game) {
	std::vector<std::tuple<float, std::string, std::string>> vFontPath =
		game->getNeededFont();
	graphicUI = new GUI(_window, vFontPath);
}

bool GameRenderer::_initDepthMap(void) {
	// Create framebuffer object for rendering the depth map
	glGenFramebuffers(1, &_depthMapFBO);

	// Create a 2D texture that we'll use as the framebuffer's depth buffer
	glGenTextures(1, &_depthMap);
	glBindTexture(GL_TEXTURE_2D, _depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_W, SHADOW_H, 0,
				 GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// Attach depth texture as the framebuffer's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, _depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
						   _depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Always check that our framebuffer is ok
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Failed to initialize Depth Map." << std::endl;
		return false;
	}
	return true;
}

void GameRenderer::_initShader(void) {
	_shaderProgram =
		new ShaderProgram(_srcsDir + "engine/shaders/4.1.vs",
							_srcsDir + "engine/shaders/4.1.fs");
	_shadowShaderProgram =
		new ShaderProgram(_srcsDir + "engine/shaders/depthMap.vs",
						  _srcsDir + "engine/shaders/depthMap.fs");

	_skyboxShaderProgram =
		new ShaderProgram(_srcsDir + "engine/shaders/skybox.vs",
						  _srcsDir + "engine/shaders/skybox.fs");

	glUseProgram(_shaderProgram->getID());
	_shaderProgram->setInt("shadowMap", 0);

	glUseProgram(_shaderProgram->getID());
	_shaderProgram->setInt("diffuseTexture", 1);

	glUseProgram(_skyboxShaderProgram->getID());
	_skyboxShaderProgram->setInt("skybox", 2);
}

void GameRenderer::_initModels(void) {
	_models["Box"] = new Model("box");
	_models["Explosion"] = new Model("explosion");
	_models["Wall"] = new Model("wall");
	// _models["Light"] = new Model("light");
	_models["Player"] = new Model("player");
	_models["Bomb"] = new Model("bomb");
	_models["Enemy"] = new Model("enemy");
	_models["Island"] = new Model("island");
}

void GameRenderer::getUserInput(void) { glfwPollEvents(); }

void GameRenderer::refreshWindow(std::vector<Entity *> &entities,
								 Camera *camera, Light *light, Skybox *skybox) {
	glfwSetWindowTitle(_window,
					   toString(1.0f / _gameEngine->getDeltaTime()).c_str());
	_lightSpaceMatrix = light->getProjectionMatrix() * light->getViewMatrix();

	// Custom OpenGL state
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Shadow map
	glUseProgram(_shadowShaderProgram->getID());
	glViewport(0, 0, SHADOW_W, SHADOW_H);
	glBindFramebuffer(GL_FRAMEBUFFER, _depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	_shadowShaderProgram->setMat4("lightSpaceMatrix", _lightSpaceMatrix);
	glCullFace(GL_FRONT);
	for (auto entity : entities) {
		_shadowShaderProgram->setMat4("M", entity->getModelMatrix());
		entity->getModel()->draw(*_shadowShaderProgram);
	}
	glCullFace(GL_BACK);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Basic rendering OpenGL state
	glViewport(0, 0, WINDOW_W, WINDOW_H);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(_shaderProgram->getID());
	_shaderProgram->setMat4("V", camera->getViewMatrix());
	_shaderProgram->setMat4("P", camera->getProjectionMatrix());
	_shaderProgram->setMat4("lightSpaceMatrix", _lightSpaceMatrix);
	_shaderProgram->setVec3("lightDir", light->getDir());
	_shaderProgram->setVec3("viewPos", camera->getPosition());
	_shaderProgram->setVec3("lightColor", light->getColor());
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _depthMap);
	for (auto entity : entities) {
		_shaderProgram->setMat4("M", entity->getModelMatrix());
		entity->getModel()->draw(*_shaderProgram);
	}

	// Skybox
	glDepthFunc(GL_LEQUAL);
	glUseProgram(_skyboxShaderProgram->getID());
	
	_skyboxShaderProgram->setMat4("view", glm::mat4(glm::mat3(camera->getViewMatrix())));
	_skyboxShaderProgram->setMat4("projection", camera->getProjectionMatrix());
	
	glBindVertexArray(skybox->getVAO());
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->getTexture());
	_skyboxShaderProgram->setInt("skybox", 2);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS); // set depth function back to default

	// Default OpenGL state
	glUseProgram(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_MULTISAMPLE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	graphicUI->nkNewFrame();
	camera->drawGUI(graphicUI);
	graphicUI->nkRender();

	// Put everything to screen
	glfwSwapBuffers(_window);
}

Model *GameRenderer::getModel(std::string modelName) const {
	if (_models.find(modelName) != _models.end()) return _models.at(modelName);
	return nullptr;
}

GUI *GameRenderer::getGUI() { return graphicUI; }

int GameRenderer::getWidth(void) const { return _width; }

int GameRenderer::getHeight(void) const { return _height; }

void GameRenderer::errorCallback(int error, const char *description) {
	std::cerr << "Error n." << error << ": " << description << std::endl;
}

void GameRenderer::switchCursorMode(bool debug) const {
	glfwSetInputMode(_window, GLFW_CURSOR,
					 debug ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void GameRenderer::mouseCallback(GLFWwindow *window, double xPos, double yPos) {
	_mousePos.x = xPos;
	_mousePos.y = yPos;
	(void)window;
}

glm::vec2 GameRenderer::_mousePos = glm::vec2(WINDOW_W / 2, WINDOW_H / 2);

glm::vec2 GameRenderer::getMousePos(void) const { return _mousePos; }

void GameRenderer::keyCallback(GLFWwindow *window, int key, int scancode,
							   int action, int mods) {
	if (action == GLFW_PRESS || action == GLFW_RELEASE) {
		switch (key) {
			case GLFW_KEY_UNKNOWN:
				_gameEngine->buttonStateChanged("UNKNOWN",
												action == GLFW_PRESS);
				break;
			case GLFW_KEY_SPACE:
				_gameEngine->buttonStateChanged("SPACE", action == GLFW_PRESS);
				break;
			case GLFW_KEY_APOSTROPHE:
				_gameEngine->buttonStateChanged("APOSTROPHE",
												action == GLFW_PRESS);
				break;
			case GLFW_KEY_COMMA:
				_gameEngine->buttonStateChanged("COMMA", action == GLFW_PRESS);
				break;
			case GLFW_KEY_MINUS:
				_gameEngine->buttonStateChanged("MINUS", action == GLFW_PRESS);
				break;
			case GLFW_KEY_PERIOD:
				_gameEngine->buttonStateChanged("PERIOD", action == GLFW_PRESS);
				break;
			case GLFW_KEY_SLASH:
				_gameEngine->buttonStateChanged("SLASH", action == GLFW_PRESS);
				break;
			case GLFW_KEY_0:
				_gameEngine->buttonStateChanged("0", action == GLFW_PRESS);
				break;
			case GLFW_KEY_1:
				_gameEngine->buttonStateChanged("1", action == GLFW_PRESS);
				break;
			case GLFW_KEY_2:
				_gameEngine->buttonStateChanged("2", action == GLFW_PRESS);
				break;
			case GLFW_KEY_3:
				_gameEngine->buttonStateChanged("3", action == GLFW_PRESS);
				break;
			case GLFW_KEY_4:
				_gameEngine->buttonStateChanged("4", action == GLFW_PRESS);
				break;
			case GLFW_KEY_5:
				_gameEngine->buttonStateChanged("5", action == GLFW_PRESS);
				break;
			case GLFW_KEY_6:
				_gameEngine->buttonStateChanged("6", action == GLFW_PRESS);
				break;
			case GLFW_KEY_7:
				_gameEngine->buttonStateChanged("7", action == GLFW_PRESS);
				break;
			case GLFW_KEY_8:
				_gameEngine->buttonStateChanged("8", action == GLFW_PRESS);
				break;
			case GLFW_KEY_9:
				_gameEngine->buttonStateChanged("9", action == GLFW_PRESS);
				break;
			case GLFW_KEY_SEMICOLON:
				_gameEngine->buttonStateChanged(";", action == GLFW_PRESS);
				break;
			case GLFW_KEY_EQUAL:
				_gameEngine->buttonStateChanged("EQUAL", action == GLFW_PRESS);
				break;
			case GLFW_KEY_A:
				_gameEngine->buttonStateChanged("A", action == GLFW_PRESS);
				break;
			case GLFW_KEY_B:
				_gameEngine->buttonStateChanged("B", action == GLFW_PRESS);
				break;
			case GLFW_KEY_C:
				_gameEngine->buttonStateChanged("C", action == GLFW_PRESS);
				break;
			case GLFW_KEY_D:
				_gameEngine->buttonStateChanged("D", action == GLFW_PRESS);
				break;
			case GLFW_KEY_E:
				_gameEngine->buttonStateChanged("E", action == GLFW_PRESS);
				break;
			case GLFW_KEY_F:
				_gameEngine->buttonStateChanged("F", action == GLFW_PRESS);
				break;
			case GLFW_KEY_G:
				_gameEngine->buttonStateChanged("G", action == GLFW_PRESS);
				break;
			case GLFW_KEY_H:
				_gameEngine->buttonStateChanged("H", action == GLFW_PRESS);
				break;
			case GLFW_KEY_I:
				_gameEngine->buttonStateChanged("I", action == GLFW_PRESS);
				break;
			case GLFW_KEY_J:
				_gameEngine->buttonStateChanged("L", action == GLFW_PRESS);
				break;
			case GLFW_KEY_K:
				_gameEngine->buttonStateChanged("K", action == GLFW_PRESS);
				break;
			case GLFW_KEY_L:
				_gameEngine->buttonStateChanged("L", action == GLFW_PRESS);
				break;
			case GLFW_KEY_M:
				_gameEngine->buttonStateChanged("M", action == GLFW_PRESS);
				break;
			case GLFW_KEY_N:
				_gameEngine->buttonStateChanged("N", action == GLFW_PRESS);
				break;
			case GLFW_KEY_O:
				_gameEngine->buttonStateChanged("O", action == GLFW_PRESS);
				break;
			case GLFW_KEY_P:
				_gameEngine->buttonStateChanged("P", action == GLFW_PRESS);
				break;
			case GLFW_KEY_Q:
				_gameEngine->buttonStateChanged("Q", action == GLFW_PRESS);
				break;
			case GLFW_KEY_R:
				_gameEngine->buttonStateChanged("R", action == GLFW_PRESS);
				break;
			case GLFW_KEY_S:
				_gameEngine->buttonStateChanged("S", action == GLFW_PRESS);
				break;
			case GLFW_KEY_T:
				_gameEngine->buttonStateChanged("T", action == GLFW_PRESS);
				break;
			case GLFW_KEY_U:
				_gameEngine->buttonStateChanged("U", action == GLFW_PRESS);
				break;
			case GLFW_KEY_V:
				_gameEngine->buttonStateChanged("V", action == GLFW_PRESS);
				break;
			case GLFW_KEY_W:
				_gameEngine->buttonStateChanged("W", action == GLFW_PRESS);
				break;
			case GLFW_KEY_X:
				_gameEngine->buttonStateChanged("X", action == GLFW_PRESS);
				break;
			case GLFW_KEY_Y:
				_gameEngine->buttonStateChanged("Y", action == GLFW_PRESS);
				break;
			case GLFW_KEY_Z:
				_gameEngine->buttonStateChanged("Z", action == GLFW_PRESS);
				break;
			case GLFW_KEY_LEFT_BRACKET:
				_gameEngine->buttonStateChanged("[", action == GLFW_PRESS);
				break;
			case GLFW_KEY_BACKSLASH:
				_gameEngine->buttonStateChanged("\\", action == GLFW_PRESS);
				break;
			case GLFW_KEY_RIGHT_BRACKET:
				_gameEngine->buttonStateChanged("]", action == GLFW_PRESS);
				break;
			case GLFW_KEY_GRAVE_ACCENT:
				_gameEngine->buttonStateChanged("`", action == GLFW_PRESS);
				break;
			case GLFW_KEY_WORLD_1:
				_gameEngine->buttonStateChanged("WORLD_1",
												action == GLFW_PRESS);
				break;
			case GLFW_KEY_WORLD_2:
				_gameEngine->buttonStateChanged("WORLD_2",
												action == GLFW_PRESS);
				break;
			case GLFW_KEY_ESCAPE:
				_gameEngine->buttonStateChanged("ESCAPE", action == GLFW_PRESS);
				break;
			case GLFW_KEY_ENTER:
				_gameEngine->buttonStateChanged("ENTER", action == GLFW_PRESS);
				break;
			case GLFW_KEY_TAB:
				_gameEngine->buttonStateChanged("TAB", action == GLFW_PRESS);
				break;
			case GLFW_KEY_BACKSPACE:
				_gameEngine->buttonStateChanged("BACKSPACE",
												action == GLFW_PRESS);
				break;
			case GLFW_KEY_INSERT:
				_gameEngine->buttonStateChanged("INSERT", action == GLFW_PRESS);
				break;
			case GLFW_KEY_DELETE:
				_gameEngine->buttonStateChanged("DELETE", action == GLFW_PRESS);
				break;
			case GLFW_KEY_RIGHT:
				_gameEngine->buttonStateChanged("RIGHT", action == GLFW_PRESS);
				break;
			case GLFW_KEY_LEFT:
				_gameEngine->buttonStateChanged("LEFT", action == GLFW_PRESS);
				break;
			case GLFW_KEY_DOWN:
				_gameEngine->buttonStateChanged("DOWN", action == GLFW_PRESS);
				break;
			case GLFW_KEY_UP:
				_gameEngine->buttonStateChanged("UP", action == GLFW_PRESS);
				break;
			case GLFW_KEY_PAGE_UP:
				_gameEngine->buttonStateChanged("PAGE_UP",
												action == GLFW_PRESS);
				break;
			case GLFW_KEY_PAGE_DOWN:
				_gameEngine->buttonStateChanged("PAGE_DOWN",
												action == GLFW_PRESS);
				break;
			case GLFW_KEY_HOME:
				_gameEngine->buttonStateChanged("HOME", action == GLFW_PRESS);
				break;
			case GLFW_KEY_END:
				_gameEngine->buttonStateChanged("END", action == GLFW_PRESS);
				break;
			case GLFW_KEY_CAPS_LOCK:
				_gameEngine->buttonStateChanged("CAPS_LOCK",
												action == GLFW_PRESS);
				break;
			case GLFW_KEY_SCROLL_LOCK:
				_gameEngine->buttonStateChanged("SCROLL_LOCK",
												action == GLFW_PRESS);
				break;
			case GLFW_KEY_NUM_LOCK:
				_gameEngine->buttonStateChanged("NUM_LOCK",
												action == GLFW_PRESS);
				break;
			case GLFW_KEY_PRINT_SCREEN:
				_gameEngine->buttonStateChanged("PRINT_SCREEN",
												action == GLFW_PRESS);
				break;
			case GLFW_KEY_PAUSE:
				_gameEngine->buttonStateChanged("PAUSE", action == GLFW_PRESS);
				break;
			case GLFW_KEY_F1:
				_gameEngine->buttonStateChanged("F1", action == GLFW_PRESS);
				break;
			case GLFW_KEY_F2:
				_gameEngine->buttonStateChanged("F2", action == GLFW_PRESS);
				break;
			case GLFW_KEY_F3:
				_gameEngine->buttonStateChanged("F3", action == GLFW_PRESS);
				break;
			case GLFW_KEY_F4:
				_gameEngine->buttonStateChanged("F4", action == GLFW_PRESS);
				break;
			case GLFW_KEY_F5:
				_gameEngine->buttonStateChanged("F5", action == GLFW_PRESS);
				break;
			case GLFW_KEY_F6:
				_gameEngine->buttonStateChanged("F6", action == GLFW_PRESS);
				break;
			case GLFW_KEY_F7:
				_gameEngine->buttonStateChanged("F7", action == GLFW_PRESS);
				break;
			case GLFW_KEY_F8:
				_gameEngine->buttonStateChanged("F8", action == GLFW_PRESS);
				break;
			case GLFW_KEY_F9:
				_gameEngine->buttonStateChanged("F9", action == GLFW_PRESS);
				break;
			case GLFW_KEY_F10:
				_gameEngine->buttonStateChanged("F10", action == GLFW_PRESS);
				break;
			case GLFW_KEY_F11:
				_gameEngine->buttonStateChanged("F11", action == GLFW_PRESS);
				break;
			case GLFW_KEY_F12:
				_gameEngine->buttonStateChanged("F12", action == GLFW_PRESS);
				break;
			case GLFW_KEY_F13:
				_gameEngine->buttonStateChanged("F13", action == GLFW_PRESS);
				break;
			case GLFW_KEY_F14:
				_gameEngine->buttonStateChanged("F14", action == GLFW_PRESS);
				break;
			case GLFW_KEY_F15:
				_gameEngine->buttonStateChanged("F15", action == GLFW_PRESS);
				break;
			case GLFW_KEY_F16:
				_gameEngine->buttonStateChanged("F16", action == GLFW_PRESS);
				break;
			case GLFW_KEY_F17:
				_gameEngine->buttonStateChanged("F17", action == GLFW_PRESS);
				break;
			case GLFW_KEY_F18:
				_gameEngine->buttonStateChanged("F18", action == GLFW_PRESS);
				break;
			case GLFW_KEY_F19:
				_gameEngine->buttonStateChanged("F19", action == GLFW_PRESS);
				break;
			case GLFW_KEY_F20:
				_gameEngine->buttonStateChanged("F20", action == GLFW_PRESS);
				break;
			case GLFW_KEY_F21:
				_gameEngine->buttonStateChanged("F21", action == GLFW_PRESS);
				break;
			case GLFW_KEY_F22:
				_gameEngine->buttonStateChanged("F22", action == GLFW_PRESS);
				break;
			case GLFW_KEY_F23:
				_gameEngine->buttonStateChanged("F23", action == GLFW_PRESS);
				break;
			case GLFW_KEY_F24:
				_gameEngine->buttonStateChanged("F24", action == GLFW_PRESS);
				break;
			case GLFW_KEY_F25:
				_gameEngine->buttonStateChanged("F25", action == GLFW_PRESS);
				break;
			case GLFW_KEY_KP_0:
				_gameEngine->buttonStateChanged("KP_0", action == GLFW_PRESS);
				break;
			case GLFW_KEY_KP_1:
				_gameEngine->buttonStateChanged("KP_1", action == GLFW_PRESS);
				break;
			case GLFW_KEY_KP_2:
				_gameEngine->buttonStateChanged("KP_2", action == GLFW_PRESS);
				break;
			case GLFW_KEY_KP_3:
				_gameEngine->buttonStateChanged("KP_3", action == GLFW_PRESS);
				break;
			case GLFW_KEY_KP_4:
				_gameEngine->buttonStateChanged("KP_4", action == GLFW_PRESS);
				break;
			case GLFW_KEY_KP_5:
				_gameEngine->buttonStateChanged("KP_5", action == GLFW_PRESS);
				break;
			case GLFW_KEY_KP_6:
				_gameEngine->buttonStateChanged("KP_6", action == GLFW_PRESS);
				break;
			case GLFW_KEY_KP_7:
				_gameEngine->buttonStateChanged("KP_7", action == GLFW_PRESS);
				break;
			case GLFW_KEY_KP_8:
				_gameEngine->buttonStateChanged("KP_8", action == GLFW_PRESS);
				break;
			case GLFW_KEY_KP_9:
				_gameEngine->buttonStateChanged("KP_9", action == GLFW_PRESS);
				break;
			case GLFW_KEY_KP_DECIMAL:
				_gameEngine->buttonStateChanged("KP_DECIMAL",
												action == GLFW_PRESS);
				break;
			case GLFW_KEY_KP_DIVIDE:
				_gameEngine->buttonStateChanged("KP_DIVIDE",
												action == GLFW_PRESS);
				break;
			case GLFW_KEY_KP_MULTIPLY:
				_gameEngine->buttonStateChanged("KP_MULTIPLY",
												action == GLFW_PRESS);
				break;
			case GLFW_KEY_KP_SUBTRACT:
				_gameEngine->buttonStateChanged("KP_SUBTRACT",
												action == GLFW_PRESS);
				break;
			case GLFW_KEY_KP_ADD:
				_gameEngine->buttonStateChanged("KP_ADD", action == GLFW_PRESS);
				break;
			case GLFW_KEY_KP_ENTER:
				_gameEngine->buttonStateChanged("KP_ENTER",
												action == GLFW_PRESS);
				break;
			case GLFW_KEY_KP_EQUAL:
				_gameEngine->buttonStateChanged("KP_EQUAL",
												action == GLFW_PRESS);
				break;
			case GLFW_KEY_LEFT_SHIFT:
				_gameEngine->buttonStateChanged("LEFT_SHIFT",
												action == GLFW_PRESS);
				break;
			case GLFW_KEY_LEFT_CONTROL:
				_gameEngine->buttonStateChanged("LEFT_CONTROL",
												action == GLFW_PRESS);
				break;
			case GLFW_KEY_LEFT_ALT:
				_gameEngine->buttonStateChanged("LEFT_ALT",
												action == GLFW_PRESS);
				break;
			case GLFW_KEY_LEFT_SUPER:
				_gameEngine->buttonStateChanged("LEFT_SUPER",
												action == GLFW_PRESS);
				break;
			case GLFW_KEY_RIGHT_SHIFT:
				_gameEngine->buttonStateChanged("RIGHT_SHIFT",
												action == GLFW_PRESS);
				break;
			case GLFW_KEY_RIGHT_CONTROL:
				_gameEngine->buttonStateChanged("RIGHT_CONTROL",
												action == GLFW_PRESS);
				break;
			case GLFW_KEY_RIGHT_ALT:
				_gameEngine->buttonStateChanged("RIGHT_ALT",
												action == GLFW_PRESS);
				break;
			case GLFW_KEY_RIGHT_SUPER:
				_gameEngine->buttonStateChanged("RIGHT_SUPER",
												action == GLFW_PRESS);
				break;
			case GLFW_KEY_MENU:
				_gameEngine->buttonStateChanged("MENU", action == GLFW_PRESS);
				break;
		}
	}
	(void)scancode;
	(void)window;
	(void)mods;
}

GameEngine *GameRenderer::_gameEngine = NULL;
