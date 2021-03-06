#include "game/entities/Bomb.hpp"
#include "engine/GameEngine.hpp"
#include "game/Bomberman.hpp"
#include "game/scenes/SceneTools.hpp"

Bomb::Bomb(glm::vec3 position, float timer, size_t range, Entity *sceneManager)
	: Damageable(
		  glm::vec3(position.x, position.y + 0.24f, position.z), glm::vec3(0),
		  new Collider(Collider::Rectangle, LayerTag::BombLayer, 0.4f, 0.4f),
		  "Bomb", "Bomb", "Bomb", 1, BombLayer, BombLayer, 0.0f, sceneManager),
	  _timer(timer),
	  _range(range),
	  _slideSpeed(0.0f) {
	scale(glm::vec3(0.4f));
	shouldBeAnimated = true;
	loopAnim = false;
	_neededSounds.insert("put_bomb_1");
	_neededSounds.insert("put_bomb_2");
	_initSounds.push_back("put_bomb_1");
	_initSounds.push_back("put_bomb_2");
}

Bomb::~Bomb(void) {}

void Bomb::update(void) {
	if (_slideSpeed != 0.0f) {
		float deltaTime = _gameEngine->getDeltaTime();
		_targetMovement.x = _xDirection * _slideSpeed * deltaTime;
		_targetMovement.z = _zDirection * _slideSpeed * deltaTime;
	} else {
		float deltaTime = _gameEngine->getDeltaTime();

		_timer -= deltaTime;
		(void)_range;

		if (_timer <= 0.0f) {
			this->takeDamage();
		}
	}
}

void Bomb::onDeath(void) {
	SceneTools *cam = dynamic_cast<SceneTools *>(_sceneManager);

	if (cam != nullptr) {
		cam->putExplosion(getPosition().x, getPosition().z, _range);
	}
	_needToBeDestroyed = true;
}

void Bomb::onCollisionEnter(Entity *entity) {
	if (_slideSpeed != 0.0f) {
		if (entity->getId() != _pusherID) {
			this->takeDamage();
		}
	}
}

void Bomb::pushBomb(int xSign, int zSign, size_t pusherID) {
	if (_slideSpeed != 0.0f || (xSign == 0.0f && zSign == 0.0f)) return;
	shouldBeAnimated = false;
	_pusherID = pusherID;
	_slideSpeed = 7.0f;
	if (xSign != 0 && zSign != 0) zSign = 0;
	_xDirection = static_cast<float>(xSign);
	_zDirection = static_cast<float>(zSign);
}
