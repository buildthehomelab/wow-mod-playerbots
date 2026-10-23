/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GnomereganActions.h"

#include "GameObject.h"
#include "GnomereganShared.h"
#include "MoveSpline.h"
#include "Playerbots.h"
#include "Timer.h"

Unit* GnomereganFocusAction::ValidatedTarget()
{
    Unit* target = Gnomeregan::GetFocus(botAI);
    if (target && !Gnomeregan::CanFocus(botAI, target))
    {
        context->GetValue<ObjectGuid>("gnomeregan focus")->Set(ObjectGuid::Empty);
        return nullptr;
    }
    return target;
}

bool GnomereganFocusAction::isUseful()
{
    Unit* target = ValidatedTarget();
    return target && AI_VALUE(Unit*, "current target") != target;
}

bool GnomereganFocusAction::Execute(Event /*event*/)
{
    Unit* target = ValidatedTarget();
    return target && Attack(target);
}

bool GnomereganEscapeAction::isUseful() { return botAI->CanMove() && Gnomeregan::InHazard(botAI); }

bool GnomereganEscapeAction::Execute(Event /*event*/)
{
    if (!Gnomeregan::Enabled(botAI) || !botAI->CanMove())
        return false;
    auto const hazards = Gnomeregan::CollectHazards(botAI);
    Position const from = bot->GetPosition();
    if (_lastAttemptMs && bot->isMoving() && !bot->movespline->Finalized())
    {
        G3D::Vector3 const destination = bot->movespline->FinalDestination();
        Position const to(destination.x, destination.y, destination.z);
        if (_destination.GetExactDist(to) < 0.5f && Gnomeregan::SafeProgress(hazards, from, to))
            return true;
    }
    uint32 const now = getMSTime();
    if (_lastAttemptMs && getMSTimeDiff(_lastAttemptMs, now) < 500)
        return false;
    _lastAttemptMs = now;
    for (Gnomeregan::Hazard const& area : hazards)
    {
        if (area.center.GetExactDist2d(from.GetPositionX(), from.GetPositionY()) >= area.radius)
            continue;
        Position const planned = PlayerbotAI::IsMelee(bot) ? BestPositionForMeleeToFlee(area.center, area.radius)
                                                           : BestPositionForRangedToFlee(area.center, area.radius);
        if (planned == Position() || !Gnomeregan::SafeProgress(hazards, from, planned))
            continue;
        bool const castDefersMovement = bot->IsMovementPreventedByCasting();
        if (!MoveTo(bot->GetMapId(), planned.GetPositionX(), planned.GetPositionY(), planned.GetPositionZ(), false,
                    false, true, false, MovementPriority::MOVEMENT_FORCED))
            return false;
        if (castDefersMovement)
        {
            bot->CastStop();
            _destination = planned;
            return true;
        }
        G3D::Vector3 const destination = bot->movespline->FinalDestination();
        Position const to(destination.x, destination.y, destination.z);
        if (!Gnomeregan::SafeProgress(hazards, from, to))
        {
            bot->StopMoving();
            return false;
        }
        bot->CastStop();
        _destination = to;
        return true;
    }
    return false;
}

bool GnomereganButtonAction::isUseful()
{
    return Gnomeregan::GetButton(botAI, AI_VALUE(ObjectGuid, "gnomeregan button")) != nullptr;
}

bool GnomereganButtonAction::Execute(Event /*event*/)
{
    GameObject* button = Gnomeregan::GetButton(botAI, AI_VALUE(ObjectGuid, "gnomeregan button"));
    if (!button || Gnomeregan::InHazard(botAI) ||
        !Gnomeregan::SafeProgress(Gnomeregan::CollectHazards(botAI), bot->GetPosition(), button->GetPosition(), false))
        return false;
    if (!button->IsWithinDistInMap(bot, INTERACTION_DISTANCE))
        return MoveTo(bot->GetMapId(), button->GetPositionX(), button->GetPositionY(), button->GetPositionZ());
    button->Use(bot);
    context->GetValue<ObjectGuid>("gnomeregan button")->Reset();
    return true;
}
