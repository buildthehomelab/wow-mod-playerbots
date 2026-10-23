/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BlackfathomCharm.h"

namespace Blackfathom
{
void InitCharmTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("blackfathom charm control", {NextAction("blackfathom charm control", ACTION_EMERGENCY + 1)}));
    triggers.push_back(
        new TriggerNode("blackfathom charm channel", {NextAction("blackfathom charm channel", ACTION_EMERGENCY + 1)}));
    triggers.push_back(
        new TriggerNode("blackfathom charm target", {NextAction("blackfathom charm target", ACTION_EMERGENCY + 1)}));
}

void InitCharmMultipliers(PlayerbotAI* botAI, std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new BlackfathomCharmMultiplier<RcCharmedSelfMultiplier>(botAI));
    multipliers.push_back(new BlackfathomCharmMultiplier<RcCharmedSplashMultiplier>(botAI));
    multipliers.push_back(new BlackfathomCharmMultiplier<RcCharmedDispelMultiplier>(botAI));
}
}  // namespace Blackfathom
