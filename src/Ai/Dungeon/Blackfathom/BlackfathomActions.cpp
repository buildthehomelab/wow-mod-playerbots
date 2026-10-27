/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BlackfathomActions.h"

#include "BlackfathomShared.h"
#include "Playerbots.h"

Unit* BlackfathomFocusAction::ValidatedTarget()
{
    Unit* target = Blackfathom::GetFocus(botAI);
    if (target && !Blackfathom::CanFocus(botAI, target))
    {
        context->GetValue<ObjectGuid>("blackfathom focus")->Set(ObjectGuid::Empty);
        return nullptr;
    }
    return target;
}

bool BlackfathomFocusAction::isUseful()
{
    Unit* target = ValidatedTarget();
    return target && AI_VALUE(Unit*, "current target") != target;
}

bool BlackfathomFocusAction::Execute(Event /*event*/)
{
    Unit* target = ValidatedTarget();
    return target && Attack(target);
}
