/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BLACKFATHOMCONTEXTS_H
#define PLAYERBOTS_BLACKFATHOMCONTEXTS_H

#include "BlackfathomActions.h"
#include "BlackfathomShared.h"
#include "NamedObjectContext.h"
#include "Trigger.h"
#include "Value.h"

class BlackfathomFocusValue : public ObjectGuidCalculatedValue
{
public:
    BlackfathomFocusValue(PlayerbotAI* botAI) : ObjectGuidCalculatedValue(botAI, "blackfathom focus", 500) {}

protected:
    ObjectGuid Calculate() override { return Blackfathom::FindFocus(botAI); }
};
class BlackfathomExplicitTargetValue : public BoolCalculatedValue
{
public:
    BlackfathomExplicitTargetValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "blackfathom explicit target", 500)
    {
    }

protected:
    bool Calculate() override { return Blackfathom::HasExplicitTarget(botAI); }
};
class BlackfathomValueContext : public NamedObjectContext<UntypedValue>
{
public:
    BlackfathomValueContext()
    {
        creators["blackfathom focus"] = &focus;
        creators["blackfathom explicit target"] = &explicitTarget;
    }

private:
    static UntypedValue* focus(PlayerbotAI* botAI) { return new BlackfathomFocusValue(botAI); }
    static UntypedValue* explicitTarget(PlayerbotAI* botAI) { return new BlackfathomExplicitTargetValue(botAI); }
};
class BlackfathomFocusTrigger : public Trigger
{
public:
    BlackfathomFocusTrigger(PlayerbotAI* botAI) : Trigger(botAI, "blackfathom focus") {}
    bool IsActive() override { return Blackfathom::GetFocus(botAI) != nullptr; }
};
class BlackfathomTriggerContext : public NamedObjectContext<Trigger>
{
public:
    BlackfathomTriggerContext() { creators["blackfathom focus"] = &focus; }

private:
    static Trigger* focus(PlayerbotAI* botAI) { return new BlackfathomFocusTrigger(botAI); }
};
class BlackfathomActionContext : public NamedObjectContext<Action>
{
public:
    BlackfathomActionContext() { creators["blackfathom focus"] = &focus; }

private:
    static Action* focus(PlayerbotAI* botAI) { return new BlackfathomFocusAction(botAI); }
};
#endif
