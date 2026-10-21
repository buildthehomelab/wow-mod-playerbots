/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SMTriggers.h"

#include "Pet.h"
#include "Playerbots.h"
#include "SMShared.h"

using namespace SMShared;

bool ScarletHazardTrigger::IsActive()
{
    Unit* boss = GetEncounterTarget(botAI, _encounter);
    return IsHazardActive(boss, _encounter) && bot->GetExactDist2d(boss) < SafeDistance(_encounter);
}

bool ScarletFocusTrigger::IsActive()
{
    Unit* target = GetFocusTarget(botAI);
    return target && AI_VALUE(Unit*, "current target") != target;
}

bool ScarletDominateTrigger::IsActive() { return bot->GetMapId() == SM_MAP_ID && GetDominatedAlly(botAI); }

bool ScarletProtectAllyTrigger::IsActive()
{
    Pet* pet = bot->GetPet();
    return IsCastingAtDominatedAlly(botAI, bot) || IsCastingAtDominatedAlly(botAI, pet) ||
           IsDominatedPartyMember(bot, bot->GetVictim()) ||
           IsDominatedPartyMember(bot, AI_VALUE(Unit*, "current target")) ||
           (pet && IsDominatedPartyMember(bot, pet->GetVictim()));
}

bool ScarletGroundDamageTrigger::IsActive()
{
    return bot->GetMapId() == SM_MAP_ID && AI_VALUE(bool, "scarlet ground damage");
}
