/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "DeadminesShared.h"

#include "Creature.h"
#include "DynamicObject.h"
#include "Group.h"
#include "MotionMaster.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "TargetValue.h"

namespace
{
constexpr uint32 NPC_BLACKGUARD = 636;
constexpr uint32 SPELL_ACID_SPLASH = 6306;
constexpr uint32 SPELL_MELT_ORE = 5159;
constexpr float ESCAPE_MARGIN = 3.0f;
constexpr float MELEE_FOCUS_RANGE = 12.0f;

void IncludeHazard(Deadmines::Hazard& result, Position const& center, float radius)
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

bool FocusAllowed(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (!sPlayerbotAIConfig.EnableDeadminesMechanics || bot->GetMapId() != Deadmines::MAP_ID || !bot->IsInCombat() ||
        PlayerbotAI::IsTank(bot) || PlayerbotAI::IsHeal(bot) ||
        !botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets")->Get().empty())
        return false;
    Group* group = bot->GetGroup();
    return !group || !group->GetTargetIcon(7);
}

uint32 FocusPriority(PlayerbotAI* botAI, Unit* target)
{
    if (!target || !target->IsCreature())
        return 0;
    Unit* victim = target->GetVictim();
    if (target->GetEntry() == NPC_BLACKGUARD && victim && victim->IsPlayer() &&
        !PlayerbotAI::IsTank(victim->ToPlayer()) &&
        (victim == botAI->GetBot() || botAI->GetBot()->IsInSameRaidWith(victim->ToPlayer())))
        return 2;
    MovementGeneratorType const movement = target->GetMotionMaster()->GetCurrentMovementGeneratorType();
    return movement == FLEEING_MOTION_TYPE || movement == TIMED_FLEEING_MOTION_TYPE ? 1 : 0;
}

class FocusSelector : public FindNonCcTargetStrategy
{
public:
    FocusSelector(PlayerbotAI* botAI) : FindNonCcTargetStrategy(botAI) {}
    bool Eligible(Unit* target)
    {
        Player* bot = botAI->GetBot();
        if (!target || !target->IsCreature() || !target->IsAlive() || !target->IsInWorld() ||
            target->GetMap() != bot->GetMap() || !target->IsInCombat() || !bot->IsValidAttackTarget(target) ||
            target->ToCreature()->GetReactState() == REACT_PASSIVE || target->HasBreakableByDamageCrowdControlAura() ||
            IsCcTarget(target) || !bot->IsWithinLOSInMap(target))
            return false;
        float const range = PlayerbotAI::IsRanged(bot) ? botAI->GetRange("spell") : MELEE_FOCUS_RANGE;
        return bot->IsWithinCombatRange(target, range) && FocusPriority(botAI, target);
    }
    void CheckAttacker(Unit* target, ThreatManager* /*threatMgr*/) override
    {
        if (!Eligible(target))
            return;
        uint32 const priority = FocusPriority(botAI, target);
        if (!result || priority > _priority || (priority == _priority && target->GetGUID() < result->GetGUID()))
        {
            result = target;
            _priority = priority;
        }
    }

private:
    uint32 _priority = 0;
};
}  // namespace

namespace Deadmines
{
Hazard CollectHazard(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (!sPlayerbotAIConfig.EnableDeadminesMechanics || bot->GetMapId() != MAP_ID || !bot->IsAlive())
        return {};
    Hazard result;
    for (AuraType type :
         {SPELL_AURA_PERIODIC_DAMAGE, SPELL_AURA_PERIODIC_DAMAGE_PERCENT, SPELL_AURA_PERIODIC_TRIGGER_SPELL,
          SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE, SPELL_AURA_DUMMY})
    {
        for (AuraEffect* effect : bot->GetAuraEffectsByType(type))
        {
            Aura* aura = effect ? effect->GetBase() : nullptr;
            if (!aura || aura->IsRemoved() || aura->IsExpired() || aura->GetType() != DYNOBJ_AURA_TYPE ||
                aura->GetSpellInfo()->IsPositive() ||
                sPlayerbotAIConfig.AoeAvoidSpellWhitelist.contains(aura->GetSpellInfo()->Id))
                continue;
            DynamicObject* area = aura->GetDynobjOwner();
            if (!area || !area->IsInWorld() || area->GetRadius() <= 0.0f ||
                area->GetRadius() > sPlayerbotAIConfig.MaxAoeAvoidRadius || bot->GetDistance(area) > area->GetRadius())
                continue;
            IncludeHazard(result, area->GetPosition(), area->GetRadius());
        }
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
        if (!spell || (spell->m_spellInfo->Id != SPELL_ACID_SPLASH && spell->m_spellInfo->Id != SPELL_MELT_ORE))
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
    return sPlayerbotAIConfig.EnableDeadminesMechanics && botAI->GetBot()->GetMapId() == MAP_ID &&
           botAI->GetAiObjectContext()->GetValue<Hazard>("deadmines hazard")->Get().active;
}

ObjectGuid FindFocus(PlayerbotAI* botAI)
{
    if (!FocusAllowed(botAI))
        return ObjectGuid::Empty;
    FocusSelector selector(botAI);
    GuidVector const attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    for (ObjectGuid const guid : attackers)
        selector.CheckAttacker(botAI->GetUnit(guid), nullptr);
    Unit* target = selector.GetResult();
    return target ? target->GetGUID() : ObjectGuid::Empty;
}

Unit* GetFocus(PlayerbotAI* botAI)
{
    if (!FocusAllowed(botAI))
        return nullptr;
    Unit* target = botAI->GetUnit(botAI->GetAiObjectContext()->GetValue<ObjectGuid>("deadmines focus")->Get());
    if (!target || !target->IsCreature() || !target->IsAlive() || !target->IsInWorld() ||
        target->GetMap() != botAI->GetBot()->GetMap() || !target->IsInCombat() ||
        !botAI->GetBot()->IsValidAttackTarget(target) || !FocusPriority(botAI, target) ||
        target->ToCreature()->GetReactState() == REACT_PASSIVE || target->HasBreakableByDamageCrowdControlAura() ||
        !botAI->GetBot()->IsWithinLOSInMap(target))
        return nullptr;
    float const range = PlayerbotAI::IsRanged(botAI->GetBot()) ? botAI->GetRange("spell") : MELEE_FOCUS_RANGE;
    return botAI->GetBot()->IsWithinCombatRange(target, range) ? target : nullptr;
}

bool CanFocus(PlayerbotAI* botAI, Unit* target) { return FocusAllowed(botAI) && FocusSelector(botAI).Eligible(target); }
}  // namespace Deadmines
