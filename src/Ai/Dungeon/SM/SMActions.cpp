/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SMActions.h"

#include "LastMovementValue.h"
#include "MoveSpline.h"
#include "Pet.h"
#include "Playerbots.h"
#include "SMShared.h"
#include "Timer.h"

using namespace SMShared;

bool ScarletAvoidanceAction::isUseful()
{
    Unit* boss = GetEncounterTarget(botAI, _encounter);
    return IsHazardActive(boss, _encounter) && bot->GetExactDist2d(boss) < SafeDistance(_encounter);
}

bool ScarletAvoidanceAction::Execute(Event /*event*/)
{
    Unit* boss = GetEncounterTarget(botAI, _encounter);
    if (!IsHazardActive(boss, _encounter))
        return false;
    float const distance = bot->GetExactDist2d(boss);
    float const safeDistance = SafeDistance(_encounter);
    if (distance >= safeDistance)
        return false;

    if (_escapeBoss == boss->GetGUID() && bot->isMoving() && !bot->movespline->Finalized())
    {
        G3D::Vector3 const destination = bot->movespline->FinalDestination();
        if (_escapeDestination.GetExactDist(destination.x, destination.y, destination.z) < 0.5f)
            return true;
    }

    uint32 const now = getMSTime();
    if (_escapeBoss == boss->GetGUID() && _lastAttemptMs && getMSTimeDiff(_lastAttemptMs, now) < 500)
        return false;
    _escapeBoss = boss->GetGUID();
    _lastAttemptMs = now;
    bot->CastStop();
    AI_VALUE(LastMovement&, "last movement").clear();
    if (!MoveAway(boss, safeDistance - distance))
        return false;

    G3D::Vector3 const destination = bot->movespline->FinalDestination();
    _escapeDestination.Relocate(destination.x, destination.y, destination.z);
    return true;
}

bool ScarletFocusAction::isUseful()
{
    Unit* target = GetFocusTarget(botAI);
    return target && AI_VALUE(Unit*, "current target") != target && CanFocusTarget(botAI, target);
}

bool ScarletFocusAction::Execute(Event /*event*/)
{
    Unit* target = GetFocusTarget(botAI);
    return target && CanFocusTarget(botAI, target) && Attack(target);
}

bool ScarletDispelDominateAction::isUseful() { return bot->GetMapId() == SM_MAP_ID && GetDominatedAlly(botAI); }

bool ScarletDispelDominateAction::isPossible()
{
    uint32 const spellId = CharmDispelSpell(botAI);
    Unit* target = GetDominatedAlly(botAI);
    return CanDispelDominate(botAI, target, spellId) && botAI->CanCastSpell(spellId, target);
}

bool ScarletDispelDominateAction::Execute(Event /*event*/)
{
    uint32 const spellId = CharmDispelSpell(botAI);
    Unit* target = GetDominatedAlly(botAI);
    return CanDispelDominate(botAI, target, spellId) && botAI->CastSpell(spellId, target);
}

bool ScarletProtectAllyAction::isUseful()
{
    Pet* pet = bot->GetPet();
    return IsCastingAtDominatedAlly(botAI, bot) || IsCastingAtDominatedAlly(botAI, pet) ||
           IsDominatedPartyMember(bot, bot->GetVictim()) ||
           IsDominatedPartyMember(bot, AI_VALUE(Unit*, "current target")) ||
           (pet && IsDominatedPartyMember(bot, pet->GetVictim()));
}

bool ScarletProtectAllyAction::Execute(Event /*event*/)
{
    bool changed = false;
    if (IsCastingAtDominatedAlly(botAI, bot))
    {
        bot->CastStop();
        changed = true;
    }
    if (IsDominatedPartyMember(bot, bot->GetVictim()) || IsDominatedPartyMember(bot, AI_VALUE(Unit*, "current target")))
    {
        bot->CastStop();
        bot->AttackStop();
        context->GetValue<Unit*>("current target")->Set(nullptr);
        bot->SetSelection(ObjectGuid::Empty);
        changed = true;
    }
    if (Pet* pet = bot->GetPet())
        if (IsDominatedPartyMember(bot, pet->GetVictim()) || IsCastingAtDominatedAlly(botAI, pet))
        {
            pet->CastStop();
            pet->AttackStop();
            changed = true;
        }
    return changed;
}

bool ScarletGroundEscapeAction::isUseful()
{
    return bot->GetMapId() == SM_MAP_ID && AI_VALUE(bool, "scarlet ground damage");
}

bool ScarletGroundEscapeAction::Execute(Event /*event*/)
{
    if (!IsInGroundDamage(botAI))
        return false;
    if (_lastAttemptMs && bot->isMoving() && !bot->movespline->Finalized())
    {
        G3D::Vector3 const destination = bot->movespline->FinalDestination();
        if (_destination.GetExactDist(destination.x, destination.y, destination.z) < 0.5f)
            return true;
    }
    uint32 const now = getMSTime();
    if (_lastAttemptMs && getMSTimeDiff(_lastAttemptMs, now) < 1000)
        return false;
    _lastAttemptMs = now;
    bot->CastStop();
    AI_VALUE(LastMovement&, "last movement").clear();
    if (!AvoidAuraWithDynamicObj())
        return false;
    G3D::Vector3 const destination = bot->movespline->FinalDestination();
    _destination.Relocate(destination.x, destination.y, destination.z);
    return true;
}
