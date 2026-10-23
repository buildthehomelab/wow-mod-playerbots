/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "WailingCavernsActions.h"

#include "Playerbots.h"
#include "WailingCavernsShared.h"

Unit* WailingCavernsFocusAction::ValidatedTarget()
{
    Unit* target = WailingCaverns::GetFocus(botAI);
    if (target && !WailingCaverns::CanFocus(botAI, target))
    {
        context->GetValue<ObjectGuid>("wailing caverns focus")->Set(ObjectGuid::Empty);
        return nullptr;
    }
    return target;
}

bool WailingCavernsFocusAction::isUseful()
{
    Unit* target = ValidatedTarget();
    return target && AI_VALUE(Unit*, "current target") != target;
}

bool WailingCavernsFocusAction::Execute(Event /*event*/)
{
    Unit* target = ValidatedTarget();
    return target && Attack(target);
}
