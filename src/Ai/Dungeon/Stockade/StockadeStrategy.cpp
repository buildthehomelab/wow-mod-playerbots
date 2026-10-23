/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "StockadeStrategy.h"

#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "StockadeActions.h"
#include "StockadeShared.h"

namespace
{
class StockadeFocusMultiplier : public Multiplier
{
public:
    StockadeFocusMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "stockade focus") {}
    float GetValue(Action* action) override
    {
        if (!dynamic_cast<AttackAction*>(action) || dynamic_cast<StockadeFocusAction*>(action) ||
            dynamic_cast<MeleeAction*>(action) || dynamic_cast<AttackMyTargetAction*>(action) ||
            dynamic_cast<AttackDuelOpponentAction*>(action))
            return 1.0f;
        return Stockade::GetFocus(botAI) ? 0.0f : 1.0f;
    }
};
}  // namespace

void StockadeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    if (!sPlayerbotAIConfig.EnableStockadeMechanics)
        return;
    triggers.push_back(new TriggerNode("stockade focus", {NextAction("stockade focus", ACTION_RAID + 1)}));
}

void StockadeStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    if (!sPlayerbotAIConfig.EnableStockadeMechanics)
        return;
    multipliers.push_back(new StockadeFocusMultiplier(botAI));
}
