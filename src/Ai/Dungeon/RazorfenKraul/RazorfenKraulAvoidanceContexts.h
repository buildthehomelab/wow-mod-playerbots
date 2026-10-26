/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAZORFENKRAULAVOIDANCECONTEXTS_H
#define PLAYERBOTS_RAZORFENKRAULAVOIDANCECONTEXTS_H

#include "NamedObjectContext.h"
#include "RazorfenKraulAvoidance.h"
#include "RazorfenKraulEscapeAction.h"
#include "Trigger.h"
#include "Value.h"

class RazorfenKraulAvoidanceHazardValue : public CalculatedValue<RazorfenKraul::Hazard>
{
public:
    RazorfenKraulAvoidanceHazardValue(PlayerbotAI* botAI)
        : CalculatedValue<RazorfenKraul::Hazard>(botAI, "razorfen kraul hazard", RazorfenKraul::CACHE_MS)
    {
    }

protected:
    RazorfenKraul::Hazard Calculate() override { return RazorfenKraul::CollectHazard(botAI); }
};
class RazorfenKraulAvoidanceValueContext : public NamedObjectContext<UntypedValue>
{
public:
    RazorfenKraulAvoidanceValueContext() { creators["razorfen kraul hazard"] = &hazard; }

private:
    static UntypedValue* hazard(PlayerbotAI* botAI) { return new RazorfenKraulAvoidanceHazardValue(botAI); }
};
class RazorfenKraulAvoidanceHazardTrigger : public Trigger
{
public:
    RazorfenKraulAvoidanceHazardTrigger(PlayerbotAI* botAI) : Trigger(botAI, "razorfen kraul hazard") {}
    bool IsActive() override { return RazorfenKraul::InHazard(botAI); }
};
class RazorfenKraulAvoidanceTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RazorfenKraulAvoidanceTriggerContext() { creators["razorfen kraul hazard"] = &hazard; }

private:
    static Trigger* hazard(PlayerbotAI* botAI) { return new RazorfenKraulAvoidanceHazardTrigger(botAI); }
};
class RazorfenKraulAvoidanceActionContext : public NamedObjectContext<Action>
{
public:
    RazorfenKraulAvoidanceActionContext() { creators["razorfen kraul escape"] = &escape; }

private:
    static Action* escape(PlayerbotAI* botAI) { return new RazorfenKraulEscapeAction(botAI); }
};
#endif
