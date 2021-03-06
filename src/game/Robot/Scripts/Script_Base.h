#ifndef ROBOT_STRATEGIES_SCRIPT_BASE_H
#define ROBOT_STRATEGIES_SCRIPT_BASE_H

#include "Unit.h"
#include "Item.h"
#include "Player.h"
#include "RobotManager.h"

class Script_Base
{
public:
	Script_Base(Player* pmMe);
	virtual void Reset();
	virtual bool DPS(Unit* pmTarget, bool pmChase = true);
	virtual bool Tank(Unit* pmTarget, bool pmChase, bool pmSingle = false);
	virtual bool SubTank(Unit* pmTarget, bool pmChase);
	virtual bool Taunt(Unit* pmTarget);
	virtual bool InterruptCasting(Unit* pmTarget);
	virtual bool Heal(Unit* pmTarget, bool pmCure = true);
	virtual bool SubHeal(Unit* pmTarget, bool pmCure = true);
	virtual bool GroupHeal(float pmMaxHealthPercent = 60.0f);
	virtual bool Attack(Unit* pmTarget);
	virtual bool Buff(Unit* pmTarget, bool pmCure = true);
	virtual bool Assist();

	bool Update(uint32 pmDiff);

	void PetAttack(Unit* pmTarget);
	void PetStop();

	void IdentifyCharacterSpells();
	Item* GetItemInInventory(uint32 pmEntry);
	bool UseItem(Item* pmItem, Unit* pmTarget);
	bool UseHealingPotion();
	bool UseManaPotion();
	uint32 FindSpellID(std::string pmSpellName);
	bool SpellValid(uint32 pmSpellID);
	bool CastSpell(Unit* pmTarget, std::string pmSpellName, float pmDistance = VISIBILITY_DISTANCE_NORMAL, bool pmCheckAura = false, bool pmOnlyMyAura = false, bool pmClearShapeShift = false);
	void ClearShapeshift();
	void CancelAura(uint32 pmSpellID);
	bool CancelAura(std::string pmSpellName);
	bool Eat();
	bool Drink();
	bool Chase(Unit* pmTarget, float pmMaxDistance = MELEE_MAX_DISTANCE, float pmMinDistance = MELEE_MIN_DISTANCE);
	bool Follow(Unit* pmTarget, float pmDistance = FOLLOW_MIN_DISTANCE);
	void ChooseTarget(Unit* pmTarget);
	void ClearTarget();
	std::set<Unit*> GetAttackersInRange(float pmRangeLimit = RANGED_MAX_DISTANCE);

	Player* me;
	std::unordered_map<std::string, uint32> spellIDMap;
	std::unordered_map<std::string, uint8> spellLevelMap;

	// 0 dps, 1 tank, 2 healer
	uint32 characterType;
	bool petting;
	float chaseDistanceMin;
	float chaseDistanceMax;
	int rti;
};
#endif
