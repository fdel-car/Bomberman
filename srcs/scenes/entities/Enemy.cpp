#include "scenes/entities/Enemy.hpp"
#include "GameEngine.hpp"

Enemy::Enemy(glm::vec3 position, glm::vec3 eulerAngles)
	: Entity(position, eulerAngles,
			 new Collider(Collider::Circle, 0.75f, 0.75f), "Enemy") {
	_name = "Enemy";
	_tag = "Enemy";
	_speed = 8.0f;
}

Enemy::~Enemy(void) {}

void Enemy::update(void) {
	float deltaTime = _gameEngine->getDeltaTime();

	int xSign = 0;
	int zSign = 0;
	float xDirection = 0.0f;
	// float yDirection = 0.0f;  // Not used since we cannot jump yet
	float zDirection = 0.0f;

	// Update position based on keyboard
	if (_gameEngine->isKeyPressed(KEY_A)) xSign -= 1;
	if (_gameEngine->isKeyPressed(KEY_D)) xSign += 1;
	if (_gameEngine->isKeyPressed(KEY_W)) zSign -= 1;
	if (_gameEngine->isKeyPressed(KEY_S)) zSign += 1;
	if (xSign == 0 && zSign == 0) {
		// TODO: check for joystick input
	} else {
		// Normalize direction
		xDirection = static_cast<float>(xSign);
		zDirection = static_cast<float>(zSign);
		xSign = abs(xSign);
		zSign = abs(zSign);
		float totalMagnitude = abs(xSign) + abs(zSign);
		xDirection *= sqrt(xSign / totalMagnitude);
		zDirection *= sqrt(zSign / totalMagnitude);
	}

	_targetMovement.x = xDirection * _speed * deltaTime;
	_targetMovement.z = zDirection * _speed * deltaTime;
}
