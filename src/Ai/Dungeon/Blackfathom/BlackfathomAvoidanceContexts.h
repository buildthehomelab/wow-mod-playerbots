/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BLACKFATHOMAVOIDANCECONTEXTS_H
#define PLAYERBOTS_BLACKFATHOMAVOIDANCECONTEXTS_H

#include "BlackfathomAvoidance.h"
#include "BlackfathomEscapeAction.h"
#include "NamedObjectContext.h"
#include "Trigger.h"
#include "Value.h"

class BlackfathomAvoidanceHazardValue : public CalculatedValue<Blackfathom::Hazard>
{
public:
    BlackfathomAvoidanceHazardValue(PlayerbotAI* botAI)
        : CalculatedValue<Blackfathom::Hazard>(botAI, "blackfathom hazard", Blackfathom::CACHE_MS)
    {
    }

protected:
    Blackfathom::Hazard Calculate() override { return Blackfathom::CollectHazard(botAI); }
};
class BlackfathomAvoidanceValueContext : public NamedObjectContext<UntypedValue>
{
public:
    BlackfathomAvoidanceValueContext() { creators["blackfathom hazard"] = &hazard; }

private:
    static UntypedValue* hazard(PlayerbotAI* botAI) { return new BlackfathomAvoidanceHazardValue(botAI); }
};
class BlackfathomAvoidanceHazardTrigger : public Trigger
{
public:
    BlackfathomAvoidanceHazardTrigger(PlayerbotAI* botAI) : Trigger(botAI, "blackfathom hazard") {}
    bool IsActive() override { return Blackfathom::InHazard(botAI); }
};
class BlackfathomAvoidanceTriggerContext : public NamedObjectContext<Trigger>
{
public:
    BlackfathomAvoidanceTriggerContext() { creators["blackfathom hazard"] = &hazard; }

private:
    static Trigger* hazard(PlayerbotAI* botAI) { return new BlackfathomAvoidanceHazardTrigger(botAI); }
};
class BlackfathomAvoidanceActionContext : public NamedObjectContext<Action>
{
public:
    BlackfathomAvoidanceActionContext() { creators["blackfathom escape"] = &escape; }

private:
    static Action* escape(PlayerbotAI* botAI) { return new BlackfathomEscapeAction(botAI); }
};
#endif
