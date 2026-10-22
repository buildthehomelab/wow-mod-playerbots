/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "DeadminesStrategy.h"

#include "DeadminesActions.h"
#include "DeadminesShared.h"
#include "GenericSpellActions.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

namespace
{
class DeadminesHazardMultiplier : public Multiplier
{
public:
    DeadminesHazardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "deadmines hazard") {}
    float GetValue(Action* action) override
    {
        if (dynamic_cast<DeadminesEscapeAction*>(action))
            return 1.0f;
        if (dynamic_cast<CastSpellAction*>(action) && !botAI->CanMove())
            return 1.0f;
        if (!dynamic_cast<MovementAction*>(action) && !dynamic_cast<CastSpellAction*>(action))
            return 1.0f;
        return Deadmines::InHazard(botAI) ? 0.0f : 1.0f;
    }
};
class DeadminesFocusMultiplier : public Multiplier
{
public:
    DeadminesFocusMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "deadmines focus") {}
    float GetValue(Action* action) override
    {
        if (!dynamic_cast<AttackAction*>(action) || dynamic_cast<DeadminesFocusAction*>(action) ||
            dynamic_cast<MeleeAction*>(action))
            return 1.0f;
        return Deadmines::GetFocus(botAI) ? 0.0f : 1.0f;
    }
};
}  // namespace

void DeadminesStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    if (!sPlayerbotAIConfig.EnableDeadminesMechanics)
        return;
    triggers.push_back(new TriggerNode("deadmines hazard", {NextAction("deadmines escape", ACTION_EMERGENCY + 8)}));
    triggers.push_back(new TriggerNode("deadmines focus", {NextAction("deadmines focus", ACTION_RAID + 1)}));
}

void DeadminesStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    if (!sPlayerbotAIConfig.EnableDeadminesMechanics)
        return;
    multipliers.push_back(new DeadminesHazardMultiplier(botAI));
    multipliers.push_back(new DeadminesFocusMultiplier(botAI));
}
