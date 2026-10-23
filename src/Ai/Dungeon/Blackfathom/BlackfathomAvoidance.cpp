/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BlackfathomAvoidance.h"

#include "BlackfathomShared.h"
#include "Creature.h"
#include "Group.h"
#include "MotionMaster.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "TargetValue.h"

namespace
{
constexpr uint32 SPELL_POISON_CLOUD = 3815;
constexpr uint32 SPELL_SLEEP = 8399;
constexpr float ESCAPE_MARGIN = 3.0f;

void IncludeHazard(Blackfathom::Hazard& result, Position const& center, float radius)
{
    if (!result.active)
    {
        result = {true, center, radius};
        return;
    }
    float const distance = result.center.GetExactDist2d(center.GetPositionX(), center.GetPositionY());
    if (distance + radius <= result.radius)
        return;
    if (distance + result.radius <= radius)
    {
        result = {true, center, radius};
        return;
    }
    // Use one escape objective for all currently overlapping hazards.
    float const enclosingRadius = (distance + result.radius + radius) / 2.0f;
    float const ratio = (enclosingRadius - result.radius) / distance;
    result.center.Relocate(
        result.center.GetPositionX() + ratio * (center.GetPositionX() - result.center.GetPositionX()),
        result.center.GetPositionY() + ratio * (center.GetPositionY() - result.center.GetPositionY()),
        result.center.GetPositionZ());
    result.radius = enclosingRadius;
}

}  // namespace

namespace Blackfathom
{
Hazard CollectHazard(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (!sPlayerbotAIConfig.EnableBlackfathomMechanics || bot->GetMapId() != MAP_ID || !bot->IsAlive())
        return {};
    Hazard result;
    if (!bot->IsInCombat())
        return result;
    GuidVector const attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    for (ObjectGuid const guid : attackers)
    {
        Unit* enemy = botAI->GetUnit(guid);
        if (!enemy || !enemy->IsAlive() || !enemy->IsInWorld() || enemy->GetMap() != bot->GetMap())
            continue;
        Spell* spell = enemy->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (!spell || (spell->m_spellInfo->Id != SPELL_POISON_CLOUD && spell->m_spellInfo->Id != SPELL_SLEEP))
            continue;
        float radius = 0.0f;
        for (SpellEffectInfo const& effect : spell->m_spellInfo->Effects)
            radius = std::max(radius, effect.CalcRadius(enemy));
        if (radius <= 0.0f || radius > sPlayerbotAIConfig.MaxAoeAvoidRadius ||
            bot->GetExactDist2d(enemy) >= radius + ESCAPE_MARGIN)
            continue;
        IncludeHazard(result, enemy->GetPosition(), radius + ESCAPE_MARGIN);
    }
    return result;
}

bool InHazard(PlayerbotAI* botAI)
{
    return sPlayerbotAIConfig.EnableBlackfathomMechanics && botAI->GetBot()->GetMapId() == MAP_ID &&
           botAI->GetAiObjectContext()->GetValue<Hazard>("blackfathom hazard")->Get().active;
}

}  // namespace Blackfathom
