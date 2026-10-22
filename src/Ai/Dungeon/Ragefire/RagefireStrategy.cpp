/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RagefireStrategy.h"

#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "RagefireActions.h"
#include "RagefireShared.h"

namespace
{
class RagefireHazardMultiplier : public Multiplier
{
public:
    RagefireHazardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "ragefire hazard") {}
    float GetValue(Action* action) override
    {
        if (dynamic_cast<RagefireEscapeAction*>(action))
            return 1.0f;
        if (!dynamic_cast<MovementAction*>(action))
            return 1.0f;
        return Ragefire::InHazard(botAI) ? 0.0f : 1.0f;
    }
};
class RagefireFocusMultiplier : public Multiplier
{
public:
    RagefireFocusMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "ragefire focus") {}
    float GetValue(Action* action) override
    {
        if (dynamic_cast<RagefireFocusAction*>(action))
            return Ragefire::InHazard(botAI) ? 0.0f : 1.0f;
        if (!dynamic_cast<AttackAction*>(action) || dynamic_cast<MeleeAction*>(action))
            return 1.0f;
        return Ragefire::InHazard(botAI) || Ragefire::GetFocus(botAI) ? 0.0f : 1.0f;
    }
};
}  // namespace

void RagefireStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    if (!sPlayerbotAIConfig.EnableRagefireMechanics)
        return;
    triggers.push_back(new TriggerNode("ragefire hazard", {NextAction("ragefire escape", ACTION_INTERRUPT - 1)}));
    triggers.push_back(new TriggerNode("ragefire focus", {NextAction("ragefire focus", ACTION_RAID + 1)}));
}

void RagefireStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    if (!sPlayerbotAIConfig.EnableRagefireMechanics)
        return;
    multipliers.push_back(new RagefireHazardMultiplier(botAI));
    multipliers.push_back(new RagefireFocusMultiplier(botAI));
}
