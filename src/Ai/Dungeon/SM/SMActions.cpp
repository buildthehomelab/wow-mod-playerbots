/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SMActions.h"

#include "LastMovementValue.h"
#include "Playerbots.h"
#include "SMShared.h"

using namespace SMShared;

bool ArcanistDoanMoveOutOfDetonationAction::Execute(Event /*event*/)
{
    Unit* doan = AI_VALUE2(Unit*, "find target", "arcanist doan");
    if (!doan)
        return false;

    float const distance = bot->GetExactDist2d(doan);
    if (distance >= DETONATION_SAFE_DISTANCE)
        return false;

    bot->CastStop();
    return MoveAway(doan, DETONATION_SAFE_DISTANCE - distance);
}

bool HerodWhirlwindEscapeAction::isUseful()
{
    Unit* herod = AI_VALUE2(Unit*, "find target", "herod");
    return IsHerodWhirlwinding(herod) && bot->GetExactDist2d(herod) < HEROD_WHIRLWIND_SAFE_DISTANCE;
}

bool HerodWhirlwindEscapeAction::Execute(Event /*event*/)
{
    Unit* herod = AI_VALUE2(Unit*, "find target", "herod");
    if (!IsHerodWhirlwinding(herod))
        return false;

    float const distance = bot->GetExactDist2d(herod);
    if (distance >= HEROD_WHIRLWIND_SAFE_DISTANCE)
        return false;

    bot->CastStop();
    AI_VALUE(LastMovement&, "last movement").clear();
    return MoveAway(herod, HEROD_WHIRLWIND_SAFE_DISTANCE - distance);
}
