#include "character.h"
#include <vector>

void character::SetName()
{
}

std::string character::GetDisplayName() const
{
	return "Character";
}

void character::SetPartyNames(const std::vector<std::string>& names)
{
}

void character::SetEnemyNames(const std::vector<std::string>& names)
{
}

std::vector<int> character::Action()
{
	return {1};
}

void character::TakeDamage(int amount)
{
}

int character::GetHP() const
{
	return 0;
}

int character::GetMaxHP() const
{
	return 0;
}

int character::GetDamageReduction() const
{
	return 0;
}

void character::IncreaseMaxHP(int amount)
{
}

void character::LoadHealth(int hp, int maxHP)
{
}

void character::Heal(int amount)
{
}

void character::SetDamageReduction(int amount)
{
}

void character::ResetDamageReduction()
{
}
