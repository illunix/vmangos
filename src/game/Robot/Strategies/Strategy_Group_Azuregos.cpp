#include "Strategy_Group_Azuregos.h"
#include "Script_Warrior.h"
#include "Script_Hunter.h"
#include "Script_Shaman.h"
#include "Script_Paladin.h"
#include "Script_Warlock.h"
#include "Script_Priest.h"
#include "Script_Rogue.h"
#include "Script_Mage.h"
#include "Script_Druid.h"
#include "RobotConfig.h"
#include "RobotManager.h"
#include "Group.h"
#include "MotionMaster.h"
#include "TargetedMovementGenerator.h"
#include "GridNotifiers.h"
#include "Map.h"
#include "Pet.h"
#include "Spell.h"

void Strategy_Group_Azuregos::InitialStrategy()
{
	engageAngle = 0.0f;
	engageDistance = 0.0f;
	Strategy_Group::InitialStrategy();
}

std::string Strategy_Group_Azuregos::GetGroupRoleName()
{
	if (!me)
	{
		return "";
	}
	switch (me->groupRole)
	{
	case GroupRole_Azuregos::GroupRole_Azuregos_Tank:
	{
		return "tank";
	}
	case GroupRole_Azuregos::GroupRole_Azuregos_Healer1:
	{
		return "healer1";
	}
	case GroupRole_Azuregos::GroupRole_Azuregos_Healer2:
	{
		return "healer2";
	}
	case GroupRole_Azuregos::GroupRole_Azuregos_DPS:
	{
		return "dps";
	}
	default:
	{
		break;
	}
	}
	return "dps";
}

void Strategy_Group_Azuregos::SetGroupRole(std::string pmRoleName)
{
	if (!me)
	{
		return;
	}
	else if (pmRoleName == "tank")
	{
		me->groupRole = GroupRole_Azuregos::GroupRole_Azuregos_Tank;
	}
	else if (pmRoleName == "healer1")
	{
		me->groupRole = GroupRole_Azuregos::GroupRole_Azuregos_Healer1;
	}
	else if (pmRoleName == "healer2")
	{
		me->groupRole = GroupRole_Azuregos::GroupRole_Azuregos_Healer2;
	}
	else if (pmRoleName == "dps")
	{
		me->groupRole = GroupRole_Azuregos::GroupRole_Azuregos_DPS;
	}
	else
	{
		me->groupRole = GroupRole_Azuregos::GroupRole_Azuregos_DPS;
	}
}

bool Strategy_Group_Azuregos::Stay(std::string pmTargetGroupRole)
{
	if (!me)
	{
		return false;
	}
	bool todo = true;
	if (pmTargetGroupRole == "dps")
	{
		if (me->groupRole != GroupRole_Azuregos::GroupRole_Azuregos_DPS)
		{
			todo = false;
		}
	}
	else if (pmTargetGroupRole == "healer")
	{
		if (me->groupRole != GroupRole_Azuregos::GroupRole_Azuregos_Healer1 && me->groupRole != GroupRole_Azuregos::GroupRole_Azuregos_Healer2)
		{
			todo = false;
		}
	}
	else if (pmTargetGroupRole == "tank")
	{
		if (me->groupRole != GroupRole_Azuregos::GroupRole_Azuregos_Tank)
		{
			todo = false;
		}
	}
	if (todo)
	{
		me->StopMoving();
		me->GetMotionMaster()->Clear();
		me->AttackStop();
		me->InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
		sb->PetStop();
		staying = true;
		return true;
	}
	return false;
}

bool Strategy_Group_Azuregos::Hold(std::string pmTargetGroupRole)
{
	if (!me)
	{
		return false;
	}
	bool todo = true;
	if (pmTargetGroupRole == "dps")
	{
		if (me->groupRole != GroupRole_Azuregos::GroupRole_Azuregos_DPS)
		{
			todo = false;
		}
	}
	else if (pmTargetGroupRole == "healer")
	{
		if (me->groupRole != GroupRole_Azuregos::GroupRole_Azuregos_Healer1 && me->groupRole != GroupRole_Azuregos::GroupRole_Azuregos_Healer2)
		{
			todo = false;
		}
	}
	else if (pmTargetGroupRole == "tank")
	{
		if (me->groupRole != GroupRole_Azuregos::GroupRole_Azuregos_Tank)
		{
			todo = false;
		}
	}
	if (todo)
	{
		holding = true;
		staying = false;
		return true;
	}
	return false;
}

bool Strategy_Group_Azuregos::Engage(Unit* pmTarget)
{
	if (!me)
	{
		return false;
	}
	if (!me->IsAlive())
	{
		return false;
	}
	switch (me->groupRole)
	{
	case GroupRole_Azuregos::GroupRole_Azuregos_Tank:
	{
		return sb->Tank(pmTarget, Chasing());
	}
	case GroupRole_Azuregos::GroupRole_Azuregos_Healer1:
	{
		return Heal();
	}
	case GroupRole_Azuregos::GroupRole_Azuregos_Healer2:
	{
		return Heal();
	}
	case GroupRole_Azuregos::GroupRole_Azuregos_DPS:
	{
		return DPS();
	}
	default:
	{
		return DPS();
	}
	}

	return false;
}

bool Strategy_Group_Azuregos::Follow()
{
	if (!me)
	{
		return false;
	}
	if (!me->IsAlive())
	{
		return false;
	}
	if (holding)
	{
		return false;
	}
	if (!following)
	{
		return false;
	}
	if (Group* myGroup = me->GetGroup())
	{
		if (Player* leader = ObjectAccessor::FindPlayer(myGroup->GetLeaderGuid()))
		{
			return sb->Follow(leader, followDistance);
		}
	}
	return false;
}

void Strategy_Group_Azuregos::Update(uint32 pmDiff)
{
	Strategy_Base::Update(pmDiff);
	if (!Update0(pmDiff))
	{
		return;
	}
	if (me->IsAlive())
	{
		if (me->HasAura(21098))
		{
			if (!me->HasAura(11359))
			{
				if (!me->HasItemCount(9030, 1))
				{
					me->StoreNewItemInBestSlots(9030, 20);
				}
				if (Item* pPotion = sb->GetItemInInventory(9030))
				{
					sb->UseItem(pPotion, me);
				}
			}
		}
	}
	if (Group* myGroup = me->GetGroup())
	{
		if (actionDelay > 0)
		{
			actionDelay -= pmDiff;
			switch (actionType)
			{
			case ActionType_Azuregos::ActionType_Azuregos_MarkMove:
			{
				if (me->GetDistance2d(markPos) < 0.5f)
				{
					actionDelay = 0;
				}
				break;
			}
			default:
			{
				break;
			}
			}
			return;
		}
		bool groupInCombat = GroupInCombat();
		if (groupInCombat)
		{
			eatDelay = 0;
			combatTime += pmDiff;
		}
		else
		{
			combatTime = 0;
		}
		if (engageDelay > 0)
		{
			engageDelay -= pmDiff;
			switch (me->groupRole)
			{
			case GroupRole_Azuregos::GroupRole_Azuregos_Tank:
			{
				if (sb->Tank(engageTarget, Chasing()))
				{
					return;
				}
				else
				{
					engageTarget = NULL;
					engageDelay = 0;
				}
				break;
			}
			case GroupRole_Azuregos::GroupRole_Azuregos_Healer1:
			{
				if (Heal())
				{
					return;
				}
				break;
			}
			case GroupRole_Azuregos::GroupRole_Azuregos_Healer2:
			{
				if (Heal())
				{
					return;
				}
				break;
			}
			case GroupRole_Azuregos::GroupRole_Azuregos_DPS:
			{
				if (sb->DPS(engageTarget, Chasing()))
				{
					return;
				}
				else
				{
					engageTarget = NULL;
					engageDelay = 0;
				}
				break;
			}
			default:
			{
				break;
			}
			}
			return;
		}
		if (groupInCombat)
		{
			if (sb->Assist())
			{
				return;
			}
			switch (me->groupRole)
			{
			case GroupRole_Azuregos::GroupRole_Azuregos_Tank:
			{
				if (Tank())
				{
					return;
				}
				break;
			}
			case GroupRole_Azuregos::GroupRole_Azuregos_Healer1:
			{
				if (Heal())
				{
					return;
				}
				break;
			}
			case GroupRole_Azuregos::GroupRole_Azuregos_Healer2:
			{
				if (Heal())
				{
					return;
				}
				break;
			}
			case GroupRole_Azuregos::GroupRole_Azuregos_DPS:
			{
				if (DPS())
				{
					return;
				}
				break;
			}
			default:
			{
				break;
			}
			}
		}
		else
		{
			if (eatDelay > 0)
			{
				eatDelay -= pmDiff;
				return;
			}
			switch (me->groupRole)
			{
			case GroupRole_Azuregos::GroupRole_Azuregos_Tank:
			{
				if (Rest())
				{
					return;
				}
				if (Buff())
				{
					return;
				}
				break;
			}
			case GroupRole_Azuregos::GroupRole_Azuregos_Healer1:
			{
				if (Rest())
				{
					return;
				}
				if (Heal())
				{
					return;
				}
				if (Buff())
				{
					return;
				}
				break;
			}
			case GroupRole_Azuregos::GroupRole_Azuregos_Healer2:
			{
				if (Rest())
				{
					return;
				}
				if (Heal())
				{
					return;
				}
				if (Buff())
				{
					return;
				}
				break;
			}
			case GroupRole_Azuregos::GroupRole_Azuregos_DPS:
			{
				if (Rest())
				{
					return;
				}
				if (Buff())
				{
					return;
				}
				break;
			}
			default:
			{
				break;
			}
			}
		}
		Follow();
	}
}

bool Strategy_Group_Azuregos::DPS()
{
	if (!me->IsAlive())
	{
		return true;
	}
	if (Group* myGroup = me->GetGroup())
	{
		if (Unit* boss = myGroup->GetGroupAttacker(CreatureEntry_RobotStrategy::CreatureEntry_Strategy_Azuregos))
		{
			bool myPositionValid = true;
			if (me->GetDistance2dToCenter(boss) < 5.0f)
			{
				myPositionValid = false;
			}
			else
			{
				if (Player* bossTarget = ObjectAccessor::FindPlayer(boss->GetTargetGuid()))
				{
					if (bossTarget->groupRole == GroupRole_Azuregos::GroupRole_Azuregos_Tank)
					{
						if (AngleInRange(basePos.o, boss->GetOrientation(), ANGLE_RANGE))
						{
							float myBossAngle = boss->GetAngle(me);
							float myBossDistance = me->GetDistance2dToCenter(boss);
							if (!AngleInRange(myBossAngle, engageAngle, M_PI / 2))
							{
								myPositionValid = false;
							}
							else if (myBossDistance < engageDistance - 2.0f || myBossDistance > engageDistance + 2.0f)
							{
								myPositionValid = false;
							}
						}
					}
				}
			}
			if (!myPositionValid)
			{
				markPos = GetNearPoint(boss->GetPosition(), engageDistance, engageAngle);
				actionDelay = 3000;
				actionType = ActionType_Azuregos::ActionType_Azuregos_MarkMove;
				me->InterruptSpell(CurrentSpellTypes::CURRENT_AUTOREPEAT_SPELL);
				me->InterruptSpell(CurrentSpellTypes::CURRENT_CHANNELED_SPELL);
				me->InterruptSpell(CurrentSpellTypes::CURRENT_GENERIC_SPELL);
				me->InterruptSpell(CurrentSpellTypes::CURRENT_MELEE_SPELL);
				me->GetMotionMaster()->MovePoint(1, markPos.x, markPos.y, markPos.z, MoveOptions::MOVE_PATHFINDING | MoveOptions::MOVE_RUN_MODE, 0.0f, me->GetAngle(boss));
				return true;
			}
			if (boss->IsNonMeleeSpellCasted(false))
			{
				if (Spell* castingSpell = boss->GetCurrentSpell(CurrentSpellTypes::CURRENT_GENERIC_SPELL))
				{
					if (castingSpell->m_spellInfo->Id == 21097)
					{
						if (sb->InterruptCasting(boss))
						{
							me->Yell("Mana storm interrupted!", Language::LANG_UNIVERSAL);
							return true;
						}
					}
				}
			}
			if (combatTime > dpsDelay)
			{
				if (boss->HasAura(22067))
				{
					me->InterruptSpell(CurrentSpellTypes::CURRENT_AUTOREPEAT_SPELL);
					me->InterruptSpell(CurrentSpellTypes::CURRENT_CHANNELED_SPELL);
					me->InterruptSpell(CurrentSpellTypes::CURRENT_GENERIC_SPELL);
					me->InterruptSpell(CurrentSpellTypes::CURRENT_MELEE_SPELL);
					me->AttackStop();
				}
				else
				{
					if (Player* bossTarget = ObjectAccessor::FindPlayer(boss->GetTargetGuid()))
					{
						if (bossTarget->groupRole == GroupRole_Azuregos::GroupRole_Azuregos_Tank)
						{
							sb->DPS(boss, false);
						}
					}
				}
			}
			return true;
		}
	}

	return Strategy_Group::DPS();
}

bool Strategy_Group_Azuregos::Tank()
{
	if (Group* myGroup = me->GetGroup())
	{
		if (Unit* boss = myGroup->GetGroupAttacker(CreatureEntry_RobotStrategy::CreatureEntry_Strategy_Azuregos))
		{
			if (me->IsAlive())
			{
				float myBossDistance = me->GetDistance2dToCenter(boss);
				if (myBossDistance < 22.0f)
				{
					if (boss->GetTargetGuid() == me->GetObjectGuid())
					{
						bool positionValid = true;
						if (myBossDistance < 13.0f)
						{
							positionValid = false;
						}
						else if (!AngleInRange(basePos.o, boss->GetOrientation(), ANGLE_RANGE))
						{
							positionValid = false;
						}
						if (!positionValid)
						{
							markPos = GetNearPoint(boss->GetPosition(), 14.0f, basePos.o);
							actionDelay = 3000;
							actionType = ActionType_Azuregos::ActionType_Azuregos_MarkMove;
							me->InterruptSpell(CurrentSpellTypes::CURRENT_AUTOREPEAT_SPELL);
							me->InterruptSpell(CurrentSpellTypes::CURRENT_CHANNELED_SPELL);
							me->InterruptSpell(CurrentSpellTypes::CURRENT_GENERIC_SPELL);
							me->InterruptSpell(CurrentSpellTypes::CURRENT_MELEE_SPELL);
							me->GetMotionMaster()->MovePoint(1, markPos.x, markPos.y, markPos.z, MoveOptions::MOVE_PATHFINDING | MoveOptions::MOVE_RUN_MODE, 0.0f, me->GetAngle(boss));
							return true;
						}
					}
				}
				sb->Taunt(boss);
				sb->Tank(boss, false);
			}
			return true;
		}
	}
	return Strategy_Group::Tank();
}

bool Strategy_Group_Azuregos::Tank(Unit* pmTarget)
{
	if (!me)
	{
		return false;
	}
	if (!me->IsAlive())
	{
		return false;
	}
	switch (me->groupRole)
	{
	case GroupRole_Azuregos::GroupRole_Azuregos_Tank:
	{
		sb->ClearTarget();
		sb->ChooseTarget(pmTarget);
		return sb->Tank(pmTarget, Chasing());
	}
	default:
	{
		break;
	}
	}

	return false;
}

bool Strategy_Group_Azuregos::Heal()
{
	if (!me->IsAlive())
	{
		return false;
	}
	if (Group* myGroup = me->GetGroup())
	{
		if (Unit* boss = myGroup->GetGroupAttacker(CreatureEntry_RobotStrategy::CreatureEntry_Strategy_Azuregos))
		{
			bool myPositionValid = true;
			if (me->GetDistance2dToCenter(boss) < 5.0f)
			{
				myPositionValid = false;
			}
			else
			{
				if (Player* bossTarget = ObjectAccessor::FindPlayer(boss->GetTargetGuid()))
				{
					if (bossTarget->groupRole == GroupRole_Azuregos::GroupRole_Azuregos_Tank)
					{
						if (AngleInRange(basePos.o, boss->GetOrientation(), ANGLE_RANGE))
						{
							float myBossAngle = boss->GetAngle(me);
							float myBossDistance = me->GetDistance2dToCenter(boss);
							if (!AngleInRange(myBossAngle, engageAngle, M_PI / 2))
							{
								myPositionValid = false;
							}
							else if (myBossDistance < engageDistance - 2.0f || myBossDistance > engageDistance + 2.0f)
							{
								myPositionValid = false;
							}
						}
					}
				}
			}
			if (!myPositionValid)
			{
				markPos = GetNearPoint(boss->GetPosition(), engageDistance, engageAngle);
				actionDelay = 3000;
				actionType = ActionType_Azuregos::ActionType_Azuregos_MarkMove;
				me->InterruptSpell(CurrentSpellTypes::CURRENT_AUTOREPEAT_SPELL);
				me->InterruptSpell(CurrentSpellTypes::CURRENT_CHANNELED_SPELL);
				me->InterruptSpell(CurrentSpellTypes::CURRENT_GENERIC_SPELL);
				me->InterruptSpell(CurrentSpellTypes::CURRENT_MELEE_SPELL);
				me->GetMotionMaster()->MovePoint(1, markPos.x, markPos.y, markPos.z, MoveOptions::MOVE_PATHFINDING | MoveOptions::MOVE_RUN_MODE, 0.0f, me->GetAngle(boss));
				return true;
			}
			switch (me->groupRole)
			{
			case GroupRole_Azuregos::GroupRole_Azuregos_Healer1:
			{
				if (Player* tank = GetPlayerByGroupRole(GroupRole_Azuregos::GroupRole_Azuregos_Tank))
				{
					if (tank->IsAlive())
					{
						if (tank->GetHealthPercent() < 90.0f)
						{
							if (sb->Heal(tank, true))
							{
								return true;
							}
						}
					}
				}
				if (me->GetHealthPercent() < 50.0f)
				{
					if (sb->Heal(me, true))
					{
						return true;
					}
				}
				break;
			}
			case GroupRole_Azuregos::GroupRole_Azuregos_Healer2:
			{
				for (GroupReference* groupRef = myGroup->GetFirstMember(); groupRef != nullptr; groupRef = groupRef->next())
				{
					if (Player* member = groupRef->getSource())
					{
						if (member->IsAlive())
						{
							if (member->groupRole == GroupRole_Azuregos::GroupRole_Azuregos_Tank)
							{
								continue;
							}
							if (member->GetHealthPercent() < 60.0f)
							{
								if (sb->Heal(member, true))
								{
									return true;
								}
							}
						}
					}
				}
				break;
			}
			default:
			{
				break;
			}
			}
			return true;
		}
	}

	return Strategy_Group::Heal();
}
