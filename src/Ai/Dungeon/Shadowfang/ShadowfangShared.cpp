/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ShadowfangShared.h"

#include "Creature.h"
#include "DynamicObject.h"
#include "Group.h"
#include "MotionMaster.h"
#include "Pet.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"

namespace
{
constexpr uint32 SPELL_CHAIN_REACTION = 68821;
constexpr uint32 SPELL_PERFUME_SPILL = 68798;
constexpr uint32 SPELL_COLOGNE_SPILL = 68614;
constexpr uint32 SPELL_PERFUME_IMMUNITY = 68529;
constexpr uint32 SPELL_COLOGNE_IMMUNITY = 68530;
constexpr float ESCAPE_MARGIN = 3.0f;

void IncludeHazard(Shadowfang::Hazard& result, Position const& center, float radius)
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

namespace Shadowfang
{
Hazard CollectHazard(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (!sPlayerbotAIConfig.EnableShadowfangMechanics || bot->GetMapId() != MAP_ID || !bot->IsAlive())
        return {};
    Hazard result;
    GuidVector const attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    for (ObjectGuid const guid : attackers)
    {
        Unit* enemy = botAI->GetUnit(guid);
        if (!enemy || !enemy->IsAlive() || !enemy->IsInWorld() || enemy->GetMap() != bot->GetMap())
            continue;
        Spell* spell = enemy->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (!spell || spell->m_spellInfo->Id != SPELL_CHAIN_REACTION)
            continue;
        float radius = 0.0f;
        for (SpellEffectInfo const& effect : spell->m_spellInfo->Effects)
            radius = std::max(radius, effect.CalcRadius(enemy));
        if (radius <= 0.0f || radius > sPlayerbotAIConfig.MaxAoeAvoidRadius ||
            bot->GetExactDist(enemy) >= radius + ESCAPE_MARGIN)
            continue;
        IncludeHazard(result, enemy->GetPosition(), radius + ESCAPE_MARGIN);
    }
    // DynObjAura applies this periodic dummy to hostile players; its tick self-casts spill damage.
    for (AuraEffect* effect : bot->GetAuraEffectsByType(SPELL_AURA_PERIODIC_DUMMY))
    {
        Aura* aura = effect ? effect->GetBase() : nullptr;
        if (!aura || aura->IsRemoved() || aura->GetType() != DYNOBJ_AURA_TYPE ||
            (aura->GetId() != SPELL_PERFUME_SPILL && aura->GetId() != SPELL_COLOGNE_SPILL) ||
            bot->HasAura(aura->GetId() == SPELL_PERFUME_SPILL ? SPELL_PERFUME_IMMUNITY : SPELL_COLOGNE_IMMUNITY))
            continue;
        DynamicObject* area = aura->GetDynobjOwner();
        if (!area || !area->IsInWorld() || area->GetRadius() <= 0.0f ||
            area->GetRadius() > sPlayerbotAIConfig.MaxAoeAvoidRadius)
            continue;
        IncludeHazard(result, area->GetPosition(), area->GetRadius() + ESCAPE_MARGIN);
    }
    return result;
}

bool InHazard(PlayerbotAI* botAI)
{
    return sPlayerbotAIConfig.EnableShadowfangMechanics && botAI->GetBot()->GetMapId() == MAP_ID &&
           botAI->GetAiObjectContext()->GetValue<Hazard>("shadowfang hazard")->Get().active;
}

bool Enabled(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    return sPlayerbotAIConfig.EnableShadowfangMechanics && bot->GetMapId() == MAP_ID && bot->IsAlive();
}
bool IsCursedAlly(PlayerbotAI* botAI, Unit* target)
{
    Player* bot = botAI->GetBot();
    return Enabled(botAI) && target && target->IsPlayer() && target->IsAlive() && target->IsInWorld() &&
           target->GetMap() == bot->GetMap() && bot->IsInSameRaidWith(target->ToPlayer()) &&
           target->HasAura(SPELL_ARUGALS_CURSE);
}
std::vector<ObjectGuid> CursedParty(PlayerbotAI* botAI)
{
    GuidVector result;
    if (!Enabled(botAI))
        return result;
    Group* group = botAI->GetBot()->GetGroup();
    if (!group)
        return result;
    for (GroupReference* member = group->GetFirstMember(); member; member = member->next())
        if (Player* player = member->GetSource(); IsCursedAlly(botAI, player))
            result.push_back(player->GetGUID());
    return result;
}
uint32 DecurseSpell(PlayerbotAI* botAI)
{
    for (char const* name : {"remove curse", "remove lesser curse", "cleanse spirit"})
    {
        uint32 const spellId = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", name)->Get();
        if (spellId && botAI->GetBot()->HasSpell(spellId))
            return spellId;
    }
    return 0;
}
bool CanDecurse(PlayerbotAI* botAI, Unit* target, uint32 spellId)
{
    if (!IsCursedAlly(botAI, target) || !spellId ||
        !botAI->GetBot()->IsWithinCombatRange(target, botAI->GetRange("spell")) ||
        !botAI->GetBot()->IsWithinLOSInMap(target))
        return false;
    SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
    if (!info || info->CheckExplicitTarget(botAI->GetBot(), target) != SPELL_CAST_OK)
        return false;
    DispelChargesList auras;
    target->GetDispellableAuraList(botAI->GetBot(), 1 << DISPEL_CURSE, auras, info);
    for (auto const& [aura, charges] : auras)
        if (aura->GetId() == SPELL_ARUGALS_CURSE)
            return true;
    return false;
}
ObjectGuid FindDecurseTarget(PlayerbotAI* botAI)
{
    if (!Enabled(botAI))
        return ObjectGuid::Empty;
    uint32 const spellId = DecurseSpell(botAI);
    if (!spellId)
        return ObjectGuid::Empty;
    for (ObjectGuid const guid : botAI->GetAiObjectContext()->GetValue<GuidVector>("shadowfang cursed party")->Get())
        if (Unit* target = botAI->GetUnit(guid); CanDecurse(botAI, target, spellId))
            return guid;
    return ObjectGuid::Empty;
}
Unit* DecurseTarget(PlayerbotAI* botAI)
{
    if (!Enabled(botAI))
        return nullptr;
    Unit* target =
        botAI->GetUnit(botAI->GetAiObjectContext()->GetValue<ObjectGuid>("shadowfang decurse target")->Get());
    return CanDecurse(botAI, target, DecurseSpell(botAI)) ? target : nullptr;
}
bool AreaThreatensAlly(PlayerbotAI* botAI, Unit* caster, Unit* target, SpellInfo const* info)
{
    if (!Enabled(botAI) || !caster || !info || !info->IsAffectingArea())
        return false;
    float radius = 0.0f;
    for (SpellEffectInfo const& effect : info->Effects)
        radius = std::max(radius, effect.CalcRadius(caster));
    if (radius <= 0.0f)
        return false;
    for (ObjectGuid const guid : botAI->GetAiObjectContext()->GetValue<GuidVector>("shadowfang cursed party")->Get())
    {
        Unit* ally = botAI->GetUnit(guid);
        if (!IsCursedAlly(botAI, ally))
            continue;
        // Conservatively cover caster and unit-target areas without suppressing distant safe casts.
        if (caster->GetExactDist(ally) <= radius || (target && target->GetExactDist(ally) <= radius))
            return true;
    }
    return false;
}
bool UnsafeCast(PlayerbotAI* botAI, Unit* caster)
{
    if (!Enabled(botAI) || !caster)
        return false;
    for (CurrentSpellTypes slot : {CURRENT_GENERIC_SPELL, CURRENT_CHANNELED_SPELL, CURRENT_AUTOREPEAT_SPELL})
    {
        Spell* spell = caster->GetCurrentSpell(slot);
        if (!spell || spell->m_spellInfo->IsPositive() || spell->m_spellInfo->HasEffect(SPELL_EFFECT_DISPEL))
            continue;
        if (IsCursedAlly(botAI, botAI->GetUnit(spell->m_targets.GetUnitTargetGUID())))
            return true;
        if (AreaThreatensAlly(botAI, caster, botAI->GetUnit(spell->m_targets.GetUnitTargetGUID()), spell->m_spellInfo))
            return true;
    }
    return false;
}
bool NeedsProtection(PlayerbotAI* botAI)
{
    if (!Enabled(botAI))
        return false;
    Player* bot = botAI->GetBot();
    Pet* pet = bot->GetPet();
    return UnsafeCast(botAI, bot) || UnsafeCast(botAI, pet) || IsCursedAlly(botAI, bot->GetVictim()) ||
           IsCursedAlly(botAI, botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get()) ||
           (pet && IsCursedAlly(botAI, pet->GetVictim()));
}
}  // namespace Shadowfang
