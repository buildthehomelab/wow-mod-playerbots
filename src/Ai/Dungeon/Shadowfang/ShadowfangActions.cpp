/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ShadowfangActions.h"

#include <algorithm>

#include "MoveSpline.h"
#include "Pet.h"
#include "Playerbots.h"
#include "ShadowfangShared.h"
#include "Timer.h"

namespace
{
constexpr float MIN_ESCAPE_PROGRESS = 0.1f;
}

bool ShadowfangEscapeAction::isUseful() { return botAI->CanMove() && Shadowfang::InHazard(botAI); }

bool ShadowfangEscapeAction::Execute(Event /*event*/)
{
    Shadowfang::Hazard const area = Shadowfang::CollectHazard(botAI);
    if (!area.active || !botAI->CanMove())
        return false;
    float const minimumDistance = std::min(
        area.radius, area.center.GetExactDist2d(bot->GetPositionX(), bot->GetPositionY()) + MIN_ESCAPE_PROGRESS);
    if (_lastAttemptMs && bot->isMoving() && !bot->movespline->Finalized())
    {
        G3D::Vector3 const destination = bot->movespline->FinalDestination();
        if (_destination.GetExactDist(destination.x, destination.y, destination.z) < 0.5f &&
            area.center.GetExactDist2d(destination.x, destination.y) > minimumDistance)
            return true;
    }
    uint32 const now = getMSTime();
    if (_lastAttemptMs && getMSTimeDiff(_lastAttemptMs, now) < 500)
        return false;
    _lastAttemptMs = now;
    Position const planned = PlayerbotAI::IsMelee(bot) ? BestPositionForMeleeToFlee(area.center, area.radius)
                                                       : BestPositionForRangedToFlee(area.center, area.radius);
    if (planned == Position() ||
        area.center.GetExactDist2d(planned.GetPositionX(), planned.GetPositionY()) <= minimumDistance)
        return false;
    bool const castDefersMovement = bot->IsMovementPreventedByCasting();
    if (!MoveTo(bot->GetMapId(), planned.GetPositionX(), planned.GetPositionY(), planned.GetPositionZ(), false, false,
                true, false, MovementPriority::MOVEMENT_FORCED))
        return false;
    if (castDefersMovement)
    {
        // PointMovementGenerator launches the accepted path after the cast gate clears.
        bot->CastStop();
        _destination = planned;
        return true;
    }
    G3D::Vector3 const destination = bot->movespline->FinalDestination();
    if (area.center.GetExactDist2d(destination.x, destination.y) <= minimumDistance)
    {
        bot->StopMoving();
        return false;
    }
    bot->CastStop();
    _destination.Relocate(destination.x, destination.y, destination.z);
    return true;
}

bool ShadowfangDecurseAction::isUseful() { return Shadowfang::DecurseTarget(botAI) != nullptr; }
bool ShadowfangDecurseAction::isPossible()
{
    uint32 const spellId = Shadowfang::DecurseSpell(botAI);
    Unit* target = Shadowfang::DecurseTarget(botAI);
    return Shadowfang::CanDecurse(botAI, target, spellId) && botAI->CanCastSpell(spellId, target);
}
bool ShadowfangDecurseAction::Execute(Event /*event*/)
{
    uint32 const spellId = Shadowfang::DecurseSpell(botAI);
    Unit* target = Shadowfang::DecurseTarget(botAI);
    return Shadowfang::CanDecurse(botAI, target, spellId) && botAI->CastSpell(spellId, target);
}
bool ShadowfangProtectAction::isUseful() { return Shadowfang::NeedsProtection(botAI); }
bool ShadowfangProtectAction::Execute(Event /*event*/)
{
    if (!Shadowfang::Enabled(botAI))
        return false;
    bool changed = false;
    if (Shadowfang::UnsafeCast(botAI, bot))
    {
        bot->CastStop();
        changed = true;
    }
    if (Shadowfang::IsCursedAlly(botAI, bot->GetVictim()) ||
        Shadowfang::IsCursedAlly(botAI, AI_VALUE(Unit*, "current target")))
    {
        bot->AttackStop();
        context->GetValue<Unit*>("current target")->Set(nullptr);
        bot->SetSelection(ObjectGuid::Empty);
        changed = true;
    }
    if (Pet* pet = bot->GetPet())
        if (Shadowfang::UnsafeCast(botAI, pet) || Shadowfang::IsCursedAlly(botAI, pet->GetVictim()))
        {
            if (Shadowfang::UnsafeCast(botAI, pet))
                pet->CastStop();
            pet->AttackStop();
            changed = true;
        }
    return changed;
}
