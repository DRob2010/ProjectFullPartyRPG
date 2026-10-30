#include "skeleton.h"
#include "game_config.h"

skeleton::skeleton(): _hp(GameConfig::SkeletonMaxHP), _attackDamage(GameConfig::SkeletonAttackDamage),
	_attackReduction(0), _nextAttackBonus(0), _skipTurn(false)
{
}

std::vector<int> skeleton::Action(int target)
{
	int damage = _attackDamage - _attackReduction;

	if (damage < 0) {
		damage = 0;
	}

	// Attack reduction only affects one attack.
	_attackReduction = 0;

	return { 1, target, damage };
}

int skeleton::TakeDamage(int amount)
{
	amount += _nextAttackBonus;
	_nextAttackBonus = 0;

	_hp -= amount;

	if (_hp < 0) {
		_hp = 0;
	}

	return amount;
}

int skeleton::GetHP() const
{
	return _hp;
}

int skeleton::GetAttackReduction() const
{
	return _attackReduction;
}

int skeleton::GetNextAttackBonus() const
{
	return _nextAttackBonus;
}

void skeleton::SetAttackReduction(int amount)
{
	_attackReduction = amount;
}

void skeleton::SetNextAttackBonus(int amount)
{
	_nextAttackBonus = amount;
}

void skeleton::SkipNextTurn()
{
	_skipTurn = true;
}

void skeleton::ClearSkipTurn()
{
	_skipTurn = false;
}

bool skeleton::IsSkippingTurn() const
{
	return _skipTurn;
}
