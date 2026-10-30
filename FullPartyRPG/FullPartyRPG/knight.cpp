#include "knight.h"
#include "game_config.h"
#include <iostream>
#include <vector>
using namespace std;

knight::knight(): _hp(GameConfig::KnightMaxHP), _maxHP(GameConfig::KnightMaxHP), _spiritHP(0), _damageReduction(0)
{

}

void knight::SetName()
{
	cout << "What is this character's name? ";
	cin >> ws;
	getline(cin, _name);
}

std::string knight::GetDisplayName() const
{
	return "Knight " + _name;
}

void knight::SetEnemyNames(const std::vector<std::string>& names)
{
	_enemyNames = names;
}

std::vector<int> knight::Action()
{
	int action;
	_action = { 0,0,0 };

	cout << "Knight " << _name << " " << _hp << "/" << _maxHP << " +" << _spiritHP << "\n";
	cout << "1) Sword Attack\n2) Provoke\n3) Defend\n";
	cin >> action;

	if (action == 1) {
		SwordAttack();
	}
	else if (action == 2) {
		Provoke();
	}
	else if (action == 3) {
		Defend();
	}
	else {
		cout << "Invalid choice.\n";
	}

	return _action;
}

void knight::SwordAttack()
{
	int target;

	cout << "Who are you targetting\n";
	for (int i = 0; i < _enemyNames.size(); i++) {
		cout << i + 1 << ") " << _enemyNames[i] << "\n";
	}
	cin >> target;

	// Sword attacks build spirit, which acts as temporary HP.
	_spiritHP += 1;
	_action = { ATTACK, target+3, GameConfig::KnightSwordDamage };
}

void knight::Provoke()
{
	_action = { PROVOKE, 1, GameConfig::KnightProvokeReduction };
}

void knight::Defend()
{
	_action = { DEFEND, 1, GameConfig::KnightDefendReduction };
}

void knight::TakeDamage(int amount)
{
	int damageTaken = amount - _damageReduction;

	if (damageTaken < 0) {
		damageTaken = 0;
	}

	int spiritDamage = damageTaken;

	if (spiritDamage > _spiritHP) {
		spiritDamage = _spiritHP;
	}

	_spiritHP -= spiritDamage;
	damageTaken -= spiritDamage;
	_hp -= damageTaken;

	if (_hp < 0) {
		_hp = 0;
	}

	cout << "Knight " << _name << " takes " << damageTaken
		<< " HP damage and loses " << spiritDamage
		<< " spirit. HP: " << _hp << "/" << _maxHP << "\n";
}

int knight::GetHP() const
{
	return _hp;
}

int knight::GetMaxHP() const
{
	return _maxHP;
}

int knight::GetDamageReduction() const
{
	return _damageReduction;
}

void knight::IncreaseMaxHP(int amount)
{
	_maxHP += amount;
	_hp += amount;
}

void knight::LoadHealth(int hp, int maxHP)
{
	_hp = hp;
	_maxHP = maxHP;
}

void knight::SetSpirit(int amount)
{
	_spiritHP = amount;
}

int knight::GetSpirit() const
{
	return _spiritHP;
}

void knight::AddSpirit(int amount)
{
	_spiritHP += amount;
}

void knight::Heal(int amount)
{
	_hp += amount;

	if (_hp > _maxHP) {
		_hp = _maxHP;
	}

	cout << "Knight " << _name << " heals to " << _hp
		<< "/" << _maxHP << " HP.\n";
}

void knight::SetDamageReduction(int amount)
{
	_damageReduction = amount;
}

void knight::ResetDamageReduction()
{
	_damageReduction = 0;
}
