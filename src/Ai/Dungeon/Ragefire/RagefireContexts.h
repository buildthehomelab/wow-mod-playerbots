/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAGEFIRECONTEXTS_H
#define PLAYERBOTS_RAGEFIRECONTEXTS_H

#include "NamedObjectContext.h"
#include "RagefireActions.h"
#include "RagefireShared.h"
#include "Trigger.h"
#include "Value.h"

class RagefireHazardValue : public CalculatedValue<Ragefire::Hazard>
{
public:
    RagefireHazardValue(PlayerbotAI* botAI)
        : CalculatedValue<Ragefire::Hazard>(botAI, "ragefire hazard", Ragefire::CACHE_MS)
    {
    }

protected:
    Ragefire::Hazard Calculate() override { return Ragefire::CollectHazard(botAI); }
};
class RagefireFocusValue : public ObjectGuidCalculatedValue
{
public:
    RagefireFocusValue(PlayerbotAI* botAI) : ObjectGuidCalculatedValue(botAI, "ragefire focus", 500) {}

protected:
    ObjectGuid Calculate() override { return Ragefire::FindFocus(botAI); }
};
class RagefireValueContext : public NamedObjectContext<UntypedValue>
{
public:
    RagefireValueContext()
    {
        creators["ragefire hazard"] = &hazard;
        creators["ragefire focus"] = &focus;
    }

private:
    static UntypedValue* hazard(PlayerbotAI* botAI) { return new RagefireHazardValue(botAI); }
    static UntypedValue* focus(PlayerbotAI* botAI) { return new RagefireFocusValue(botAI); }
};
class RagefireHazardTrigger : public Trigger
{
public:
    RagefireHazardTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ragefire hazard") {}
    bool IsActive() override { return Ragefire::InHazard(botAI); }
};
class RagefireFocusTrigger : public Trigger
{
public:
    RagefireFocusTrigger(PlayerbotAI* botAI) : Trigger(botAI, "ragefire focus") {}
    bool IsActive() override { return Ragefire::GetFocus(botAI) != nullptr; }
};
class RagefireTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RagefireTriggerContext()
    {
        creators["ragefire hazard"] = &hazard;
        creators["ragefire focus"] = &focus;
    }

private:
    static Trigger* hazard(PlayerbotAI* botAI) { return new RagefireHazardTrigger(botAI); }
    static Trigger* focus(PlayerbotAI* botAI) { return new RagefireFocusTrigger(botAI); }
};
class RagefireActionContext : public NamedObjectContext<Action>
{
public:
    RagefireActionContext()
    {
        creators["ragefire escape"] = &escape;
        creators["ragefire focus"] = &focus;
    }

private:
    static Action* escape(PlayerbotAI* botAI) { return new RagefireEscapeAction(botAI); }
    static Action* focus(PlayerbotAI* botAI) { return new RagefireFocusAction(botAI); }
};
#endif
