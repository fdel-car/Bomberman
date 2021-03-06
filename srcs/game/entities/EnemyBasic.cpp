#include "game/entities/EnemyBasic.hpp"

EnemyBasic::EnemyBasic(glm::vec3 position, glm::vec3 eulerAngles, std::string modelName,
					   Entity *sceneManager)
	: AEnemy(position, eulerAngles, "EnemyBasic", EnemyBasicLayer, true,
			 sceneManager, modelName) {
	_speed = 1.0f;
}

EnemyBasic::~EnemyBasic(void) {}

void EnemyBasic::update(void) {
	SceneTools *cam = dynamic_cast<SceneTools *>(_sceneManager);
	if (cam == nullptr) {
		std::cerr << "[EnemyBasic] Update has failed." << std::endl;
		return;
	}
	randomMove(cam, 3.0f);
	walk(cam);
}
