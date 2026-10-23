/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GnomereganStrategy.h"

#include "Common/Charmed.h"
#include "Common/SpellReflect.h"
#include "GnomereganActions.h"
#include "GnomereganShared.h"
#include "Playerbots.h"

namespace
{
class GnomereganFocusMultiplier : public Multiplier
{
public:
    GnomereganFocusMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "gnomeregan focus") {}
    float GetValue(Action* action) override
    {
        if (!dynamic_cast<AttackAction*>(action) || dynamic_cast<GnomereganFocusAction*>(action) ||
            dynamic_cast<MeleeAction*>(action) || dynamic_cast<AttackMyTargetAction*>(action) ||
            dynamic_cast<AttackDuelOpponentAction*>(action) || dynamic_cast<RcLeaveCharmedTargetAction*>(action))
            return 1.0f;
        return Gnomeregan::GetFocus(botAI) ? 0.0f : 1.0f;
    }
};
class GnomereganMovementMultiplier : public Multiplier
{
public:
    GnomereganMovementMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "gnomeregan movement") {}
    float GetValue(Action* action) override
    {
        if (!Gnomeregan::Enabled(botAI) || !dynamic_cast<MovementAction*>(action) ||
            dynamic_cast<AttackAction*>(action) || dynamic_cast<GnomereganEscapeAction*>(action) ||
            dynamic_cast<AvoidAoeAction*>(action) || dynamic_cast<FleeAction*>(action) ||
            dynamic_cast<FleeWithPetAction*>(action) || dynamic_cast<RunAwayAction*>(action))
            return 1.0f;
        if (!context->GetValue<bool>("gnomeregan hazard")->Get())
        {
            Unit* hostile = context->GetValue<Unit*>("current target")->Get();
            bool const towardHostile = hostile && bot->IsValidAttackTarget(hostile) &&
                                       (action->GetTarget() == hostile || dynamic_cast<TankFaceAction*>(action) ||
                                        dynamic_cast<SetBehindTargetAction*>(action));
            if (!towardHostile)
                return 1.0f;
        }
        return context->GetValue<bool>("gnomeregan block approach")->Get() ? 0.0f : 1.0f;
    }
};
class GnomereganReflectMultiplier : public RcSpellReflectMultiplier
{
public:
    GnomereganReflectMultiplier(PlayerbotAI* botAI) : RcSpellReflectMultiplier(botAI) {}
    float GetValue(Action* action) override
    {
        if (!Gnomeregan::Enabled(botAI) || !botAI->GetAiObjectContext()->GetValue<bool>("gnomeregan reflect")->Get())
            return 1.0f;
        return RcSpellReflectMultiplier::GetValue(action);
    }
};
}  // namespace

void GnomereganStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("gnomeregan hazard", {NextAction("gnomeregan escape", ACTION_EMERGENCY + 5)}));
    triggers.push_back(new TriggerNode("gnomeregan button", {NextAction("gnomeregan button", ACTION_RAID + 2)}));
    triggers.push_back(new TriggerNode("gnomeregan focus", {NextAction("gnomeregan focus", ACTION_RAID + 1)}));
}

void GnomereganStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new GnomereganFocusMultiplier(botAI));
    multipliers.push_back(new GnomereganMovementMultiplier(botAI));
    multipliers.push_back(new GnomereganReflectMultiplier(botAI));
}
