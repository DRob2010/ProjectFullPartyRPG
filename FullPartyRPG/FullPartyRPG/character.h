#pragma once
#include <string>
#include <vector>

enum ActionType {
	NO_ACTION = 0,
	ATTACK = 1,
	HEAL = 2,
	PROVOKE = 3,
	DEFEND = 4,
	RANGER_ATTACK = 5,
	//HERBALISM = 6,
	STUDY = 7,
	BARD_SONG = 8,
	//WIZARD_STAFF = 9,
	DARK_BLAST = 10,
	JUMPING_SPARK = 11,
	CHILLING_AIR = 12,
	FOCUS_MANA = 13
};

class character
{
private:


public:
	virtual ~character() = default;

	virtual void SetName();

	virtual std::string GetDisplayName() const;

	virtual void SetPartyNames(const std::vector<std::string>& names);

	virtual void SetEnemyNames(const std::vector<std::string>& names);

	virtual std::vector<int> Action();

	virtual void TakeDamage(int amount);

	virtual int GetHP() const;
	virtual int GetMaxHP() const;
	virtual int GetDamageReduction() const;
	virtual void IncreaseMaxHP(int amount);
	virtual void LoadHealth(int hp, int maxHP);

	virtual void Heal(int amount);

	virtual void SetDamageReduction(int amount);

	virtual void ResetDamageReduction();
};

