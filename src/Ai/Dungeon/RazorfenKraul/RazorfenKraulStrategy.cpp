/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RazorfenKraulStrategy.h"

#include "Playerbots.h"
#include "RazorfenKraulActions.h"
#include "RazorfenKraulAvoidance.h"
#include "RazorfenKraulCharm.h"
#include "RazorfenKraulEscapeAction.h"
#include "RazorfenKraulShared.h"

namespace
{
class RazorfenKraulHazardMultiplier : public Multiplier
{
public:
    RazorfenKraulHazardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "razorfen kraul hazard") {}
    float GetValue(Action* action) override
    {
        if (!dynamic_cast<MovementAction*>(action) || dynamic_cast<RazorfenKraulEscapeAction*>(action) ||
            dynamic_cast<RcLeaveCharmedTargetAction*>(action))
            return 1.0f;
        return RazorfenKraul::InHazard(botAI) ? 0.0f : 1.0f;
    }
};
class RazorfenKraulFocusMultiplier : public Multiplier
{
public:
    RazorfenKraulFocusMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "razorfen kraul focus") {}
    float GetValue(Action* action) override
    {
        if (!dynamic_cast<AttackAction*>(action) || dynamic_cast<RazorfenKraulFocusAction*>(action) ||
            dynamic_cast<MeleeAction*>(action) || dynamic_cast<AttackMyTargetAction*>(action) ||
            dynamic_cast<AttackDuelOpponentAction*>(action) || dynamic_cast<RcLeaveCharmedTargetAction*>(action))
            return 1.0f;
        return RazorfenKraul::GetFocus(botAI) ? 0.0f : 1.0f;
    }
};
}  // namespace

void RazorfenKraulStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    RazorfenKraul::InitCharmTriggers(triggers);
    triggers.push_back(
        new TriggerNode("razorfen kraul hazard", {NextAction("razorfen kraul escape", ACTION_INTERRUPT - 1)}));
    triggers.push_back(new TriggerNode("razorfen kraul focus", {NextAction("razorfen kraul focus", ACTION_RAID + 1)}));
}

void RazorfenKraulStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    RazorfenKraul::InitCharmMultipliers(botAI, multipliers);
    multipliers.push_back(new RazorfenKraulHazardMultiplier(botAI));
    multipliers.push_back(new RazorfenKraulFocusMultiplier(botAI));
}
