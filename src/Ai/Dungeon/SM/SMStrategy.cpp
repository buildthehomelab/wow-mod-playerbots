/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SMStrategy.h"

#include "PlayerbotAIConfig.h"
#include "SMMultipliers.h"

void VanillaDungeonScarletMonasteryStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    if (sPlayerbotAIConfig.EnableHerodWhirlwindAvoidance)
        triggers.push_back(
            new TriggerNode("herod whirlwind", {NextAction("herod whirlwind escape", ACTION_EMERGENCY + 8)}));

    // Arcanist Doan
    triggers.push_back(new TriggerNode("arcanist doan boss casting detonation",
                                       {NextAction("arcanist doan move out of detonation", ACTION_EMERGENCY + 10)}));
}

void VanillaDungeonScarletMonasteryStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    if (sPlayerbotAIConfig.EnableHerodWhirlwindAvoidance)
        multipliers.push_back(new HerodWhirlwindMultiplier(botAI));

    multipliers.push_back(new ArcanistDoanDetonationMultiplier(botAI));
}
