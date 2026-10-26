/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAZORFENKRAULCONTEXTS_H
#define PLAYERBOTS_RAZORFENKRAULCONTEXTS_H

#include "NamedObjectContext.h"
#include "RazorfenKraulActions.h"
#include "RazorfenKraulShared.h"
#include "Trigger.h"
#include "Value.h"

class RazorfenKraulFocusValue : public ObjectGuidCalculatedValue
{
public:
    RazorfenKraulFocusValue(PlayerbotAI* botAI) : ObjectGuidCalculatedValue(botAI, "razorfen kraul focus", 500) {}

protected:
    ObjectGuid Calculate() override { return RazorfenKraul::FindFocus(botAI); }
};
class RazorfenKraulExplicitTargetValue : public BoolCalculatedValue
{
public:
    RazorfenKraulExplicitTargetValue(PlayerbotAI* botAI)
        : BoolCalculatedValue(botAI, "razorfen kraul explicit target", 500)
    {
    }

protected:
    bool Calculate() override { return RazorfenKraul::HasExplicitTarget(botAI); }
};
class RazorfenKraulValueContext : public NamedObjectContext<UntypedValue>
{
public:
    RazorfenKraulValueContext()
    {
        creators["razorfen kraul focus"] = &focus;
        creators["razorfen kraul explicit target"] = &explicitTarget;
    }

private:
    static UntypedValue* focus(PlayerbotAI* botAI) { return new RazorfenKraulFocusValue(botAI); }
    static UntypedValue* explicitTarget(PlayerbotAI* botAI) { return new RazorfenKraulExplicitTargetValue(botAI); }
};
class RazorfenKraulFocusTrigger : public Trigger
{
public:
    RazorfenKraulFocusTrigger(PlayerbotAI* botAI) : Trigger(botAI, "razorfen kraul focus") {}
    bool IsActive() override { return RazorfenKraul::GetFocus(botAI) != nullptr; }
};
class RazorfenKraulTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RazorfenKraulTriggerContext() { creators["razorfen kraul focus"] = &focus; }

private:
    static Trigger* focus(PlayerbotAI* botAI) { return new RazorfenKraulFocusTrigger(botAI); }
};
class RazorfenKraulActionContext : public NamedObjectContext<Action>
{
public:
    RazorfenKraulActionContext() { creators["razorfen kraul focus"] = &focus; }

private:
    static Action* focus(PlayerbotAI* botAI) { return new RazorfenKraulFocusAction(botAI); }
};
#endif
