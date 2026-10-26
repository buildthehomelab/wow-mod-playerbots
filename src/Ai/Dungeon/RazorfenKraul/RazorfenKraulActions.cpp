/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RazorfenKraulActions.h"

#include "Playerbots.h"
#include "RazorfenKraulShared.h"

Unit* RazorfenKraulFocusAction::ValidatedTarget()
{
    Unit* target = RazorfenKraul::GetFocus(botAI);
    if (target && !RazorfenKraul::CanFocus(botAI, target))
    {
        context->GetValue<ObjectGuid>("razorfen kraul focus")->Set(ObjectGuid::Empty);
        return nullptr;
    }
    return target;
}

bool RazorfenKraulFocusAction::isUseful()
{
    Unit* target = ValidatedTarget();
    return target && AI_VALUE(Unit*, "current target") != target;
}

bool RazorfenKraulFocusAction::Execute(Event /*event*/)
{
    Unit* target = ValidatedTarget();
    return target && Attack(target);
}
