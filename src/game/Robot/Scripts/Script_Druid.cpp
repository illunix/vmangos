#include "Script_Druid.h"
#include "Group.h"
#include "RobotManager.h"

Script_Druid::Script_Druid(Player* pmMe) :Script_Base(pmMe)
{
	demoralizingRoarDelay = 20 * TimeConstants::IN_MILLISECONDS;
}

bool Script_Druid::SubTank(Unit* pmTarget, bool pmChase)
{
	if (!pmTarget)
	{
		return false;
	}
	else if (!pmTarget->IsAlive())
	{
		return false;
	}
	if (!me)
	{
		return false;
	}
	if (!me->IsValidAttackTarget(pmTarget))
	{
		return false;
	}
	float targetDistance = me->GetDistance(pmTarget);
	if (pmChase)
	{
		if (targetDistance > ATTACK_RANGE_LIMIT)
		{
			return false;
		}
		if (!Chase(pmTarget))
		{
			return false;
		}
	}
	else
	{
		if (targetDistance > RANGED_MAX_DISTANCE)
		{
			return false;
		}
		if (!me->isInFront(pmTarget, M_PI / 16))
		{
			me->SetFacingToObject(pmTarget);
		}
	}
	if (me->GetHealthPercent() < 20.0f)
	{
		UseHealingPotion();
	}
	switch (me->GetShapeshiftForm())
	{
	case ShapeshiftForm::FORM_NONE:
	{
		if (CastSpell(me, "Dire Bear Form"))
		{
			return true;
		}
		if (CastSpell(me, "Bear Form"))
		{
			return true;
		}
		break;
	}
	case ShapeshiftForm::FORM_BEAR:
	{
		break;
	}
	case ShapeshiftForm::FORM_DIREBEAR:
	{
		break;
	}
	default:
	{
		ClearShapeshift();
		return true;
	}
	}
	if (me->GetHealthPercent() < 30.0f)
	{
		UseHealingPotion();
	}
	if (me->GetHealthPercent() < 20.0f)
	{
		if (CastSpell(me, "Survival Instincts"))
		{
			me->Yell("Survival Instincts!", Language::LANG_UNIVERSAL);
		}
	}
	me->Attack(pmTarget, true);
	uint32 rage = me->GetPower(Powers::POWER_RAGE);
	if (rage > 300)
	{
		if (CastSpell(pmTarget, "Maul", MELEE_MAX_DISTANCE))
		{
			return true;
		}
	}

	return true;
}

void Script_Druid::Update(uint32 pmDiff)
{
	if (!me)
	{
		return;
	}
	if (me->IsInCombat())
	{
		if (demoralizingRoarDelay > 0)
		{
			demoralizingRoarDelay -= pmDiff;
		}
	}
	else
	{
		demoralizingRoarDelay = 0;
	}
	Script_Base::Update(pmDiff);
}

bool Script_Druid::DPS(Unit* pmTarget, bool pmChase)
{
	if (!me)
	{
		return false;
	}
	uint32 characterTalentTab = me->GetMaxTalentCountTab();
	switch (characterTalentTab)
	{
	case 0:
	{
		if ((me->GetPower(Powers::POWER_MANA) * 100 / me->GetMaxPower(Powers::POWER_MANA)) < 30)
		{
			UseManaPotion();
		}
		return DPS_Balance(pmTarget, pmChase);
	}
	case 1:
	{
		if (me->GetHealthPercent() < 30.0f)
		{
			UseHealingPotion();
		}
		if (me->GetHealthPercent() < 20.0f)
		{
			if (CastSpell(me, "Survival Instincts"))
			{
				me->Yell("Survival Instincts!", Language::LANG_UNIVERSAL);
			}
		}
		return DPS_Feral(pmTarget, pmChase);
	}
	case 2:
	{
		if ((me->GetPower(Powers::POWER_MANA) * 100 / me->GetMaxPower(Powers::POWER_MANA)) < 30)
		{
			UseManaPotion();
		}
		return DPS_Common(pmTarget, pmChase);
	}
	default:
	{
		if ((me->GetPower(Powers::POWER_MANA) * 100 / me->GetMaxPower(Powers::POWER_MANA)) < 30)
		{
			UseManaPotion();
		}
		return DPS_Common(pmTarget, pmChase);
	}
	}
	return false;
}

bool Script_Druid::DPS_Balance(Unit* pmTarget, bool pmChase)
{
	if (!me)
	{
		return false;
	}
	if (!pmTarget)
	{
		return false;
	}
	else if (!me->IsValidAttackTarget(pmTarget))
	{
		return false;
	}
	else if (!pmTarget->IsAlive())
	{
		return false;
	}
	float targetDistance = me->GetDistance(pmTarget);
	if (pmChase)
	{
		if (targetDistance > ATTACK_RANGE_LIMIT)
		{
			return false;
		}
		if (!Chase(pmTarget))
		{
			return false;
		}
	}
	else
	{
		if (targetDistance > RANGED_MAX_DISTANCE)
		{
			return false;
		}
		if (!me->isInFront(pmTarget, M_PI / 16))
		{
			me->SetFacingToObject(pmTarget);
		}
	}
	if (me->GetShapeshiftForm() == ShapeshiftForm::FORM_NONE)
	{
		if (CastSpell(me, "Moonkin Form"))
		{
			return true;
		}
	}
	if ((me->GetPower(Powers::POWER_MANA) * 100 / me->GetMaxPower(Powers::POWER_MANA)) < 20)
	{
		if (CastSpell(me, "Innervate", DRUID_RANGE_DISTANCE))
		{
			return true;
		}
	}

	// when facing boss 
	if (pmTarget->GetMaxHealth() / me->GetMaxHealth() > 10.0f)
	{
		if (CastSpell(pmTarget, "Moonfire", DRUID_RANGE_DISTANCE, true, true))
		{
			return true;
		}
		if (CastSpell(pmTarget, "Insect Swarm", DRUID_RANGE_DISTANCE, true, true))
		{
			return true;
		}
		if (CastSpell(pmTarget, "Faerie Fire", DRUID_RANGE_DISTANCE, true))
		{
			return true;
		}
		if (me->HasAura(DRUID_AURA_ECLIPSE_LUNAR))
		{
			if (CastSpell(pmTarget, "Starfire", DRUID_RANGE_DISTANCE))
			{
				return true;
			}
		}
		if (me->HasAura(DRUID_AURA_ECLIPSE_SOLAR))
		{
			if (CastSpell(pmTarget, "Wrath", DRUID_RANGE_DISTANCE))
			{
				return true;
			}
		}
		if (CastSpell(pmTarget, "Starfire", DRUID_RANGE_DISTANCE))
		{
			return true;
		}
		if (CastSpell(pmTarget, "Wrath", DRUID_RANGE_DISTANCE))
		{
			return true;
		}
	}
	else
	{
		if (CastSpell(pmTarget, "Moonfire", DRUID_RANGE_DISTANCE, true, true))
		{
			return true;
		}
		if (CastSpell(pmTarget, "Insect Swarm", DRUID_RANGE_DISTANCE, true, true))
		{
			return true;
		}
		if (me->HasAura(DRUID_AURA_ECLIPSE_LUNAR))
		{
			if (CastSpell(pmTarget, "Starfire", DRUID_RANGE_DISTANCE))
			{
				return true;
			}
		}
		if (me->HasAura(DRUID_AURA_ECLIPSE_SOLAR))
		{
			if (CastSpell(pmTarget, "Wrath", DRUID_RANGE_DISTANCE))
			{
				return true;
			}
		}
		if (CastSpell(pmTarget, "Starfire", DRUID_RANGE_DISTANCE))
		{
			return true;
		}
		if (CastSpell(pmTarget, "Wrath", DRUID_RANGE_DISTANCE))
		{
			return true;
		}
	}

	return true;
}

bool Script_Druid::DPS_Feral(Unit* pmTarget, bool pmChase)
{
	if (!pmTarget)
	{
		return false;
	}
	else if (!pmTarget->IsAlive())
	{
		return false;
	}
	if (!me)
	{
		return false;
	}
	else if (!me->IsValidAttackTarget(pmTarget))
	{
		return false;
	}
	float targetDistance = me->GetDistance(pmTarget);
	if (targetDistance > ATTACK_RANGE_LIMIT)
	{
		return false;
	}

	switch (me->GetShapeshiftForm())
	{
	case ShapeshiftForm::FORM_NONE:
	{
		CastSpell(me, "Cat Form");
		break;
	}
	case ShapeshiftForm::FORM_CAT:
	{
		float targetDistance = me->GetDistance(pmTarget);
		if (pmChase)
		{
			if (targetDistance > ATTACK_RANGE_LIMIT)
			{
				return false;
			}
			if (!Chase(pmTarget))
			{
				return false;
			}
		}
		else
		{
			if (targetDistance > RANGED_MAX_DISTANCE)
			{
				return false;
			}
			if (!me->isInFront(pmTarget, M_PI / 16))
			{
				me->SetFacingToObject(pmTarget);
			}
		}
		CastSpell(me, "Dash");
		me->Attack(pmTarget, true);
		if (me->GetHealthPercent() < 50.0f)
		{
			CastSpell(me, "Barkskin");
		}
		uint32 energy = me->GetPower(Powers::POWER_ENERGY);
		if (CastSpell(pmTarget, "Faerie Fire (Feral)", DRUID_RANGE_DISTANCE, true))
		{
			return true;
		}
		if (sRobotManager->HasAura(me, "Berserk"))
		{
			uint8 comboPoints = me->GetComboPoints();
			if (comboPoints >= 4)
			{
				if (energy > 17)
				{
					if (CastSpell(pmTarget, "Rip", MELEE_MAX_DISTANCE, true, true))
					{
						return true;
					}
					if (CastSpell(pmTarget, "Ferocious Bite", MELEE_MAX_DISTANCE))
					{
						return true;
					}
				}
			}
			if (energy > 22)
			{
				if (CastSpell(pmTarget, "Rake", MELEE_MAX_DISTANCE, true, true))
				{
					return true;
				}
				if (CastSpell(pmTarget, "Mangle (Cat)", MELEE_MAX_DISTANCE))
				{
					return true;
				}
				if (CastSpell(pmTarget, "Claw", MELEE_MAX_DISTANCE))
				{
					return true;
				}
			}
		}
		else
		{
			// when facing boss 
			if (pmTarget->GetMaxHealth() / me->GetMaxHealth() > 10.0f)
			{
				if (energy > 70)
				{
					if (CastSpell(me, "Berserk", MELEE_MAX_DISTANCE))
					{
						me->Yell("Berserk!", Language::LANG_UNIVERSAL);
						return true;
					}
				}
			}
			if (energy < 30)
			{
				if (CastSpell(me, "Tiger's Fury", MELEE_MAX_DISTANCE, true))
				{
					return true;
				}
			}
			uint8 comboPoints = me->GetComboPoints();
			if (comboPoints >= 4)
			{
				if (energy > 35)
				{
					if (CastSpell(pmTarget, "Rip", MELEE_MAX_DISTANCE, true, true))
					{
						return true;
					}
					if (CastSpell(pmTarget, "Ferocious Bite", MELEE_MAX_DISTANCE))
					{
						return true;
					}
				}
			}
			if (energy > 45)
			{
				if (CastSpell(pmTarget, "Rake", MELEE_MAX_DISTANCE, true, true))
				{
					return true;
				}
				if (CastSpell(pmTarget, "Mangle (Cat)", MELEE_MAX_DISTANCE))
				{
					return true;
				}
				if (CastSpell(pmTarget, "Claw", MELEE_MAX_DISTANCE))
				{
					return true;
				}
			}
		}
		break;
	}
	default:
	{
		ClearShapeshift();
		break;
	}
	}

	return true;
}

bool Script_Druid::DPS_Common(Unit* pmTarget, bool pmChase)
{
	return DPS_Balance(pmTarget, pmChase);
}

bool Script_Druid::Tank(Unit* pmTarget, bool pmChase, bool pmSingle)
{
	if (!me)
	{
		return false;
	}
	if (me->GetHealthPercent() < 30.0f)
	{
		UseHealingPotion();
	}
	if (me->GetHealthPercent() < 20.0f)
	{
		if (CastSpell(me, "Survival Instincts"))
		{
			me->Yell("Survival Instincts!", Language::LANG_UNIVERSAL);
		}
	}
	uint32 characterTalentTab = me->GetMaxTalentCountTab();
	switch (characterTalentTab)
	{
	case 0:
	{
		return Tank_Feral(pmTarget, pmChase, pmSingle);
	}
	case 1:
	{
		return Tank_Feral(pmTarget, pmChase, pmSingle);
	}
	case 2:
	{
		return Tank_Feral(pmTarget, pmChase, pmSingle);
	}
	default:
	{
		return Tank_Feral(pmTarget, pmChase, pmSingle);
	}
	}

	return false;
}

bool Script_Druid::Tank_Feral(Unit* pmTarget, bool pmChase, bool pmSingle)
{
	if (!pmTarget)
	{
		return false;
	}
	else if (!pmTarget->IsAlive())
	{
		return false;
	}
	if (!me)
	{
		return false;
	}
	else if (!me->IsValidAttackTarget(pmTarget))
	{
		return false;
	}
	float targetDistance = me->GetDistance(pmTarget);
	if (pmChase)
	{
		if (targetDistance > ATTACK_RANGE_LIMIT)
		{
			return false;
		}
		if (!Chase(pmTarget))
		{
			return false;
		}
		if (targetDistance > DRUID_CHARGE_DISTANCE && targetDistance < DRUID_RANGE_DISTANCE)
		{
			if (CastSpell(pmTarget, "Feral Charge - Bear", DRUID_RANGE_DISTANCE))
			{
				return true;
			}
		}
	}
	else
	{
		if (targetDistance > RANGED_MAX_DISTANCE)
		{
			return false;
		}
		if (!me->isInFront(pmTarget, M_PI / 16))
		{
			me->SetFacingToObject(pmTarget);
		}
	}
	me->Attack(pmTarget, true);
	switch (me->GetShapeshiftForm())
	{
	case ShapeshiftForm::FORM_NONE:
	{
		if (CastSpell(me, "Dire Bear Form"))
		{
			return true;
		}
		if (CastSpell(me, "Bear Form"))
		{
			return true;
		}
		break;
	}
	case ShapeshiftForm::FORM_BEAR:
	{
		break;
	}
	case ShapeshiftForm::FORM_DIREBEAR:
	{
		break;
	}
	default:
	{
		ClearShapeshift();
		return true;
	}
	}
	if (me->GetHealthPercent() < 50.0f)
	{
		CastSpell(me, "Barkskin");
	}
	CastSpell(me, "Enrage");
	uint32 rage = me->GetPower(Powers::POWER_RAGE);
	if (rage > 500)
	{
		if (me->GetHealthPercent() < 50.0f)
		{
			if (CastSpell(me, "Frenzied Regeneration"))
			{
				return true;
			}
		}
	}
	if (rage > 100)
	{
		if (demoralizingRoarDelay <= 0)
		{
			if (!pmSingle)
			{
				if (CastSpell(pmTarget, "Demoralizing Roar", MELEE_MAX_DISTANCE, true))
				{
					demoralizingRoarDelay = 20 * TimeConstants::IN_MILLISECONDS;
					return true;
				}
			}
		}
		if (pmTarget->IsNonMeleeSpellCasted(false))
		{
			if (CastSpell(pmTarget, "Bash", MELEE_MAX_DISTANCE))
			{
				return true;
			}
		}
	}

	uint32 validAttackerCount = 0;
	bool groupTaunt = false;
	std::set<Unit*> groupAttackers = GetAttackersInRange(AOE_TARGETS_RANGE);
	for (std::set<Unit*>::iterator gaIT = groupAttackers.begin(); gaIT != groupAttackers.end(); gaIT++)
	{
		if (Unit* eachAttacker = *gaIT)
		{
			if (!eachAttacker->GetTargetGuid().IsEmpty())
			{
				if (eachAttacker->GetTargetGuid() != me->GetObjectGuid())
				{
					groupTaunt = true;
				}
			}
			validAttackerCount++;
		}
	}
	if (!pmSingle)
	{
		if (groupTaunt && validAttackerCount > 3)
		{
			if (rage > 150)
			{
				if (CastSpell(pmTarget, "Challenging Roar", MELEE_MAX_DISTANCE))
				{
					return true;
				}
			}
		}
		if (validAttackerCount > 1)
		{
			if (rage > 150)
			{
				if (CastSpell(pmTarget, "Swipe (Bear)", MELEE_MAX_DISTANCE))
				{
					return true;
				}
			}
		}
	}
	if (CastSpell(pmTarget, "Faerie Fire (Feral)", MELEE_MAX_DISTANCE, true))
	{
		return true;
	}
	// when facing boss 
	if (pmTarget->GetMaxHealth() / me->GetMaxHealth() > 10.0f)
	{
		if (rage > 200)
		{
			if (sRobotManager->GetAuraStack(pmTarget, "Lacerate", me) < 5)
			{
				if (CastSpell(pmTarget, "Lacerate", MELEE_MAX_DISTANCE))
				{
					return true;
				}
			}
			else if (sRobotManager->GetAuraDuration(pmTarget, "Lacerate", me) < 3000)
			{
				if (CastSpell(pmTarget, "Lacerate", MELEE_MAX_DISTANCE))
				{
					return true;
				}
			}
		}
		if (CastSpell(me, "Berserk", MELEE_MAX_DISTANCE))
		{
			me->Yell("Berserk!", Language::LANG_UNIVERSAL);
			return true;
		}
	}
	if (rage > 200)
	{
		if (CastSpell(pmTarget, "Mangle (Bear)", MELEE_MAX_DISTANCE))
		{
			return true;
		}
	}
	if (rage > 300)
	{
		if (CastSpell(pmTarget, "Maul", MELEE_MAX_DISTANCE))
		{
			return true;
		}
	}
	return true;
}

bool Script_Druid::Taunt(Unit* pmTarget)
{
	if (!pmTarget)
	{
		return false;
	}
	else if (!pmTarget->IsAlive())
	{
		return false;
	}
	if (!me)
	{
		return false;
	}
	else if (!me->IsValidAttackTarget(pmTarget))
	{
		return false;
	}
	float targetDistance = me->GetDistance(pmTarget);
	if (targetDistance > ATTACK_RANGE_LIMIT)
	{
		return false;
	}

	switch (me->GetShapeshiftForm())
	{
	case ShapeshiftForm::FORM_NONE:
	{
		if (CastSpell(me, "Dire Bear Form"))
		{
			return true;
		}
		if (CastSpell(me, "Bear Form"))
		{
			return true;
		}
		break;
	}
	case ShapeshiftForm::FORM_BEAR:
	{
		break;
	}
	case ShapeshiftForm::FORM_DIREBEAR:
	{
		break;
	}
	default:
	{
		ClearShapeshift();
		return true;
	}
	}

	CastSpell(pmTarget, "Growl", DRUID_RANGE_DISTANCE);
	return true;
}

bool Script_Druid::Attack(Unit* pmTarget)
{
	if (!me)
	{
		return false;
	}
	uint32 characterTalentTab = me->GetMaxTalentCountTab();
	switch (characterTalentTab)
	{
	case 0:
	{
		if ((me->GetPower(Powers::POWER_MANA) * 100 / me->GetMaxPower(Powers::POWER_MANA)) < 30)
		{
			UseManaPotion();
		}
		return Attack_Balance(pmTarget);
	}
	case 1:
	{
		if (me->GetHealthPercent() < 30.0f)
		{
			UseHealingPotion();
		}
		if (me->GetHealthPercent() < 20.0f)
		{
			if (CastSpell(me, "Survival Instincts"))
			{
				me->Yell("Survival Instincts!", Language::LANG_UNIVERSAL);
			}
		}
		return Attack_Feral(pmTarget);
	}
	case 2:
	{
		if ((me->GetPower(Powers::POWER_MANA) * 100 / me->GetMaxPower(Powers::POWER_MANA)) < 30)
		{
			UseManaPotion();
		}
		return Attack_Common(pmTarget);
	}
	default:
	{
		if ((me->GetPower(Powers::POWER_MANA) * 100 / me->GetMaxPower(Powers::POWER_MANA)) < 30)
		{
			UseManaPotion();
		}
		return Attack_Common(pmTarget);
	}
	}

	return false;
}

bool Script_Druid::Attack_Balance(Unit* pmTarget)
{
	if (!pmTarget)
	{
		return false;
	}
	else if (!pmTarget->IsAlive())
	{
		return false;
	}
	if (!me)
	{
		return false;
	}
	else if (!me->IsValidAttackTarget(pmTarget))
	{
		return false;
	}
	float targetDistance = me->GetDistance(pmTarget);
	if (targetDistance > ATTACK_RANGE_LIMIT)
	{
		return false;
	}
	if (me->GetShapeshiftForm() == ShapeshiftForm::FORM_NONE)
	{
		if (CastSpell(me, "Moonkin Form"))
		{
			return true;
		}
	}
	if (!Chase(pmTarget, FOLLOW_FAR_DISTANCE))
	{
		return false;
	}
	if (pmTarget->GetTargetGuid().IsEmpty())
	{
		if (CastSpell(pmTarget, "Starfire", DRUID_RANGE_DISTANCE))
		{
			return true;
		}
	}
	else
	{
		if (CastSpell(pmTarget, "Moonfire", DRUID_RANGE_DISTANCE, true, true))
		{
			return true;
		}
		if (me->GetObjectGuid() != pmTarget->GetTargetGuid())
		{
			if (CastSpell(pmTarget, "Starfire", DRUID_RANGE_DISTANCE))
			{
				return true;
			}
		}
	}
	if (CastSpell(pmTarget, "Typhoon", DRUID_RANGE_DISTANCE))
	{
		return true;
	}
	if (CastSpell(pmTarget, "Wrath", DRUID_RANGE_DISTANCE))
	{
		return true;
	}

	return true;
}

bool Script_Druid::Attack_Feral(Unit* pmTarget)
{
	if (!pmTarget)
	{
		return false;
	}
	else if (!pmTarget->IsAlive())
	{
		return false;
	}
	if (!me)
	{
		return false;
	}
	else if (!me->IsValidAttackTarget(pmTarget))
	{
		return false;
	}
	float targetDistance = me->GetDistance(pmTarget);
	if (targetDistance > ATTACK_RANGE_LIMIT)
	{
		return false;
	}
	switch (me->GetShapeshiftForm())
	{
	case ShapeshiftForm::FORM_NONE:
	{
		if (urand(0, 1) == 1)
		{
			if (CastSpell(me, "Cat Form"))
			{
				break;
			}
		}
		if (CastSpell(me, "Dire Bear Form"))
		{
			break;
		}
		if (CastSpell(me, "Bear Form"))
		{
			break;
		}
		break;
	}
	case ShapeshiftForm::FORM_CAT:
	{
		return Attack_Feral_Cat(pmTarget);
	}
	case ShapeshiftForm::FORM_DIREBEAR:
	{
		return Attack_Feral_Bear(pmTarget);
	}
	case ShapeshiftForm::FORM_BEAR:
	{
		return Attack_Feral_Bear(pmTarget);
	}
	default:
	{
		ClearShapeshift();
		return true;
	}
	}

	return true;
}

bool Script_Druid::Attack_Feral_Cat(Unit* pmTarget)
{
	if (!pmTarget)
	{
		return false;
	}
	else if (!pmTarget->IsAlive())
	{
		return false;
	}
	if (!me)
	{
		return false;
	}
	else if (!me->IsValidAttackTarget(pmTarget))
	{
		return false;
	}
	float targetDistance = me->GetDistance(pmTarget);
	if (targetDistance > ATTACK_RANGE_LIMIT)
	{
		return false;
	}
	switch (me->GetShapeshiftForm())
	{
	case ShapeshiftForm::FORM_NONE:
	{
		CastSpell(me, "Cat Form");
		break;
	}
	case ShapeshiftForm::FORM_CAT:
	{
		if (!Chase(pmTarget))
		{
			return false;
		}
		me->Attack(pmTarget, true);
		CastSpell(me, "Dash");
		if (me->GetHealthPercent() < 50.0f)
		{
			CastSpell(me, "Barkskin");
		}
		uint32 energy = me->GetPower(Powers::POWER_ENERGY);
		if (CastSpell(pmTarget, "Faerie Fire (Feral)", DRUID_RANGE_DISTANCE, true))
		{
			return true;
		}
		if (energy < 30)
		{
			if (CastSpell(me, "Tiger's Fury", MELEE_MAX_DISTANCE, true))
			{
				return true;
			}
		}
		uint8 comboPoints = me->GetComboPoints();
		if (comboPoints >= 4)
		{
			if (energy > 35)
			{
				if (CastSpell(pmTarget, "Rip", MELEE_MAX_DISTANCE, true, true))
				{
					return true;
				}
				if (CastSpell(pmTarget, "Ferocious Bite", MELEE_MAX_DISTANCE))
				{
					return true;
				}
			}
		}
		if (energy > 45)
		{
			if (CastSpell(pmTarget, "Rake", MELEE_MAX_DISTANCE, true, true))
			{
				return true;
			}
			if (CastSpell(pmTarget, "Mangle (Cat)", MELEE_MAX_DISTANCE))
			{
				return true;
			}
			if (CastSpell(pmTarget, "Claw", MELEE_MAX_DISTANCE))
			{
				return true;
			}
		}
		break;
	}
	default:
	{
		ClearShapeshift();
		break;
	}
	}

	return true;
}

bool Script_Druid::Attack_Feral_Bear(Unit* pmTarget)
{
	if (!pmTarget)
	{
		return false;
	}
	else if (!pmTarget->IsAlive())
	{
		return false;
	}
	if (!me)
	{
		return false;
	}
	else if (!me->IsValidAttackTarget(pmTarget))
	{
		return false;
	}
	float targetDistance = me->GetDistance(pmTarget);
	if (targetDistance > ATTACK_RANGE_LIMIT)
	{
		return false;
	}
	if (!Chase(pmTarget))
	{
		return false;
	}
	me->Attack(pmTarget, true);
	if (me->GetHealthPercent() < 50.0f)
	{
		CastSpell(me, "Barkskin");
	}
	CastSpell(me, "Enrage");
	uint32 rage = me->GetPower(Powers::POWER_RAGE);
	if (rage > 50)
	{
		if (CastSpell(pmTarget, "Feral Charge - Bear", DRUID_RANGE_DISTANCE))
		{
			return true;
		}
	}
	if (rage > 500)
	{
		if (me->GetHealthPercent() < 60)
		{
			if (CastSpell(me, "Frenzied Regeneration"))
			{
				return true;
			}
		}
	}
	if (CastSpell(pmTarget, "Faerie Fire (Feral)", MELEE_MAX_DISTANCE))
	{
		return true;
	}
	if (rage > 100)
	{
		if (CastSpell(pmTarget, "Demoralizing Roar", MELEE_MAX_DISTANCE, true))
		{
			return true;
		}
		if (pmTarget->IsNonMeleeSpellCasted(false))
		{
			if (CastSpell(pmTarget, "Bash", MELEE_MAX_DISTANCE))
			{
				return true;
			}
		}
	}
	if (rage > 200)
	{
		if (CastSpell(pmTarget, "Mangle (Bear)", MELEE_MAX_DISTANCE))
		{
			return true;
		}
	}
	if (rage > 250)
	{
		if (CastSpell(pmTarget, "Maul", MELEE_MAX_DISTANCE))
		{
			return true;
		}
	}

	return true;
}

bool Script_Druid::Attack_Common(Unit* pmTarget)
{
	return Attack_Balance(pmTarget);
}

bool Script_Druid::Heal(Unit* pmTarget, bool pmCure)
{
	if (!me)
	{
		return false;
	}
	if ((me->GetPower(Powers::POWER_MANA) * 100 / me->GetMaxPower(Powers::POWER_MANA)) < 30)
	{
		UseManaPotion();
	}
	uint32 characterTalentTab = me->GetMaxTalentCountTab();
	switch (characterTalentTab)
	{
	case 0:
	{
		return Heal_Restoration(pmTarget, pmCure);
	}
	case 1:
	{
		return Heal_Restoration(pmTarget, pmCure);
	}
	case 2:
	{
		return Heal_Restoration(pmTarget, pmCure);
	}
	default:
	{
		return Heal_Restoration(pmTarget, pmCure);
	}
	}
	return false;
}

bool Script_Druid::Heal_Restoration(Unit* pmTarget, bool pmCure)
{
	if (!pmTarget)
	{
		return false;
	}
	else if (!pmTarget->IsAlive())
	{
		return false;
	}
	if (!me)
	{
		return false;
	}
	if (me->GetDistance(pmTarget) > DRUID_RANGE_DISTANCE)
	{
		return false;
	}
	if ((me->GetPower(Powers::POWER_MANA) * 100 / me->GetMaxPower(Powers::POWER_MANA)) < 20)
	{
		if (CastSpell(me, "Innervate"))
		{
			return true;
		}
	}
	float healthPCT = pmTarget->GetHealthPercent();
	if (healthPCT < 60.0f)
	{
		if (me->GetShapeshiftForm() != ShapeshiftForm::FORM_NONE && me->GetShapeshiftForm() != ShapeshiftForm::FORM_TREE)
		{
			ClearShapeshift();
		}
		if (CastSpell(pmTarget, "Healing Touch", DRUID_RANGE_DISTANCE))
		{
			return true;
		}
	}
	else if (healthPCT < 80.0f)
	{
		if (me->GetShapeshiftForm() != ShapeshiftForm::FORM_NONE && me->GetShapeshiftForm() != ShapeshiftForm::FORM_TREE)
		{
			ClearShapeshift();
		}
		if (CastSpell(pmTarget, "Regrowth", DRUID_RANGE_DISTANCE))
		{
			return true;
		}
	}
	else if (healthPCT < 90.0f)
	{
		if (me->GetShapeshiftForm() != ShapeshiftForm::FORM_NONE && me->GetShapeshiftForm() != ShapeshiftForm::FORM_TREE)
		{
			ClearShapeshift();
		}
		if (CastSpell(pmTarget, "Rejuvenation", DRUID_RANGE_DISTANCE, true))
		{
			return true;
		}
	}

	if (pmCure)
	{
		if (pmTarget->HasAuraWithDispelType(DispelType::DISPEL_POISON))
		{
			if (CastSpell(pmTarget, "Abolish Poison", DRUID_RANGE_DISTANCE, false, false, true))
			{
				return true;
			}
			if (CastSpell(pmTarget, "Cure Poison", DRUID_RANGE_DISTANCE, false, false, true))
			{
				return true;
			}
		}
		if (pmTarget->HasAuraWithDispelType(DispelType::DISPEL_CURSE))
		{
			if (CastSpell(pmTarget, "Remove Curse", DRUID_RANGE_DISTANCE, false, false, true))
			{
				return true;
			}
		}
	}

	return false;
}

bool Script_Druid::Buff(Unit* pmTarget, bool pmCure)
{
	if (!pmTarget)
	{
		return false;
	}
	else if (!pmTarget->IsAlive())
	{
		return false;
	}
	if (!me)
	{
		return false;
	}
	if (me->GetDistance(pmTarget) < DRUID_RANGE_DISTANCE)
	{
		bool doThorn = false;
		switch (pmTarget->GetClass())
		{
		case Classes::CLASS_WARRIOR:
		{
			//doThorn = true;
			break;
		}
		case Classes::CLASS_HUNTER:
		{
			break;
		}
		case Classes::CLASS_SHAMAN:
		{
			break;
		}
		case Classes::CLASS_PALADIN:
		{
			//doThorn = true;
			break;
		}
		case Classes::CLASS_WARLOCK:
		{
			break;
		}
		case Classes::CLASS_PRIEST:
		{
			break;
		}
		case Classes::CLASS_ROGUE:
		{
			//doThorn = true;
			break;
		}
		case Classes::CLASS_MAGE:
		{
			break;
		}
		case Classes::CLASS_DRUID:
		{
			doThorn = true;
			break;
		}
		default:
		{
			break;
		}
		}
		if (doThorn)
		{
			if (!sRobotManager->HasAura(pmTarget, "Thorns"))
			{
				ClearShapeshift();
				if (CastSpell(pmTarget, "Thorns", DRUID_RANGE_DISTANCE, true))
				{
					return true;
				}
			}
		}

		if (!sRobotManager->HasAura(pmTarget, "Mark of the Wild") && !sRobotManager->HasAura(pmTarget, "Gift of the Wild"))
		{
			ClearShapeshift();
			if (FindSpellID("Gift of the Wild"))
			{
				if (CastSpell(pmTarget, "Gift of the Wild", DRUID_RANGE_DISTANCE, true))
				{
					return true;
				}
			}
			else
			{
				if (CastSpell(pmTarget, "Mark of the Wild", DRUID_RANGE_DISTANCE, true))
				{
					return true;
				}
			}
		}
	}

	if (pmCure)
	{
		if (pmTarget->HasAuraWithDispelType(DispelType::DISPEL_POISON))
		{
			if (CastSpell(pmTarget, "Abolish Poison", DRUID_RANGE_DISTANCE, false, false, true))
			{
				return true;
			}
			if (CastSpell(pmTarget, "Cure Poison", DRUID_RANGE_DISTANCE, false, false, true))
			{
				return true;
			}
		}
		if (pmTarget->HasAuraWithDispelType(DispelType::DISPEL_CURSE))
		{
			if (CastSpell(pmTarget, "Remove Curse", DRUID_RANGE_DISTANCE, false, false, true))
			{
				return true;
			}
		}
	}

	return false;
}
