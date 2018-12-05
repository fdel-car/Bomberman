#include "engine/GameRenderer.hpp"
#include "engine/AGame.hpp"
#include "engine/Entity.hpp"
#include "engine/GameEngine.hpp"
#include "engine/Model.hpp"

extern std::string _srcsDir;
// extern std::string _assetsDir;

GameRenderer::GameRenderer(GameEngine *gameEngine, AGame *game) {
	_gameEngine = gameEngine;
	glfwSetErrorCallback(errorCallback);
	if (!glfwInit()) throw std::runtime_error("Failed to initialize GLFW");
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_FOCUSED, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	_window = glfwCreateWindow(WINDOW_W, WINDOW_H, "Bomberman", NULL,
							   NULL);  // Size of screen will change
	if (!_window) {
		glfwTerminate();
		throw std::runtime_error("Failed to create windows GLFW");
	}
	const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwSetWindowPos(_window, (mode->width / 2) - (WINDOW_W / 2),
					 (mode->height / 2) - (WINDOW_H / 2));
	glfwGetWindowSize(_window, &_width, &_height);
	glfwMakeContextCurrent(_window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		throw std::runtime_error("Failed to initialize GLAD");
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
		std::cerr << "Failed to initialize Depth Map." << std::endl;
		return false;
	}
	return true;
}

void GameRenderer::_initShader(void) {
	_shaderProgram = new ShaderProgram(_srcsDir + "engine/shaders/4.1.vs",
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
	_models["Sphere"] = new Model("models/sphere/sphere.dae");
	_models["Wall"] = new Model("models/wall/wall.dae");
	// _models["Light"] = new Model("light");
	// _models["Player"] = new Model("player");
	_models["Bomb"] = new Model("models/bomb/bomb.dae");
	// _models["Enemy"] = new Model("enemy");
	// _models["Box"] = new Model("box");
	_models["Island"] = new Model("models/island/island.obj");
	_models["Player"] = new Model("models/player/player.dae");
}

void GameRenderer::getUserInput(void) { glfwPollEvents(); }

void GameRenderer::refreshWindow(std::vector<Entity *> &entities,
								 Camera *camera, Light *light, Skybox *skybox) {
	glfwSetWindowTitle(_window,
					   toString(1.0f / _gameEngine->getDeltaTime())
						   .c_str());  // TODO: Don't forget to remove this

	// Custom OpenGL state
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (light != nullptr) {
		_lightSpaceMatrix =
			light->getProjectionMatrix() * light->getViewMatrix();
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
	}

	// Basic rendering OpenGL state
	glViewport(0, 0, WINDOW_W, WINDOW_H);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(_shaderProgram->getID());
	_shaderProgram->setMat4("V", camera->getViewMatrix());
	_shaderProgram->setMat4("P", camera->getProjectionMatrix());
	_shaderProgram->setVec3("viewPos", camera->getPosition());
	if (light == nullptr) {
		_shaderProgram->setVec3("lightDir",
								glm::normalize(glm::vec3(0.5, -0.5, -0.5)));
		_shaderProgram->setVec3("lightColor", glm::vec3(1.0f));
		_shaderProgram->setMat4("lightSpaceMatrix", glm::mat4(1.0f));
	} else {
		_shaderProgram->setVec3("lightDir", light->getDir());
		_shaderProgram->setVec3("lightColor", light->getColor());
		_shaderProgram->setMat4("lightSpaceMatrix", _lightSpaceMatrix);
	}
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _depthMap);
	for (auto entity : entities) {
		_shaderProgram->setMat4("M", entity->getModelMatrix());
		entity->getModel()->draw(*_shaderProgram, entity->getColor());
	}

	if (skybox != nullptr) {
		// Skybox
		glDepthFunc(GL_LEQUAL);
		glUseProgram(_skyboxShaderProgram->getID());

		_skyboxShaderProgram->setMat4(
			"view", glm::mat4(glm::mat3(camera->getViewMatrix())));
		_skyboxShaderProgram->setMat4("projection",
									  camera->getProjectionMatrix());

		glBindVertexArray(skybox->getVAO());
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->getTexture());
		_skyboxShaderProgram->setInt("skybox", 2);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);  // Set depth function back to default
	}

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
	if (!modelName.empty())
		std::cerr << "\033[0;33m:Warning:\033[0m " << modelName
				  << " not found inside the models map." << std::endl;
	return nullptr;
}

GUI *GameRenderer::getGUI() { return graphicUI; }

int GameRenderer::getWidth(void) const { return _width; }

int GameRenderer::getHeight(void) const { return _height; }

GLFWwindow *GameRenderer::getWindow(void) const { return _window; }

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
		_gameEngine->buttonStateChanged(key, action == GLFW_PRESS);
	}
	(void)scancode;
	(void)window;
	(void)mods;
}

GameEngine *GameRenderer::_gameEngine = NULL;
