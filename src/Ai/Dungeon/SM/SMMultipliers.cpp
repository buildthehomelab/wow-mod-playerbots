/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SMMultipliers.h"

#include "GenericSpellActions.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "SMActions.h"
#include "SMShared.h"
#include "SpellInfo.h"
#include "SpellMgr.h"

using namespace SMShared;

float ScarletHazardMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<ScarletAvoidanceAction*>(action) || dynamic_cast<ScarletGroundEscapeAction*>(action))
        return 1.0f;
    bool const isMovement = dynamic_cast<MovementAction*>(action) || dynamic_cast<CastReachTargetSpellAction*>(action);
    bool const isSpell = dynamic_cast<CastSpellAction*>(action) || dynamic_cast<ScarletDispelDominateAction*>(action);
    if (!isMovement && !isSpell)
        return 1.0f;
    Unit* boss = GetEncounterTarget(botAI, _encounter);
    if (!IsHazardActive(boss, _encounter))
        return 1.0f;
    if (isMovement)
        return 0.0f;
    return bot->GetExactDist2d(boss) < SafeDistance(_encounter) ? 0.0f : 1.0f;
}

float ScarletFocusMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<AttackAction*>(action) || dynamic_cast<ScarletFocusAction*>(action) ||
        dynamic_cast<MeleeAction*>(action))
        return 1.0f;
    return GetFocusTarget(botAI) ? 0.0f : 1.0f;
}

float ScarletCharmSafetyMultiplier::GetValue(Action* action)
{
    CastSpellAction* cast = dynamic_cast<CastSpellAction*>(action);
    AttackAction* attack = dynamic_cast<AttackAction*>(action);
    if (!cast && !attack)
        return 1.0f;
    GuidVector const charmed = AI_VALUE(GuidVector, "scarlet charmed party");
    if (charmed.empty())
        return 1.0f;
    if (cast)
    {
        uint32 const spellId = AI_VALUE2(uint32, "spell id", cast->getSpell());
        SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
        if (!info || info->IsPositive())
            return 1.0f;
        if (info->IsAffectingArea())
            return 0.0f;
    }
    return IsDominatedPartyMember(bot, action->GetTarget()) ? 0.0f : 1.0f;
}

float ScarletGroundDamageMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<ScarletAvoidanceAction*>(action) || dynamic_cast<AvoidAoeAction*>(action))
        return 1.0f;
    if (!dynamic_cast<MovementAction*>(action) && !dynamic_cast<CastReachTargetSpellAction*>(action) &&
        !dynamic_cast<CastSpellAction*>(action) && !dynamic_cast<ScarletDispelDominateAction*>(action))
        return 1.0f;
    return AI_VALUE(bool, "scarlet ground damage") ? 0.0f : 1.0f;
}
