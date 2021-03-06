#pragma once

#include "game/scenes/SceneTools.hpp"

class Mario : public SceneTools {
   public:
	Mario(WorldLocation &dialogueLocation, WorldLocation &gameplayLocation,
		  float transitionTime, Bomberman *bomberman);
	virtual ~Mario(void);

	virtual void configGUI(GUI *graphicUI);
	virtual void configAI(void);
	virtual void tellPosition(Entity *entity);
	virtual void update(void);

   private:
	Mario(void);
	Mario(Mario const &src);
	Mario &operator=(Mario const &rhs);

	float _cooldown;
};
