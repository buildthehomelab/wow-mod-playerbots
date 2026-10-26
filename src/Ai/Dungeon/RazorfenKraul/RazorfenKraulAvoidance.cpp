/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RazorfenKraulAvoidance.h"

#include <cmath>

#include "Creature.h"
#include "DBCStores.h"
#include "GameObject.h"
#include "Group.h"
#include "MotionMaster.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "RazorfenKraulShared.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "TargetValue.h"

namespace
{
constexpr uint32 SPELL_WHIRLING_BARRAGE = 8259;
constexpr float ESCAPE_MARGIN = 3.0f;

bool IsContactFire(uint32 entry)
{
    switch (entry)
    {
        case 69421:
        case 69422:
        case 69423:
        case 69424:
        case 69425:
        case 69426:
        case 69427:
        case 69428:
        case 69429:
        case 69430:
        case 69431:
        case 69432:
        case 69433:
        case 69434:
        case 69435:
        case 69436:
        case 69437:
        case 69438:
        case 74075:
        case 74076:
        case 74078:
            return true;
        default:
            return false;
    }
}

void IncludeHazard(RazorfenKraul::Hazard& result, Position const& center, float radius)
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

namespace RazorfenKraul
{
Hazard CollectHazard(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (!sPlayerbotAIConfig.EnableRazorfenKraulMechanics || bot->GetMapId() != MAP_ID || !bot->IsAlive())
        return {};
    Hazard result;
    // go_flames checks 0.3-yard contact every three seconds; leave that contact area.
    constexpr float CONTACT_FIRE_RADIUS = 1.3f;
    GuidVector const objects = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest game objects no los")->Get();
    for (ObjectGuid const guid : objects)
    {
        GameObject* object = botAI->GetGameObject(guid);
        if (!object || !object->IsInWorld() || !object->isSpawned() || object->GetMap() != bot->GetMap() ||
            !IsContactFire(object->GetEntry()) ||
            !object->IsInRange3d(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), CONTACT_FIRE_RADIUS))
            continue;
        float radius = object->GetObjectSize() + CONTACT_FIRE_RADIUS;
        if (GameObjectDisplayInfoEntry const* display =
                sGameObjectDisplayInfoStore.LookupEntry(object->GetGOInfo()->displayId))
        {
            float const scale = object->GetObjectScale();
            float const x = std::max(std::abs(display->minX * scale), std::abs(display->maxX * scale));
            float const y = std::max(std::abs(display->minY * scale), std::abs(display->maxY * scale));
            // IsInRange3d expands each model axis; enclose the expanded rectangle even after rotation.
            radius = std::hypot(x + CONTACT_FIRE_RADIUS, y + CONTACT_FIRE_RADIUS);
        }
        if (radius <= sPlayerbotAIConfig.MaxAoeAvoidRadius)
            IncludeHazard(result, object->GetPosition(), radius);
    }
    if (!bot->IsInCombat())
        return result;
    GuidVector const attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    for (ObjectGuid const guid : attackers)
    {
        Unit* enemy = botAI->GetUnit(guid);
        if (!enemy || !enemy->IsAlive() || !enemy->IsInWorld() || enemy->GetMap() != bot->GetMap())
            continue;
        Spell* spell = enemy->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (!spell || spell->m_spellInfo->Id != SPELL_WHIRLING_BARRAGE)
            continue;
        float radius = 0.0f;
        for (SpellEffectInfo const& effect : spell->m_spellInfo->Effects)
            radius = std::max(radius, effect.CalcRadius(enemy));
        if (radius <= 0.0f || radius > sPlayerbotAIConfig.MaxAoeAvoidRadius ||
            bot->GetExactDist(enemy->GetPositionX(), enemy->GetPositionY(), enemy->GetPositionZ()) >=
                radius + ESCAPE_MARGIN)
            continue;
        IncludeHazard(result, enemy->GetPosition(), radius + ESCAPE_MARGIN);
    }
    return result;
}

bool InHazard(PlayerbotAI* botAI)
{
    return sPlayerbotAIConfig.EnableRazorfenKraulMechanics && botAI->GetBot()->GetMapId() == MAP_ID &&
           botAI->GetAiObjectContext()->GetValue<Hazard>("razorfen kraul hazard")->Get().active;
}

}  // namespace RazorfenKraul
