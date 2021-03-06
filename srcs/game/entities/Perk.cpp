#include "game/entities/Perk.hpp"
#include "engine/GameEngine.hpp"
#include "game/Bomberman.hpp"
#include "game/entities/Player.hpp"
#include "game/scenes/SceneTools.hpp"

const std::vector<std::tuple<PerkType, size_t, std::string>> getPossiblePerks(
	void) {
	std::vector<std::tuple<PerkType, size_t, std::string>> vPerk;
	vPerk.push_back(std::make_tuple<PerkType, size_t, std::string>(
		SpeedBoost, 4, "SpeedPerk"));
	vPerk.push_back(std::make_tuple<PerkType, size_t, std::string>(
		BombRange, 3, "RangePerk"));
	vPerk.push_back(std::make_tuple<PerkType, size_t, std::string>(
		MaxBomb, 4, "MaxBombPerk"));
	vPerk.push_back(std::make_tuple<PerkType, size_t, std::string>(
		Damage, 1, "DamagePerk"));
	vPerk.push_back(std::make_tuple<PerkType, size_t, std::string>(KickBomb, 2,
																   "KickPerk"));
	return vPerk;
}
const std::vector<std::tuple<PerkType, size_t, std::string>>
	Perk::_possiblePerks = getPossiblePerks();

size_t getTotalPerkProbability(
	std::vector<std::tuple<PerkType, size_t, std::string>> const &vPerk) {
	size_t tot = 0;
	for (auto const &i : vPerk) {
		tot += std::get<1>(i);
	}
	return tot;
}
const size_t Perk::_totalPerkProbs =
	getTotalPerkProbability(Perk::_possiblePerks);

const std::vector<std::string> getDamagingSounds(void) {
	std::vector<std::string> vSounds;
	vSounds.push_back("bad_perk");
	return vSounds;
}
const std::vector<std::string> Perk::_damagingSounds = getDamagingSounds();

bool Perk::kickPerkDropped = false;

void Perk::_defineDroppedPerk(void) {
	size_t randResult = rand() % _totalPerkProbs;

	for (auto const &perk : _possiblePerks) {
		if (std::get<1>(perk) > randResult) {
			_perkType = std::get<0>(perk);
			_modelName = std::get<2>(perk);
			break;
		} else {
			randResult -= std::get<1>(perk);
		}
	}
}

Perk::Perk(glm::vec3 position, Entity *sceneManager)
	: Entity(glm::vec3(position.x, position.y + 0.3f, position.z), glm::vec3(0),
			 new Collider(Collider::Rectangle, LayerTag::PerkLayer, 0.4f, 0.4f,
						  true),
			 "SetPerk", "Perk", "Perk", sceneManager) {
	do {
		_defineDroppedPerk();
	} while (_perkType == KickBomb && kickPerkDropped);

	if (_perkType == KickBomb) kickPerkDropped = true;

	_destroySounds.push_back("get_perk_1");
	_destroySounds.push_back("get_perk_2");
	_destroySounds.push_back("get_perk_3");
	_destroySounds.push_back("get_perk_4");
}

Perk::~Perk(void) {}

void Perk::onTriggerEnter(Entity *entity) {
	Player *player = dynamic_cast<Player *>(entity);

	if (player != nullptr) {
		switch (_perkType) {
			case SpeedBoost:
				player->gotSpeedBoost(0.8f);
				break;
			case BombRange:
				player->gotBombRangeBoost(1);
				break;
			case MaxBomb:
				player->gotMaxBombBoost(1);
				break;
			case Damage:
				player->takeDamage(_damagingSounds);
				_destroySounds.clear();
				break;
			case KickBomb:
				player->gotBombKickBoost(true);
				break;
			default:
				break;
		}
		_needToBeDestroyed = true;
	}
}