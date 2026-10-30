#include "bard.h"
#include "game_config.h"
#include <iostream>
#include <vector>
using namespace std;

bard::bard(): _hp(GameConfig::BardMaxHP), _maxHP(GameConfig::BardMaxHP), _damageReduction(0), _currentSong(1), _currentSongString("Song of Attack")
{
}

void bard::SetName()
{
	cout << "What is this character's name? ";
	cin >> ws;
	getline(cin, _name);
}

std::string bard::GetDisplayName() const
{
	return "Bard " + _name;
}

void bard::SetEnemyNames(const std::vector<std::string>& names)
{
	_enemyNames = names;
}

std::vector<int> bard::Action()
{
	int action;
	_action = { NO_ACTION, 0, 0 };

	cout << "Bard " << _name << " " << _hp << "/" << _maxHP << " HP, " << _currentSongString << "\n";
	cout << "1) Continue Song\n2) Change Song\n3) Dagger Attack\n";
	cin >> action;

	if (action == 1) {
		ContinueSong();
	}
	else if (action == 2) {
		ChangeSong();
	}
	else if (action == 3) {
		DaggerAttack();
	}
	else {
		cout << "Invalid choice.\n";
	}

	return _action;
}

void bard::ContinueSong()
{
	// Target stores the current song: 1=damage, 2=defense, 3=healing.
	_action = { BARD_SONG, _currentSong, GameConfig::BardSongAmount };
}

void bard::ChangeSong()
{
	cout << "Choose a song:\n";
	cout << "1) Song of Attack(+1 Damage)\n2) Song of Defense(-1 Damage Taken)\n3) Song of Healing(Heal 1)\n";
	cin >> _currentSong;

	if (_currentSong < 1 || _currentSong > 3) {
		cout << "Invalid song.\n";
		_currentSong = 1;
	}

	if (_currentSong == 1) {
		_currentSongString = "Song of Attack";
	}
	else if (_currentSong == 2) {
		_currentSongString = "Song of Defense";
	}
	else if (_currentSong == 3) {
		_currentSongString = "Song of Healing";
	}

	ContinueSong();
}

void bard::DaggerAttack()
{
	int target;

	cout << "Who are you targeting?\n";
	for (int i = 0; i < _enemyNames.size(); i++) {
		cout << i + 1 << ") " << _enemyNames[i] << "\n";
	}
	cin >> target;

	_action = { ATTACK, target + 3, GameConfig::BardDaggerDamage };
}

void bard::TakeDamage(int amount)
{
	amount -= _damageReduction;

	if (amount < 0) {
		amount = 0;
	}

	_hp -= amount;

	if (_hp < 0) {
		_hp = 0;
	}

	cout << "Bard " << _name << " takes " << amount
		<< " damage. HP: " << _hp << "/" << _maxHP << "\n";
}

int bard::GetHP() const
{
	return _hp;
}

int bard::GetMaxHP() const
{
	return _maxHP;
}

int bard::GetDamageReduction() const
{
	return _damageReduction;
}

void bard::IncreaseMaxHP(int amount)
{
	_maxHP += amount;
	_hp += amount;
}

void bard::LoadHealth(int hp, int maxHP)
{
	_hp = hp;
	_maxHP = maxHP;
}

void bard::SetSong(int song)
{
	_currentSong = song;
	_currentSongString = song == 2 ? "Song of Defense" : song == 3 ? "Song of Healing" : "Song of Attack";
}

std::string bard::GetCurrentSong() const
{
	return _currentSongString;
}

void bard::SetDamageReduction(int amount)
{
	_damageReduction = amount;
}

void bard::ResetDamageReduction()
{
	_damageReduction = 0;
}

void bard::Heal(int amount)
{
	_hp += amount;

	if (_hp > _maxHP) {
		_hp = _maxHP;
	}

	cout << "Bard " << _name << " heals to " << _hp
		<< "/" << _maxHP << " HP.\n";
}
