/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SMStrategy.h"

#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "SMMultipliers.h"
#include "SMShared.h"

void VanillaDungeonScarletMonasteryStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    if (sPlayerbotAIConfig.EnableHerodWhirlwindAvoidance || sPlayerbotAIConfig.EnableScarletMonasteryMechanics)
        triggers.push_back(
            new TriggerNode("herod whirlwind", {NextAction("herod whirlwind escape", ACTION_EMERGENCY + 8)}));
    if (sPlayerbotAIConfig.EnableScarletMonasteryMechanics)
    {
        triggers.push_back(
            new TriggerNode("scarlet ground damage", {NextAction("scarlet ground damage", ACTION_EMERGENCY + 6)}));
        triggers.push_back(new TriggerNode("scarlet protect dominated ally",
                                           {NextAction("scarlet protect dominated ally", ACTION_EMERGENCY + 7)}));
        triggers.push_back(
            new TriggerNode("thalnos fire nova", {NextAction("thalnos fire nova escape", ACTION_EMERGENCY + 8)}));
        triggers.push_back(
            new TriggerNode("ironspine poison cloud", {NextAction("ironspine poison escape", ACTION_EMERGENCY + 8)}));
        triggers.push_back(new TriggerNode("scarlet dominated ally",
                                           {NextAction("scarlet dispel dominated ally", ACTION_DISPEL + 8)}));
        triggers.push_back(new TriggerNode("scarlet focus healer", {NextAction("scarlet focus healer", ACTION_RAID)}));
    }
    triggers.push_back(new TriggerNode("arcanist doan boss casting detonation",
                                       {NextAction("arcanist doan move out of detonation", ACTION_EMERGENCY + 9)}));
}

void VanillaDungeonScarletMonasteryStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    if (sPlayerbotAIConfig.EnableHerodWhirlwindAvoidance || sPlayerbotAIConfig.EnableScarletMonasteryMechanics)
        multipliers.push_back(new HerodWhirlwindMultiplier(botAI));
    if (sPlayerbotAIConfig.EnableScarletMonasteryMechanics)
    {
        multipliers.push_back(new ScarletHazardMultiplier(botAI, "thalnos fire nova", SMShared::THALNOS));
        multipliers.push_back(new ScarletHazardMultiplier(botAI, "ironspine poison cloud", SMShared::IRONSPINE));
        multipliers.push_back(new ScarletFocusMultiplier(botAI));
        multipliers.push_back(new ScarletGroundDamageMultiplier(botAI));
        multipliers.push_back(new ScarletCharmSafetyMultiplier(botAI));
    }
    multipliers.push_back(new ArcanistDoanDetonationMultiplier(botAI));
}

bool VanillaDungeonScarletMonasteryStrategy::HasTargetExclusions() const
{
    return sPlayerbotAIConfig.EnableScarletMonasteryMechanics;
}

void VanillaDungeonScarletMonasteryStrategy::AppendTargetExclusions(GuidSet& exclusions,
                                                                    TargetValueExclusionType /*type*/)
{
    if (!sPlayerbotAIConfig.EnableScarletMonasteryMechanics)
        return;
    AiObjectContext* context = botAI->GetAiObjectContext();
    for (ObjectGuid const guid : AI_VALUE(GuidVector, "scarlet charmed party"))
        if (SMShared::IsDominatedPartyMember(botAI->GetBot(), botAI->GetUnit(guid)))
            exclusions.insert(guid);
}
