#include "wizard.h"
#include "game_config.h"
#include <iostream>
#include <vector>
using namespace std;

wizard::wizard(): _hp(GameConfig::WizardMaxHP), _maxHP(GameConfig::WizardMaxHP), _mana(GameConfig::WizardStartingMana), _damageReduction(0)
{
}

void wizard::SetName()
{
	cout << "What is this character's name? ";
	cin >> ws;
	getline(cin, _name);
}

std::string wizard::GetDisplayName() const
{
	return "Wizard " + _name;
}

void wizard::SetEnemyNames(const std::vector<std::string>& names)
{
	_enemyNames = names;
}

std::vector<int> wizard::Action()
{
	int action;
	_action = { NO_ACTION, 0, 0 };

	cout << "Wizard " << _name << " " << _hp << "/10 HP, " << _mana << " MP\n";
	cout << "1) Staff Attack\n2) Focus Mana\n3) Cast Spell\n";
	cin >> action;

	if (action == 1) {
		StaffAttack();
	}
	else if (action == 2) {
		FocusMana();
	}
	else if (action == 3) {
		CastSpell();
	}
	else {
		cout << "Invalid choice.\n";
	}

	return _action;
}

void wizard::StaffAttack()
{
	int target;

	cout << "Who are you targeting?\n";
	for (int i = 0; i < _enemyNames.size(); i++) {
		cout << i + 1 << ") " << _enemyNames[i] << "\n";
	}
	cin >> target;

	_action = { ATTACK, target + 3, GameConfig::WizardStaffDamage };
}

void wizard::FocusMana()
{
	_mana += 1;
	_action = { FOCUS_MANA, 1, 1 };
}

void wizard::CastSpell()
{
	int spell;

	cout << "Choose a spell:\n";
	cout << "1) Darkblast (2 MP)\n";
	cout << "2) Jumping Spark (3 MP)\n";
	cout << "3) Chilling Air (3 MP)\n";
	cin >> spell;

	if (spell == 1) {
		DarkBlast();
	}
	else if (spell == 2) {
		JumpingSpark();
	}
	else if (spell == 3) {
		ChillingAir();
	}
	else {
		cout << "Invalid spell.\n";
	}
}

void wizard::DarkBlast()
{
	int target;

	if (_mana < GameConfig::WizardDarkblastCost) {
		cout << "Not enough mana.\n";
		return;
	}

	cout << "Who are you targeting?\n";
	for (int i = 0; i < _enemyNames.size(); i++) {
		cout << i + 1 << ") " << _enemyNames[i] << "\n";
	}
	cin >> target;

	_mana -= GameConfig::WizardDarkblastCost;
	_action = { DARK_BLAST, target + 3, GameConfig::WizardDarkblastDamage };
}

void wizard::JumpingSpark()
{
	if (_mana < GameConfig::WizardJumpingSparkCost) {
		cout << "Not enough mana.\n";
		return;
	}

	_mana -= GameConfig::WizardJumpingSparkCost;
	_action = { JUMPING_SPARK, 0, GameConfig::WizardJumpingSparkDamage };
}

void wizard::ChillingAir()
{
	int target;

	if (_mana < GameConfig::WizardChillingAirCost) {
		cout << "Not enough mana.\n";
		return;
	}

	cout << "Which enemy are you targeting?\n";
	for (int i = 0; i < _enemyNames.size(); i++) {
		cout << i + 1 << ") " << _enemyNames[i] << "\n";
	}
	cin >> target;

	_mana -= GameConfig::WizardChillingAirCost;
	_action = { CHILLING_AIR, target + 3, 0 };
}

int wizard::GetMana() const
{
	return _mana;
}

void wizard::StartTurn(int bonus)
{
	_mana += GameConfig::WizardManaPerTurn + bonus;
}

void wizard::UndoStartTurn(int amount)
{
	_mana -= amount;
	if (_mana < 0) _mana = 0;
}

void wizard::TakeDamage(int amount)
{
	amount -= _damageReduction;

	if (amount < 0) {
		amount = 0;
	}

	_hp -= amount;

	if (_hp < 0) {
		_hp = 0;
	}

	cout << "Wizard " << _name << " takes " << amount
		<< " damage. HP: " << _hp << "/" << _maxHP << "\n";
}

int wizard::GetHP() const
{
	return _hp;
}

int wizard::GetMaxHP() const
{
	return _maxHP;
}

int wizard::GetDamageReduction() const
{
	return _damageReduction;
}

void wizard::IncreaseMaxHP(int amount)
{
	_maxHP += amount;
	_hp += amount;
}

void wizard::LoadHealth(int hp, int maxHP)
{
	_hp = hp;
	_maxHP = maxHP;
}

void wizard::SetMana(int amount)
{
	_mana = amount;
}

void wizard::SetDamageReduction(int amount)
{
	_damageReduction = amount;
}

void wizard::ResetDamageReduction()
{
	_damageReduction = 0;
}

void wizard::Heal(int amount)
{
	_hp += amount;

	if (_hp > _maxHP) {
		_hp = _maxHP;
	}

	cout << "Wizard " << _name << " heals to " << _hp
		<< "/" << _maxHP << " HP.\n";
}


