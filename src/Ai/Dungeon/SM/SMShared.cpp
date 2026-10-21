/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SMShared.h"

#include "Creature.h"
#include "DynamicObject.h"
#include "Group.h"
#include "ObjectAccessor.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Spell.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "TargetValue.h"

namespace
{
constexpr uint32 ENCOUNTER_ENTRIES[] = {6487, 3975, 4543, 6489};
constexpr uint32 NPC_WHITEMANE = 3977;
constexpr uint32 NPC_FAIRBANKS = 4542;
constexpr uint32 NPC_SCARLET_PROTECTOR = 4292;
constexpr uint32 NPC_SCARLET_ADEPT = 4296;
constexpr uint32 NPC_SCARLET_CHAPLAIN = 4299;
constexpr uint32 NPC_SCARLET_ABBOT = 4303;
constexpr float MELEE_FOCUS_RANGE = 12.0f;
constexpr float CHARM_DISPEL_RANGE = 30.0f;

class ScarletFocusStrategy : public FindNonCcTargetStrategy
{
public:
    ScarletFocusStrategy(PlayerbotAI* botAI) : FindNonCcTargetStrategy(botAI) {}
    bool Eligible(Unit* target)
    {
        Player* bot = botAI->GetBot();
        if (!target || !target->IsCreature() || !target->IsInCombat() || !bot->IsValidAttackTarget(target) ||
            target->ToCreature()->GetReactState() == REACT_PASSIVE || target->HasBreakableByDamageCrowdControlAura() ||
            IsCcTarget(target) || !bot->IsWithinLOSInMap(target))
            return false;

        float const range = PlayerbotAI::IsRanged(bot) ? botAI->GetRange("spell") : MELEE_FOCUS_RANGE;
        return bot->IsWithinCombatRange(target, range);
    }
    void CheckAttacker(Unit* target, ThreatManager* /*threatMgr*/) override
    {
        if (!Eligible(target))
            return;
        uint32 priority = 0;
        switch (target->GetEntry())
        {
            case NPC_WHITEMANE:
                priority = 3;
                break;
            case NPC_FAIRBANKS:
            case NPC_SCARLET_PROTECTOR:
            case NPC_SCARLET_ADEPT:
            case NPC_SCARLET_CHAPLAIN:
            case NPC_SCARLET_ABBOT:
                priority = 1;
                break;
            default:
                return;
        }
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

namespace SMShared
{
bool IsHazardActive(Unit* boss, Encounter encounter)
{
    switch (encounter)
    {
        case DOAN:
            return IsDetonationImminent(boss);
        case HEROD:
            return IsHerodWhirlwinding(boss);
        case THALNOS:
            return IsCastingHazard(boss, SPELL_FIRE_NOVA);
        case IRONSPINE:
            return IsCastingHazard(boss, SPELL_POISON_CLOUD);
        default:
            return false;
    }
}

float SafeDistance(Encounter encounter)
{
    switch (encounter)
    {
        case DOAN:
            return DETONATION_SAFE_DISTANCE;
        case HEROD:
            return HEROD_WHIRLWIND_SAFE_DISTANCE;
        case THALNOS:
            return FIRE_NOVA_SAFE_DISTANCE;
        case IRONSPINE:
            return POISON_CLOUD_SAFE_DISTANCE;
        default:
            return 0.0f;
    }
}

EncounterTargets CollectEncounterTargets(PlayerbotAI* botAI)
{
    EncounterTargets result{};
    Player* bot = botAI->GetBot();
    if (bot->GetMapId() != SM_MAP_ID || !bot->IsInCombat())
        return result;

    for (auto const& [guid, ref] : bot->GetThreatMgr().GetThreatenedByMeList())
    {
        Unit* unit = ref->GetOwner();
        if (!unit || !unit->IsAlive())
            continue;
        for (uint8 i = 0; i < ENCOUNTER_COUNT; ++i)
            if (unit->GetEntry() == ENCOUNTER_ENTRIES[i])
                result[i] = unit->GetGUID();
    }
    return result;
}

Unit* GetEncounterTarget(PlayerbotAI* botAI, Encounter encounter)
{
    if (botAI->GetBot()->GetMapId() != SM_MAP_ID)
        return nullptr;
    EncounterTargets const targets =
        botAI->GetAiObjectContext()->GetValue<EncounterTargets>("scarlet encounter targets")->Get();
    Unit* target = botAI->GetUnit(targets[encounter]);
    return target && target->IsAlive() && target->GetMapId() == SM_MAP_ID ? target : nullptr;
}

bool IsInGroundDamage(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (bot->GetMapId() != SM_MAP_ID)
        return false;
    Aura* aura = botAI->GetAiObjectContext()->GetValue<Aura*>("area debuff")->Get();
    if (!aura || aura->IsRemoved() || aura->IsExpired() || aura->GetType() != DYNOBJ_AURA_TYPE)
        return false;
    if (sPlayerbotAIConfig.AoeAvoidSpellWhitelist.contains(aura->GetSpellInfo()->Id))
        return false;
    DynamicObject* area = aura->GetDynobjOwner();
    return area && area->IsInWorld() && area->GetRadius() > 0.0f &&
           area->GetRadius() <= sPlayerbotAIConfig.MaxAoeAvoidRadius && bot->GetDistance(area) <= area->GetRadius();
}

bool CanFocusTarget(PlayerbotAI* botAI, Unit* target) { return ScarletFocusStrategy(botAI).Eligible(target); }

ObjectGuid FindFocusTarget(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (bot->GetMapId() != SM_MAP_ID || !bot->IsInCombat() || PlayerbotAI::IsTank(bot) || PlayerbotAI::IsHeal(bot) ||
        !botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets")->Get().empty())
        return ObjectGuid::Empty;
    if (Group* group = bot->GetGroup())
        if (group->GetTargetIcon(7))
            return ObjectGuid::Empty;

    ScarletFocusStrategy strategy(botAI);
    GuidVector const attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    for (ObjectGuid const guid : attackers)
        strategy.CheckAttacker(botAI->GetUnit(guid), nullptr);
    Unit* target = strategy.GetResult();
    return target ? target->GetGUID() : ObjectGuid::Empty;
}

Unit* GetFocusTarget(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (bot->GetMapId() != SM_MAP_ID || !bot->IsInCombat() || PlayerbotAI::IsTank(bot) || PlayerbotAI::IsHeal(bot))
        return nullptr;
    if (!botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets")->Get().empty())
        return nullptr;
    if (Group* group = bot->GetGroup())
        if (group->GetTargetIcon(7))
            return nullptr;
    Unit* target = botAI->GetUnit(botAI->GetAiObjectContext()->GetValue<ObjectGuid>("scarlet focus target")->Get());
    return target && target->IsCreature() && target->ToCreature()->GetReactState() != REACT_PASSIVE &&
                   bot->IsValidAttackTarget(target) && target->IsInCombat() &&
                   !target->HasBreakableByDamageCrowdControlAura()
               ? target
               : nullptr;
}

bool IsDominatedPartyMember(Player* bot, Unit* target)
{
    return bot->GetMapId() == SM_MAP_ID && target && target->IsPlayer() && target->IsAlive() &&
           target->GetMapId() == SM_MAP_ID && bot->IsInSameRaidWith(target->ToPlayer()) &&
           target->HasAura(SPELL_DOMINATE_MIND);
}

bool IsCastingAtDominatedAlly(PlayerbotAI* botAI, Unit* caster)
{
    if (!caster || botAI->GetBot()->GetMapId() != SM_MAP_ID)
        return false;
    for (CurrentSpellTypes slot : {CURRENT_GENERIC_SPELL, CURRENT_CHANNELED_SPELL})
    {
        Spell* spell = caster->GetCurrentSpell(slot);
        if (!spell || spell->m_spellInfo->IsPositive() || spell->m_spellInfo->HasEffect(SPELL_EFFECT_DISPEL))
            continue;
        if (IsDominatedPartyMember(botAI->GetBot(), botAI->GetUnit(spell->m_targets.GetUnitTargetGUID())))
            return true;
        if (spell->m_spellInfo->IsAffectingArea() &&
            !botAI->GetAiObjectContext()->GetValue<GuidVector>("scarlet charmed party")->Get().empty())
            return true;
    }
    return false;
}

std::vector<ObjectGuid> CharmedPartyMembers(PlayerbotAI* botAI)
{
    GuidVector result;
    Player* bot = botAI->GetBot();
    if (bot->GetMapId() != SM_MAP_ID || !bot->IsInCombat() || !bot->GetGroup())
        return result;
    for (GroupReference* member = bot->GetGroup()->GetFirstMember(); member; member = member->next())
    {
        Player* player = member->GetSource();
        if (IsDominatedPartyMember(bot, player))
            result.push_back(player->GetGUID());
    }
    return result;
}

uint32 CharmDispelSpell(PlayerbotAI* botAI)
{
    for (char const* name : {"dispel magic", "purge"})
    {
        uint32 const spellId = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", name)->Get();
        if (spellId && botAI->GetBot()->HasSpell(spellId))
            return spellId;
    }
    return 0;
}

bool CanDispelDominate(PlayerbotAI* botAI, Unit* target, uint32 spellId)
{
    Player* bot = botAI->GetBot();
    if (bot->GetMapId() != SM_MAP_ID || !target || !target->IsPlayer() || !target->IsAlive() ||
        !bot->IsInSameRaidWith(target->ToPlayer()) || target->GetMapId() != SM_MAP_ID ||
        !target->HasAura(SPELL_DOMINATE_MIND) || !bot->IsWithinCombatRange(target, CHARM_DISPEL_RANGE) ||
        !bot->IsWithinLOSInMap(target))
        return false;
    SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
    if (!info)
        return false;
    DispelChargesList auras;
    target->GetDispellableAuraList(bot, 1 << DISPEL_MAGIC, auras, info);
    for (auto const& [aura, charges] : auras)
        if (aura->GetId() == SPELL_DOMINATE_MIND)
            return true;
    return false;
}

ObjectGuid FindDominatedAlly(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (bot->GetMapId() != SM_MAP_ID || !bot->IsInCombat() || !bot->GetGroup())
        return ObjectGuid::Empty;
    uint32 const spellId = CharmDispelSpell(botAI);
    if (!spellId)
        return ObjectGuid::Empty;
    GuidVector const targets = botAI->GetAiObjectContext()->GetValue<GuidVector>("scarlet charmed party")->Get();
    for (ObjectGuid const guid : targets)
    {
        Unit* player = botAI->GetUnit(guid);
        if (CanDispelDominate(botAI, player, spellId))
            return guid;
    }
    return ObjectGuid::Empty;
}

Unit* GetDominatedAlly(PlayerbotAI* botAI)
{
    Unit* target = botAI->GetUnit(botAI->GetAiObjectContext()->GetValue<ObjectGuid>("scarlet dominated ally")->Get());
    return target && target->HasAura(SPELL_DOMINATE_MIND) ? target : nullptr;
}
}  // namespace SMShared
