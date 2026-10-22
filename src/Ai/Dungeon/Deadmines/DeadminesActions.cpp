/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "DeadminesActions.h"

#include "DeadminesShared.h"
#include "LastMovementValue.h"
#include "MoveSpline.h"
#include "Playerbots.h"
#include "Timer.h"

Unit* DeadminesFocusAction::ValidatedTarget()
{
    Unit* target = Deadmines::GetFocus(botAI);
    if (target && !Deadmines::CanFocus(botAI, target))
    {
        context->GetValue<ObjectGuid>("deadmines focus")->Set(ObjectGuid::Empty);
        return nullptr;
    }
    return target;
}

bool DeadminesFocusAction::isUseful()
{
    Unit* target = ValidatedTarget();
    return target && AI_VALUE(Unit*, "current target") != target;
}

bool DeadminesFocusAction::Execute(Event /*event*/)
{
    Unit* target = ValidatedTarget();
    return target && Attack(target);
}

bool DeadminesEscapeAction::isUseful() { return Deadmines::InHazard(botAI); }

bool DeadminesEscapeAction::Execute(Event /*event*/)
{
    Deadmines::Hazard const area = Deadmines::CollectHazard(botAI);
    if (!area.active)
        return false;
    if (_lastAttemptMs && bot->isMoving() && !bot->movespline->Finalized())
    {
        G3D::Vector3 const destination = bot->movespline->FinalDestination();
        if (_destination.GetExactDist(destination.x, destination.y, destination.z) < 0.5f &&
            area.center.GetExactDist2d(destination.x, destination.y) > area.radius)
            return true;
    }
    uint32 const now = getMSTime();
    if (_lastAttemptMs && getMSTimeDiff(_lastAttemptMs, now) < 500)
        return false;
    _lastAttemptMs = now;
    bot->CastStop();
    AI_VALUE(LastMovement&, "last movement").clear();
    if (!FleePosition(area.center, area.radius, 0))
        return false;
    G3D::Vector3 const destination = bot->movespline->FinalDestination();
    if (area.center.GetExactDist2d(destination.x, destination.y) <= area.radius)
    {
        bot->StopMoving();
        return false;
    }
    _destination.Relocate(destination.x, destination.y, destination.z);
    return true;
}
