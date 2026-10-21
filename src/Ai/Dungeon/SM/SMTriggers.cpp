/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SMTriggers.h"

#include "Playerbots.h"
#include "SMShared.h"

using namespace SMShared;

bool ArcanistDoanBossCastingDetonationTrigger::IsActive()
{
    Unit* doan = AI_VALUE2(Unit*, "find target", "arcanist doan");
    return doan && IsDetonationImminent(doan) && bot->GetExactDist2d(doan) < DETONATION_SAFE_DISTANCE;
}

bool HerodWhirlwindTrigger::IsActive()
{
    Unit* herod = AI_VALUE2(Unit*, "find target", "herod");
    return IsHerodWhirlwinding(herod) && bot->GetExactDist2d(herod) < HEROD_WHIRLWIND_SAFE_DISTANCE;
}
