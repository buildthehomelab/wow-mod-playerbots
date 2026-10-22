/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_WAILINGCAVERNSCONTEXTS_H
#define PLAYERBOTS_WAILINGCAVERNSCONTEXTS_H

#include "NamedObjectContext.h"
#include "Trigger.h"
#include "Value.h"
#include "WailingCavernsActions.h"
#include "WailingCavernsShared.h"

class WailingCavernsFocusValue : public ObjectGuidCalculatedValue
{
public:
    WailingCavernsFocusValue(PlayerbotAI* botAI) : ObjectGuidCalculatedValue(botAI, "wailing caverns focus", 500) {}

protected:
    ObjectGuid Calculate() override { return WailingCaverns::FindFocus(botAI); }
};
class WailingCavernsValueContext : public NamedObjectContext<UntypedValue>
{
public:
    WailingCavernsValueContext() { creators["wailing caverns focus"] = &focus; }

private:
    static UntypedValue* focus(PlayerbotAI* botAI) { return new WailingCavernsFocusValue(botAI); }
};
class WailingCavernsFocusTrigger : public Trigger
{
public:
    WailingCavernsFocusTrigger(PlayerbotAI* botAI) : Trigger(botAI, "wailing caverns focus") {}
    bool IsActive() override { return WailingCaverns::GetFocus(botAI) != nullptr; }
};
class WailingCavernsTriggerContext : public NamedObjectContext<Trigger>
{
public:
    WailingCavernsTriggerContext() { creators["wailing caverns focus"] = &focus; }

private:
    static Trigger* focus(PlayerbotAI* botAI) { return new WailingCavernsFocusTrigger(botAI); }
};
class WailingCavernsActionContext : public NamedObjectContext<Action>
{
public:
    WailingCavernsActionContext() { creators["wailing caverns focus"] = &focus; }

private:
    static Action* focus(PlayerbotAI* botAI) { return new WailingCavernsFocusAction(botAI); }
};
#endif
