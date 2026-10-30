#pragma once

#include <string>
#include <stdlib.h>
#include <memory>
#include "character.h"
#include <vector>

class ranger : public character
{
private:
	std::string _name;
	int _hp;
	int _maxHP;
	int _damageReduction;
	std::vector<std::string> _partyNames;
	std::vector<std::string> _enemyNames;
	std::vector<int> _action = { 0,0,0 };

public:
	ranger();

	void SetName() override;

	std::string GetDisplayName() const override;

	void SetPartyNames(const std::vector<std::string>& names) override;

	void SetEnemyNames(const std::vector<std::string>& names) override;

	std::vector<int> Action() override;

	void TakeDamage(int amount) override;

	int GetHP() const override;
	int GetMaxHP() const override;
	int GetDamageReduction() const override;
	void IncreaseMaxHP(int amount) override;
	void LoadHealth(int hp, int maxHP) override;

	void Heal(int amount) override;

	void SetDamageReduction(int amount) override;

	void ResetDamageReduction() override;

	void BowAttack();

	void Herbalism();

	void StudyEnemy();

	void InfluenceHP();
};

