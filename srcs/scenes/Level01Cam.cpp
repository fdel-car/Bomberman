#include "scenes/Level01Cam.hpp"
#include "GameEngine.hpp"
#include "Player.hpp"

Level01Cam::Level01Cam(glm::vec3 const &pos, glm::vec3 const &eulerAngles)
	: Camera(pos, eulerAngles) {}

Level01Cam::~Level01Cam(void) {}

void Level01Cam::configGUI(GUI *graphicUI) {
	graphicUI->setAssetImage(vNeededImage);

	graphicUI->getDefaultStyle(THEME_RED, &defaultStyle);
	graphicUI->setStyle(defaultStyle);
	activeStyle = defaultStyle;
	_pauseMenu = false;
	_slowDialogue = false;
}

void Level01Cam::drawGUI(GUI *graphicUI) {
	if (_pauseMenu || _gameEngine->isKeyPressed("E")) {
		_pauseMenu = true;
		if (graphicUI->uiStartBlock(
				"PauseMenu", "Pause",
				nk_rect((WINDOW_W / 2) - (WINDOW_W / 8), (WINDOW_H / 3),
						WINDOW_W / 4, WINDOW_H / 3),
				NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
			if (graphicUI->uiButton(WINDOW_W / 4, (WINDOW_H / 9) - 8, 0,
									"Resume", "", "14_BOMBERMA")) {
				_pauseMenu = false;
			}
			if (graphicUI->uiButton(WINDOW_W / 4, (WINDOW_H / 9) - 8, 0,
									"Restart level", "", "14_BOMBERMA")) {
				_newSceneIdx = 1;
			}
			if (graphicUI->uiButton(WINDOW_W / 4, (WINDOW_H / 9) - 8, 0,
									"Quit level", "", "14_BOMBERMA")) {
				_newSceneIdx = 0;
			}
			if (graphicUI->uiButton(0, 0, 0, "", "", "")) {
			}
		}
		graphicUI->uiEndBlock();
	}
	static int searchWord = 0;
	static int lastWord = 0;
	static int startStrIdx = 0;
	std::string str =
		"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do "
		"eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim "
		"ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut "
		"aliquip ex ea commodo consequat. Duis aute irure dolor in "
		"reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla "
		"pariatur. Excepteur sint occaecat cupidatat non proident, sunt in "
		"culpa qui officia deserunt mollit anim id est laborum. Lorem ipsum "
		"dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor "
		"incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
		"quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea "
		"commodo consequat. Duis aute irure dolor in reprehenderit in "
		"voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
		"Excepteur sint occaecat cupidatat non proident, sunt in culpa qui "
		"officia deserunt mollit anim id est laborum.";
	_displayDialogue(graphicUI, &searchWord, &lastWord, &startStrIdx,
					 "Bomberman", "", str, false, 130, 3, NK_TEXT_LEFT,
					 "12_BOMBERMA", "18_BOMBERMA");
}

void Level01Cam::_displayDialogue(GUI *graphicUI, int *searchWord,
								  int *lastWord, int *startStrIdx,
								  std::string name, std::string imgName,
								  std::string str, bool isImgLeft,
								  size_t maxCharPerLine, int nbrOfLine,
								  nk_flags textPosition, std::string fontText,
								  std::string fontTitle) {
	if (_slowDialogue) {
		if (*searchWord < (int)str.size()) {
			*searchWord += 1;
			if (str[*searchWord] == ' ') *lastWord = *searchWord;
		} else
			*lastWord = *searchWord;
		if (*lastWord - *startStrIdx >= 130 * 3) *startStrIdx += 130 * 2;
	}
	_slowDialogue = !_slowDialogue;
	std::string displayableStr =
		str.substr(*startStrIdx, *lastWord - *startStrIdx);
	graphicUI->uiDialogBox(name.c_str(), imgName, displayableStr.c_str(),
						   isImgLeft, maxCharPerLine, nbrOfLine, textPosition,
						   fontText, fontTitle);
}
