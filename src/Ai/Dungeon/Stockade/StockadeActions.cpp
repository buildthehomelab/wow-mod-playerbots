/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "StockadeActions.h"

#include "Playerbots.h"
#include "StockadeShared.h"

Unit* StockadeFocusAction::ValidatedTarget()
{
    Unit* target = Stockade::GetFocus(botAI);
    if (target && !Stockade::CanFocus(botAI, target))
    {
        context->GetValue<ObjectGuid>("stockade focus")->Set(ObjectGuid::Empty);
        return nullptr;
    }
    return target;
}

bool StockadeFocusAction::isUseful()
{
    Unit* target = ValidatedTarget();
    return target && AI_VALUE(Unit*, "current target") != target;
}

bool StockadeFocusAction::Execute(Event /*event*/)
{
    Unit* target = ValidatedTarget();
    return target && Attack(target);
}
