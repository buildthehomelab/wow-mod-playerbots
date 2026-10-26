/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RazorfenKraulCharm.h"

namespace RazorfenKraul
{
void InitCharmTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("razorfen kraul charm control",
                                       {NextAction("razorfen kraul charm control", ACTION_EMERGENCY + 1)}));
    triggers.push_back(new TriggerNode("razorfen kraul charm channel",
                                       {NextAction("razorfen kraul charm channel", ACTION_EMERGENCY + 1)}));
    triggers.push_back(new TriggerNode("razorfen kraul charm target",
                                       {NextAction("razorfen kraul charm target", ACTION_EMERGENCY + 1)}));
}

void InitCharmMultipliers(PlayerbotAI* botAI, std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new RazorfenKraulCharmMultiplier<RcCharmedSelfMultiplier>(botAI));
    multipliers.push_back(new RazorfenKraulCharmMultiplier<RcCharmedSplashMultiplier>(botAI));
    multipliers.push_back(new RazorfenKraulCharmMultiplier<RcCharmedDispelMultiplier>(botAI));
}
}  // namespace RazorfenKraul
