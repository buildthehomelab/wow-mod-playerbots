/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BLACKFATHOMCHARM_H
#define PLAYERBOTS_BLACKFATHOMCHARM_H

#include "Charmed.h"
#include "NamedObjectContext.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Value.h"

namespace Blackfathom
{
inline bool CharmEnabled(PlayerbotAI* botAI)
{
    return sPlayerbotAIConfig.EnableBlackfathomMechanics && botAI->GetBot()->GetMapId() == 48 &&
           botAI->GetBot()->IsAlive();
}
void InitCharmTriggers(std::vector<TriggerNode*>& triggers);
void InitCharmMultipliers(PlayerbotAI* botAI, std::vector<Multiplier*>& multipliers);
}  // namespace Blackfathom

template <class SharedTrigger>
class BlackfathomCharmValue : public BoolCalculatedValue
{
public:
    BlackfathomCharmValue(PlayerbotAI* botAI, std::string const& name) : BoolCalculatedValue(botAI, name, 250) {}

protected:
    bool Calculate() override { return Blackfathom::CharmEnabled(botAI) && SharedTrigger(botAI).IsActive(); }
};
class BlackfathomCharmTrigger : public Trigger
{
public:
    BlackfathomCharmTrigger(PlayerbotAI* botAI, std::string const& name) : Trigger(botAI, name) {}
    bool IsActive() override { return Blackfathom::CharmEnabled(botAI) && context->GetValue<bool>(getName())->Get(); }
};
template <class SharedAction>
class BlackfathomCharmAction : public SharedAction
{
public:
    BlackfathomCharmAction(PlayerbotAI* botAI, std::string const& name) : SharedAction(botAI), _name(name) {}
    std::string const getName() override { return _name; }
    bool Execute(Event event) override
    {
        return Blackfathom::CharmEnabled(this->botAI) && SharedAction::Execute(event);
    }

private:
    std::string _name;
};
template <class SharedMultiplier>
class BlackfathomCharmMultiplier : public SharedMultiplier
{
public:
    BlackfathomCharmMultiplier(PlayerbotAI* botAI) : SharedMultiplier(botAI) {}
    float GetValue(Action* action) override
    {
        return Blackfathom::CharmEnabled(this->botAI) ? SharedMultiplier::GetValue(action) : 1.0f;
    }
};
class BlackfathomCharmValueContext : public NamedObjectContext<UntypedValue>
{
public:
    BlackfathomCharmValueContext()
    {
        creators["blackfathom charm control"] = &control;
        creators["blackfathom charm channel"] = &channel;
        creators["blackfathom charm target"] = &target;
    }

private:
    static UntypedValue* control(PlayerbotAI* botAI)
    {
        return new BlackfathomCharmValue<RcCharmedMemberTrigger>(botAI, "blackfathom charm control");
    }
    static UntypedValue* channel(PlayerbotAI* botAI)
    {
        return new BlackfathomCharmValue<RcCharmedInChannelTrigger>(botAI, "blackfathom charm channel");
    }
    static UntypedValue* target(PlayerbotAI* botAI)
    {
        return new BlackfathomCharmValue<RcCharmedTargetTrigger>(botAI, "blackfathom charm target");
    }
};
class BlackfathomCharmTriggerContext : public NamedObjectContext<Trigger>
{
public:
    BlackfathomCharmTriggerContext()
    {
        creators["blackfathom charm control"] = &control;
        creators["blackfathom charm channel"] = &channel;
        creators["blackfathom charm target"] = &target;
    }

private:
    static Trigger* control(PlayerbotAI* botAI)
    {
        return new BlackfathomCharmTrigger(botAI, "blackfathom charm control");
    }
    static Trigger* channel(PlayerbotAI* botAI)
    {
        return new BlackfathomCharmTrigger(botAI, "blackfathom charm channel");
    }
    static Trigger* target(PlayerbotAI* botAI)
    {
        return new BlackfathomCharmTrigger(botAI, "blackfathom charm target");
    }
};
class BlackfathomCharmActionContext : public NamedObjectContext<Action>
{
public:
    BlackfathomCharmActionContext()
    {
        creators["blackfathom charm control"] = &control;
        creators["blackfathom charm channel"] = &channel;
        creators["blackfathom charm target"] = &target;
    }

private:
    static Action* control(PlayerbotAI* botAI)
    {
        return new BlackfathomCharmAction<RcControlCharmedAction>(botAI, "blackfathom charm control");
    }
    static Action* channel(PlayerbotAI* botAI)
    {
        return new BlackfathomCharmAction<RcStopChannelOnCharmedAction>(botAI, "blackfathom charm channel");
    }
    static Action* target(PlayerbotAI* botAI)
    {
        return new BlackfathomCharmAction<RcLeaveCharmedTargetAction>(botAI, "blackfathom charm target");
    }
};
#endif
