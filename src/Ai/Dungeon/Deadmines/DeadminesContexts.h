/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DEADMINESCONTEXTS_H
#define PLAYERBOTS_DEADMINESCONTEXTS_H

#include "DeadminesActions.h"
#include "DeadminesShared.h"
#include "NamedObjectContext.h"
#include "Trigger.h"
#include "Value.h"

class DeadminesHazardValue : public CalculatedValue<Deadmines::Hazard>
{
public:
    DeadminesHazardValue(PlayerbotAI* botAI)
        : CalculatedValue<Deadmines::Hazard>(botAI, "deadmines hazard", Deadmines::CACHE_MS)
    {
    }

protected:
    Deadmines::Hazard Calculate() override { return Deadmines::CollectHazard(botAI); }
};
class DeadminesFocusValue : public ObjectGuidCalculatedValue
{
public:
    DeadminesFocusValue(PlayerbotAI* botAI) : ObjectGuidCalculatedValue(botAI, "deadmines focus", 500) {}

protected:
    ObjectGuid Calculate() override { return Deadmines::FindFocus(botAI); }
};
class DeadminesValueContext : public NamedObjectContext<UntypedValue>
{
public:
    DeadminesValueContext()
    {
        creators["deadmines hazard"] = &hazard;
        creators["deadmines focus"] = &focus;
    }

private:
    static UntypedValue* hazard(PlayerbotAI* botAI) { return new DeadminesHazardValue(botAI); }
    static UntypedValue* focus(PlayerbotAI* botAI) { return new DeadminesFocusValue(botAI); }
};
class DeadminesHazardTrigger : public Trigger
{
public:
    DeadminesHazardTrigger(PlayerbotAI* botAI) : Trigger(botAI, "deadmines hazard") {}
    bool IsActive() override { return Deadmines::InHazard(botAI); }
};
class DeadminesFocusTrigger : public Trigger
{
public:
    DeadminesFocusTrigger(PlayerbotAI* botAI) : Trigger(botAI, "deadmines focus") {}
    bool IsActive() override { return Deadmines::GetFocus(botAI) != nullptr; }
};
class DeadminesTriggerContext : public NamedObjectContext<Trigger>
{
public:
    DeadminesTriggerContext()
    {
        creators["deadmines hazard"] = &hazard;
        creators["deadmines focus"] = &focus;
    }

private:
    static Trigger* hazard(PlayerbotAI* botAI) { return new DeadminesHazardTrigger(botAI); }
    static Trigger* focus(PlayerbotAI* botAI) { return new DeadminesFocusTrigger(botAI); }
};
class DeadminesActionContext : public NamedObjectContext<Action>
{
public:
    DeadminesActionContext()
    {
        creators["deadmines escape"] = &escape;
        creators["deadmines focus"] = &focus;
    }

private:
    static Action* escape(PlayerbotAI* botAI) { return new DeadminesEscapeAction(botAI); }
    static Action* focus(PlayerbotAI* botAI) { return new DeadminesFocusAction(botAI); }
};
#endif
