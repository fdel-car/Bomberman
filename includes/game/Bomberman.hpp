#pragma once

#include "engine/AGame.hpp"
#include "game/Save.hpp"

enum LayerTag {
	WallLayer = 0,
	BoxLayer,
	PlayerLayer,
	PlayerSpecialLayer,
	EnemyLayer,
	EnemyRunAwayLayer,
	EnemySpecialLayer,
	BombLayer,
	ExplosionLayer,
	PerkLayer
};

class Bomberman : public AGame {
	typedef void (Bomberman::*Scene)(void);

   public:
	Bomberman(void);
	virtual ~Bomberman(void);
	virtual bool loadSceneByIndex(int sceneIdx);
	virtual size_t getWindowWidth();
	virtual size_t getWindowHeight();
	virtual bool isFullScreen();

   private:
	Save _save;
	std::map<std::string, Scene> _scenesMap;

	void _initScenes(void);

	void _mainMenu(void);
	void _forest(void);
};
