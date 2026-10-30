#pragma once

#include <string>
#include <stdlib.h>
#include <memory>
#include <vector>

class skeleton
{
private:
	int _hp;
	int _attackDamage;
	int _attackReduction;
	int _nextAttackBonus;
	bool _skipTurn;

public:

	skeleton();

	std::vector<int> Action(int target);
	int TakeDamage(int amount);
	int GetHP() const;
	int GetAttackReduction() const;
	int GetNextAttackBonus() const;
	void SetAttackReduction(int amount);
	void SetNextAttackBonus(int amount);
	void SkipNextTurn();
	void ClearSkipTurn();
	bool IsSkippingTurn() const;
};

