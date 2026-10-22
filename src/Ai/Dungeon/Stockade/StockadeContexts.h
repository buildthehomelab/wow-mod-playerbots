/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_STOCKADECONTEXTS_H
#define PLAYERBOTS_STOCKADECONTEXTS_H

#include "NamedObjectContext.h"
#include "StockadeActions.h"
#include "StockadeShared.h"
#include "Trigger.h"
#include "Value.h"

class StockadeFocusValue : public ObjectGuidCalculatedValue
{
public:
    StockadeFocusValue(PlayerbotAI* botAI) : ObjectGuidCalculatedValue(botAI, "stockade focus", 500) {}

protected:
    ObjectGuid Calculate() override { return Stockade::FindFocus(botAI); }
};
class StockadeValueContext : public NamedObjectContext<UntypedValue>
{
public:
    StockadeValueContext() { creators["stockade focus"] = &focus; }

private:
    static UntypedValue* focus(PlayerbotAI* botAI) { return new StockadeFocusValue(botAI); }
};
class StockadeFocusTrigger : public Trigger
{
public:
    StockadeFocusTrigger(PlayerbotAI* botAI) : Trigger(botAI, "stockade focus") {}
    bool IsActive() override { return Stockade::GetFocus(botAI) != nullptr; }
};
class StockadeTriggerContext : public NamedObjectContext<Trigger>
{
public:
    StockadeTriggerContext() { creators["stockade focus"] = &focus; }

private:
    static Trigger* focus(PlayerbotAI* botAI) { return new StockadeFocusTrigger(botAI); }
};
class StockadeActionContext : public NamedObjectContext<Action>
{
public:
    StockadeActionContext() { creators["stockade focus"] = &focus; }

private:
    static Action* focus(PlayerbotAI* botAI) { return new StockadeFocusAction(botAI); }
};
#endif
