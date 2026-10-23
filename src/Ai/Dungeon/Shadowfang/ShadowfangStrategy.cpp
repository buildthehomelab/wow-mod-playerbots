/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ShadowfangStrategy.h"

#include "GenericSpellActions.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "ShadowfangActions.h"
#include "ShadowfangShared.h"
#include "SpellInfo.h"
#include "SpellMgr.h"

namespace
{
class ShadowfangHazardMultiplier : public Multiplier
{
public:
    ShadowfangHazardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "shadowfang hazard") {}
    float GetValue(Action* action) override
    {
        if (dynamic_cast<ShadowfangEscapeAction*>(action))
            return 1.0f;
        if (!dynamic_cast<MovementAction*>(action))
            return 1.0f;
        return Shadowfang::InHazard(botAI) ? 0.0f : 1.0f;
    }
};
class ShadowfangCharmMultiplier : public Multiplier
{
public:
    ShadowfangCharmMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "shadowfang charm") {}
    float GetValue(Action* action) override
    {
        if (!Shadowfang::Enabled(botAI))
            return 1.0f;
        CastSpellAction* cast = dynamic_cast<CastSpellAction*>(action);
        if (cast)
        {
            uint32 const spellId = context->GetValue<uint32>("spell id", cast->getSpell())->Get();
            SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
            if (!info || info->IsPositive() || info->HasEffect(SPELL_EFFECT_DISPEL))
                return 1.0f;
            if (Shadowfang::AreaThreatensAlly(botAI, bot, action->GetTarget(), info))
                return 0.0f;
        }
        if (!cast && !dynamic_cast<AttackAction*>(action))
            return 1.0f;
        return Shadowfang::IsCursedAlly(botAI, action->GetTarget()) ? 0.0f : 1.0f;
    }
};
}  // namespace

void ShadowfangStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    if (!sPlayerbotAIConfig.EnableShadowfangMechanics)
        return;
    triggers.push_back(new TriggerNode("shadowfang protect", {NextAction("shadowfang protect", ACTION_EMERGENCY + 1)}));
    triggers.push_back(new TriggerNode("shadowfang hazard", {NextAction("shadowfang escape", ACTION_INTERRUPT - 1)}));
    triggers.push_back(new TriggerNode("shadowfang decurse", {NextAction("shadowfang decurse", ACTION_DISPEL + 1)}));
}

void ShadowfangStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    if (!sPlayerbotAIConfig.EnableShadowfangMechanics)
        return;
    multipliers.push_back(new ShadowfangHazardMultiplier(botAI));
    multipliers.push_back(new ShadowfangCharmMultiplier(botAI));
}
