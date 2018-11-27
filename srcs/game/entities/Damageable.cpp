#include "game/entities/Damageable.hpp"
#include "engine/GameEngine.hpp"

Damageable::Damageable(glm::vec3 position, glm::vec3 eulerAngles,
					   Collider *collider, std::string modelName,
					   std::string name, std::string tag, size_t hp,
					   int baseLayer, int damagedLayer, float damagedMaxTime,
					   Entity *sceneManager)
	: Entity(position, eulerAngles, collider, modelName, name, tag,
			 sceneManager),
	  _alive(hp != 0),
	  _hp(hp),
	  _baseLayer(baseLayer),
	  _damagedLayer(damagedLayer),
	  _damagedMaxTime(damagedMaxTime),
	  _timeDamaged(0.0f) {}

Damageable::~Damageable(void) {}

void Damageable::update(void) {
	if (_alive && _timeDamaged > 0.0f) {
		_timeDamaged -= _gameEngine->getDeltaTime();
		if (_timeDamaged <= 0.0f) {
			_collider->layerTag = _baseLayer;
		}
	}
}

void Damageable::takeDamage(void) {
	if (_alive && _timeDamaged <= 0.0f) {
		onTakeDamage();
		if (_hp == 0) {
			_alive = false;
			onDeath();
		}
	}
}

void Damageable::onTakeDamage(void) {
	if (_alive) {
		_hp -= 1;
		_timeDamaged = _damagedMaxTime;
		_collider->layerTag = _damagedLayer;
	}
}

void Damageable::onDeath(void) {
	if (!_alive) _needToBeDestroyed = true;
}