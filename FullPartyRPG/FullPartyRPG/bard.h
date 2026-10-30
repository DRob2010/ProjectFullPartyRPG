#pragma once

#include <string>
#include <stdlib.h>
#include <memory>
#include "character.h"
#include <vector>

class bard : public character
{
private:
	std::string _name;
	int _hp;
	int _maxHP;
	int _damageReduction;
	std::vector<std::string> _enemyNames;
	int _currentSong; // 1=Attack, 2=Defence, 3=Healing
	std::string _currentSongString;
	std::vector<int> _action = { 0,0,0 };

public:
	bard();

	void SetName() override;

	std::string GetDisplayName() const override;

	void SetEnemyNames(const std::vector<std::string>& names) override;

	std::vector<int> Action() override;

	void TakeDamage(int amount) override;

	int GetHP() const override;
	int GetMaxHP() const override;
	int GetDamageReduction() const override;
	void IncreaseMaxHP(int amount) override;
	void LoadHealth(int hp, int maxHP) override;
	void SetSong(int song);
	std::string GetCurrentSong() const;

	void Heal(int amount) override;

	void SetDamageReduction(int amount) override;

	void ResetDamageReduction() override;

	void ContinueSong();

	void ChangeSong();

	void DaggerAttack();

	void InfluenceHP();
};

