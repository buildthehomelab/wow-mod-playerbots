/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "WailingCavernsStrategy.h"

#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "WailingCavernsActions.h"
#include "WailingCavernsShared.h"

namespace
{
class WailingCavernsFocusMultiplier : public Multiplier
{
public:
    WailingCavernsFocusMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "wailing caverns focus") {}
    float GetValue(Action* action) override
    {
        if (!dynamic_cast<AttackAction*>(action) || dynamic_cast<WailingCavernsFocusAction*>(action) ||
            dynamic_cast<MeleeAction*>(action))
            return 1.0f;
        return WailingCaverns::GetFocus(botAI) ? 0.0f : 1.0f;
    }
};
}  // namespace

void WailingCavernsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    if (!sPlayerbotAIConfig.EnableWailingCavernsMechanics)
        return;
    triggers.push_back(
        new TriggerNode("wailing caverns focus", {NextAction("wailing caverns focus", ACTION_RAID + 1)}));
}

void WailingCavernsStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    if (!sPlayerbotAIConfig.EnableWailingCavernsMechanics)
        return;
    multipliers.push_back(new WailingCavernsFocusMultiplier(botAI));
}
