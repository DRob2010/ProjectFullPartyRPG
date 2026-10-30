#include "ranger.h"
#include "game_config.h"
#include <iostream>
#include <vector>
using namespace std;

ranger::ranger(): _hp(GameConfig::RangerMaxHP), _maxHP(GameConfig::RangerMaxHP), _damageReduction(0)
{
}

void ranger::SetName()
{
	cout << "What is this character's name? ";
	cin >> ws;
	getline(cin, _name);
}

std::string ranger::GetDisplayName() const
{
	return "Ranger " + _name;
}

void ranger::SetPartyNames(const std::vector<std::string>& names)
{
	_partyNames = names;
}

void ranger::SetEnemyNames(const std::vector<std::string>& names)
{
	_enemyNames = names;
}

std::vector<int> ranger::Action()
{
	int action;
	_action = { NO_ACTION, 0, 0 };

	cout << "Ranger " << _name << " " << _hp << "/" << _maxHP << " HP\n";
	cout << "1) Bow Attack\n2) Herbalism\n3) Study Enemy\n";
	cin >> action;

	if (action == 1) {
		BowAttack();
	}
	else if (action == 2) {
		Herbalism();
	}
	else if (action == 3) {
		StudyEnemy();
	}
	else {
		cout << "Invalid choice.\n";
	}

	return _action;
}

void ranger::BowAttack()
{
	int target;

	cout << "Who are you targeting?\n";
	for (int i = 0; i < _enemyNames.size(); i++) {
		cout << i + 1 << ") " << _enemyNames[i] << "\n";
	}
	cin >> target;

	_action = { RANGER_ATTACK, target + 3, GameConfig::RangerBowDamage };
}

void ranger::Herbalism()
{
	int target;

	cout << "Who are you healing?\n";
	for (int i = 0; i < _partyNames.size(); i++) {
		cout << i + 1 << ") " << _partyNames[i] << "\n";
	}
	cin >> target;

	_action = { HEAL, target, GameConfig::RangerHerbalismHealing };
}

void ranger::StudyEnemy()
{
	int target;

	cout << "Which enemy are you studying?\n";
	for (int i = 0; i < _enemyNames.size(); i++) {
		cout << i + 1 << ") " << _enemyNames[i] << "\n";
	}
	cin >> target;

	_action = { STUDY, target + 3, GameConfig::RangerStudyBonus };
}

void ranger::TakeDamage(int amount)
{
	amount -= _damageReduction;

	if (amount < 0) {
		amount = 0;
	}

	_hp -= amount;

	if (_hp < 0) {
		_hp = 0;
	}

	cout << "Ranger " << _name << " takes " << amount
		<< " damage. HP: " << _hp << "/" << _maxHP << "\n";
}

int ranger::GetHP() const
{
	return _hp;
}

int ranger::GetMaxHP() const
{
	return _maxHP;
}

int ranger::GetDamageReduction() const
{
	return _damageReduction;
}

void ranger::IncreaseMaxHP(int amount)
{
	_maxHP += amount;
	_hp += amount;
}

void ranger::LoadHealth(int hp, int maxHP)
{
	_hp = hp;
	_maxHP = maxHP;
}

void ranger::SetDamageReduction(int amount)
{
	_damageReduction = amount;
}

void ranger::ResetDamageReduction()
{
	_damageReduction = 0;
}

void ranger::Heal(int amount)
{
	_hp += amount;

	if (_hp > _maxHP) {
		_hp = _maxHP;
	}

	cout << "Ranger " << _name << " heals to " << _hp
		<< "/" << _maxHP << " HP.\n";
}

