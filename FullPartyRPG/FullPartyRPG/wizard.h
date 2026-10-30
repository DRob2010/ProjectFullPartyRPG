#pragma once

#include <string>
#include <stdlib.h>
#include <memory>
#include "character.h"
#include <vector>


class wizard : public character
{
private:
	std::string _name;
	int _hp;
	int _maxHP;
	int _mana;
	int _damageReduction;
	std::vector<std::string> _enemyNames;
	std::vector<int> _action = { 0,0,0 };

public:
	wizard();

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
	void SetMana(int amount);

	void Heal(int amount) override;

	void SetDamageReduction(int amount) override;

	void ResetDamageReduction() override;

	int GetMana() const;
	void StartTurn(int bonus = 0);
	void UndoStartTurn(int amount = 1);

	void StaffAttack();

	void FocusMana();

	void CastSpell();

	void DarkBlast();

	void JumpingSpark();

	void ChillingAir();

	void InfluenceHP();
};

