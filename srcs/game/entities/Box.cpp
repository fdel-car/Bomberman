#include "game/entities/Box.hpp"
#include "engine/GameEngine.hpp"
#include "game/Bomberman.hpp"
#include "game/entities/Perk.hpp"
#include "game/scenes/SceneTools.hpp"

Box::Box(glm::vec3 position, Entity *sceneManager, int perkProb,
		 Entity *toSpawn)
	: Damageable(
		  glm::vec3(position.x, position.y + 0.4f, position.z), glm::vec3(0.0f),
		  new Collider(Collider::Rectangle, LayerTag::BoxLayer, 0.45f, 0.45f),
		  "Wall", "Box", "Box", 1, BoxLayer, WallLayer, 1.0f, sceneManager),
	  _onFire(false),
	  _hasSpawned(false),
	  _timer(1.0f),
	  _perkProb(perkProb),
	  _toSpawn(toSpawn) {
	scale(glm::vec3(0.9, 0.8, 0.9));
	setColor(glm::vec3(0.55, 0.3, 0.1));
}

Box::~Box(void) {
	if (!_hasSpawned && _toSpawn != nullptr) {
		delete _toSpawn;
		_toSpawn = nullptr;
	}
}

void Box::update(void) {
	if (!_onFire) return;

	float deltaTime = _gameEngine->getDeltaTime();
	_timer -= deltaTime;

	if (_timer <= 0.0f) {
		_needToBeDestroyed = true;
		if (_toSpawn != nullptr) {
			_hasSpawned = true;
			_toSpawn->translate(getPosition() - _toSpawn->getPosition());
			_gameEngine->addNewEntity(_toSpawn);
		} else if (rand() % 100 < _perkProb) {
			_gameEngine->addNewEntity(new Perk(getPosition(), _sceneManager));
		}
	}
}

void Box::onDeath(void) {
	setColor(glm::vec3(0.9, 0.6, 0.1));
	_onFire = true;
}