#pragma once

#include <string>
#include <stdlib.h>
#include <memory>
#include "character.h"
#include <vector>

class knight : public character
{
private:
	std::string _name;
	int _hp;
	int _maxHP;
	int _spiritHP;
	int _damageReduction;
	std::vector<std::string> _enemyNames;
	std::vector<int> _action = { 0,0,0 };

public:
	knight();
	
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
	void SetSpirit(int amount);
	int GetSpirit() const;
	void AddSpirit(int amount);

	void Heal(int amount) override;

	void SetDamageReduction(int amount) override;

	void ResetDamageReduction() override;

	void SwordAttack();

	void Provoke();

	void Defend();

	void InfluenceHP();
};

