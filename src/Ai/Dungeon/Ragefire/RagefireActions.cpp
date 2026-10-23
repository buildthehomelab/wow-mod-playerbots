/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RagefireActions.h"

#include <algorithm>

#include "MoveSpline.h"
#include "Playerbots.h"
#include "RagefireShared.h"
#include "Timer.h"

namespace
{
constexpr float MIN_ESCAPE_PROGRESS = 0.1f;
}

Unit* RagefireFocusAction::ValidatedTarget()
{
    Unit* target = Ragefire::GetFocus(botAI);
    if (target && !Ragefire::CanFocus(botAI, target))
    {
        context->GetValue<ObjectGuid>("ragefire focus")->Set(ObjectGuid::Empty);
        return nullptr;
    }
    return target;
}

bool RagefireFocusAction::isUseful()
{
    Unit* target = ValidatedTarget();
    return target && AI_VALUE(Unit*, "current target") != target;
}

bool RagefireFocusAction::Execute(Event /*event*/)
{
    Unit* target = ValidatedTarget();
    return target && Attack(target);
}

bool RagefireEscapeAction::isUseful() { return botAI->CanMove() && Ragefire::InHazard(botAI); }

bool RagefireEscapeAction::Execute(Event /*event*/)
{
    Ragefire::Hazard const area = Ragefire::CollectHazard(botAI);
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
