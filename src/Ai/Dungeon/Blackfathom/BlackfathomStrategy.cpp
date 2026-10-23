/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BlackfathomStrategy.h"

#include "BlackfathomActions.h"
#include "BlackfathomAvoidance.h"
#include "BlackfathomCharm.h"
#include "BlackfathomEscapeAction.h"
#include "BlackfathomShared.h"
#include "Playerbots.h"

namespace
{
class BlackfathomHazardMultiplier : public Multiplier
{
public:
    BlackfathomHazardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "blackfathom hazard") {}
    float GetValue(Action* action) override
    {
        if (!dynamic_cast<MovementAction*>(action) || dynamic_cast<BlackfathomEscapeAction*>(action) ||
            dynamic_cast<RcLeaveCharmedTargetAction*>(action))
            return 1.0f;
        return Blackfathom::InHazard(botAI) ? 0.0f : 1.0f;
    }
};
class BlackfathomFocusMultiplier : public Multiplier
{
public:
    BlackfathomFocusMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "blackfathom focus") {}
    float GetValue(Action* action) override
    {
        if (!dynamic_cast<AttackAction*>(action) || dynamic_cast<BlackfathomFocusAction*>(action) ||
            dynamic_cast<MeleeAction*>(action) || dynamic_cast<AttackMyTargetAction*>(action) ||
            dynamic_cast<AttackDuelOpponentAction*>(action) || dynamic_cast<RcLeaveCharmedTargetAction*>(action))
            return 1.0f;
        return Blackfathom::GetFocus(botAI) ? 0.0f : 1.0f;
    }
};
}  // namespace

void BlackfathomStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    Blackfathom::InitCharmTriggers(triggers);
    triggers.push_back(new TriggerNode("blackfathom hazard", {NextAction("blackfathom escape", ACTION_INTERRUPT - 1)}));
    triggers.push_back(new TriggerNode("blackfathom focus", {NextAction("blackfathom focus", ACTION_RAID + 1)}));
}

void BlackfathomStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    Blackfathom::InitCharmMultipliers(botAI, multipliers);
    multipliers.push_back(new BlackfathomHazardMultiplier(botAI));
    multipliers.push_back(new BlackfathomFocusMultiplier(botAI));
}
