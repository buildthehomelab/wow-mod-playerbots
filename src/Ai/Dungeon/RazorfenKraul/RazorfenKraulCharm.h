/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAZORFENKRAULCHARM_H
#define PLAYERBOTS_RAZORFENKRAULCHARM_H

#include "Charmed.h"
#include "NamedObjectContext.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Value.h"

namespace RazorfenKraul
{
inline bool CharmEnabled(PlayerbotAI* botAI)
{
    return sPlayerbotAIConfig.EnableRazorfenKraulMechanics && botAI->GetBot()->GetMapId() == 47 &&
           botAI->GetBot()->IsAlive();
}
void InitCharmTriggers(std::vector<TriggerNode*>& triggers);
void InitCharmMultipliers(PlayerbotAI* botAI, std::vector<Multiplier*>& multipliers);
}  // namespace RazorfenKraul

template <class SharedTrigger>
class RazorfenKraulCharmValue : public BoolCalculatedValue
{
public:
    RazorfenKraulCharmValue(PlayerbotAI* botAI, std::string const& name) : BoolCalculatedValue(botAI, name, 250) {}

protected:
    bool Calculate() override { return RazorfenKraul::CharmEnabled(botAI) && SharedTrigger(botAI).IsActive(); }
};
class RazorfenKraulCharmTrigger : public Trigger
{
public:
    RazorfenKraulCharmTrigger(PlayerbotAI* botAI, std::string const& name) : Trigger(botAI, name) {}
    bool IsActive() override { return RazorfenKraul::CharmEnabled(botAI) && context->GetValue<bool>(getName())->Get(); }
};
template <class SharedAction>
class RazorfenKraulCharmAction : public SharedAction
{
public:
    RazorfenKraulCharmAction(PlayerbotAI* botAI, std::string const& name) : SharedAction(botAI), _name(name) {}
    std::string const getName() override { return _name; }
    bool Execute(Event event) override
    {
        return RazorfenKraul::CharmEnabled(this->botAI) && SharedAction::Execute(event);
    }

private:
    std::string _name;
};
template <class SharedMultiplier>
class RazorfenKraulCharmMultiplier : public SharedMultiplier
{
public:
    RazorfenKraulCharmMultiplier(PlayerbotAI* botAI) : SharedMultiplier(botAI) {}
    float GetValue(Action* action) override
    {
        return RazorfenKraul::CharmEnabled(this->botAI) ? SharedMultiplier::GetValue(action) : 1.0f;
    }
};
class RazorfenKraulCharmValueContext : public NamedObjectContext<UntypedValue>
{
public:
    RazorfenKraulCharmValueContext()
    {
        creators["razorfen kraul charm control"] = &control;
        creators["razorfen kraul charm channel"] = &channel;
        creators["razorfen kraul charm target"] = &target;
    }

private:
    static UntypedValue* control(PlayerbotAI* botAI)
    {
        return new RazorfenKraulCharmValue<RcCharmedMemberTrigger>(botAI, "razorfen kraul charm control");
    }
    static UntypedValue* channel(PlayerbotAI* botAI)
    {
        return new RazorfenKraulCharmValue<RcCharmedInChannelTrigger>(botAI, "razorfen kraul charm channel");
    }
    static UntypedValue* target(PlayerbotAI* botAI)
    {
        return new RazorfenKraulCharmValue<RcCharmedTargetTrigger>(botAI, "razorfen kraul charm target");
    }
};
class RazorfenKraulCharmTriggerContext : public NamedObjectContext<Trigger>
{
public:
    RazorfenKraulCharmTriggerContext()
    {
        creators["razorfen kraul charm control"] = &control;
        creators["razorfen kraul charm channel"] = &channel;
        creators["razorfen kraul charm target"] = &target;
    }

private:
    static Trigger* control(PlayerbotAI* botAI)
    {
        return new RazorfenKraulCharmTrigger(botAI, "razorfen kraul charm control");
    }
    static Trigger* channel(PlayerbotAI* botAI)
    {
        return new RazorfenKraulCharmTrigger(botAI, "razorfen kraul charm channel");
    }
    static Trigger* target(PlayerbotAI* botAI)
    {
        return new RazorfenKraulCharmTrigger(botAI, "razorfen kraul charm target");
    }
};
class RazorfenKraulCharmActionContext : public NamedObjectContext<Action>
{
public:
    RazorfenKraulCharmActionContext()
    {
        creators["razorfen kraul charm control"] = &control;
        creators["razorfen kraul charm channel"] = &channel;
        creators["razorfen kraul charm target"] = &target;
    }

private:
    static Action* control(PlayerbotAI* botAI)
    {
        return new RazorfenKraulCharmAction<RcControlCharmedAction>(botAI, "razorfen kraul charm control");
    }
    static Action* channel(PlayerbotAI* botAI)
    {
        return new RazorfenKraulCharmAction<RcStopChannelOnCharmedAction>(botAI, "razorfen kraul charm channel");
    }
    static Action* target(PlayerbotAI* botAI)
    {
        return new RazorfenKraulCharmAction<RcLeaveCharmedTargetAction>(botAI, "razorfen kraul charm target");
    }
};
#endif
