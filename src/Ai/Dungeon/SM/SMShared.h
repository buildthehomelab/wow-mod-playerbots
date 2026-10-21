/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SMSHARED_H
#define PLAYERBOTS_SMSHARED_H

#include <array>
#include <vector>
class Player;

#include "Common.h"
#include "ObjectGuid.h"
#include "Unit.h"

class PlayerbotAI;

namespace SMShared
{
inline constexpr uint32 SM_MAP_ID = 189;
inline constexpr uint32 SPELL_DETONATION = 9435;
inline constexpr uint32 SPELL_ARCANE_BUBBLE = 9438;
inline constexpr uint32 SPELL_HEROD_WHIRLWIND = 8989;
inline constexpr uint32 SPELL_FIRE_NOVA = 12470;
inline constexpr uint32 SPELL_POISON_CLOUD = 3815;
inline constexpr uint32 SPELL_DOMINATE_MIND = 14515;
inline constexpr float DETONATION_SAFE_DISTANCE = 36.0f;
inline constexpr float HEROD_WHIRLWIND_SAFE_DISTANCE = 12.0f;
inline constexpr float FIRE_NOVA_SAFE_DISTANCE = 14.0f;
inline constexpr float POISON_CLOUD_SAFE_DISTANCE = 12.0f;
inline constexpr uint32 TARGET_CACHE_MS = 250;

enum Encounter : uint8
{
    DOAN,
    HEROD,
    THALNOS,
    IRONSPINE,
    ENCOUNTER_COUNT
};
using EncounterTargets = std::array<ObjectGuid, ENCOUNTER_COUNT>;

bool IsHazardActive(Unit* boss, Encounter encounter);
float SafeDistance(Encounter encounter);
EncounterTargets CollectEncounterTargets(PlayerbotAI* botAI);
Unit* GetEncounterTarget(PlayerbotAI* botAI, Encounter encounter);
bool IsInGroundDamage(PlayerbotAI* botAI);
bool CanFocusTarget(PlayerbotAI* botAI, Unit* target);
ObjectGuid FindFocusTarget(PlayerbotAI* botAI);
Unit* GetFocusTarget(PlayerbotAI* botAI);
bool IsCastingAtDominatedAlly(PlayerbotAI* botAI, Unit* caster);
bool IsDominatedPartyMember(Player* bot, Unit* target);
std::vector<ObjectGuid> CharmedPartyMembers(PlayerbotAI* botAI);
uint32 CharmDispelSpell(PlayerbotAI* botAI);
ObjectGuid FindDominatedAlly(PlayerbotAI* botAI);
Unit* GetDominatedAlly(PlayerbotAI* botAI);
bool CanDispelDominate(PlayerbotAI* botAI, Unit* target, uint32 spellId);

inline bool IsDetonationImminent(Unit* doan)
{
    return doan && doan->IsAlive() && doan->GetMapId() == SM_MAP_ID &&
           (doan->HasAura(SPELL_ARCANE_BUBBLE) || doan->FindCurrentSpellBySpellId(SPELL_DETONATION));
}

inline bool IsHerodWhirlwinding(Unit* herod)
{
    return herod && herod->IsAlive() && herod->GetMapId() == SM_MAP_ID &&
           (herod->HasAura(SPELL_HEROD_WHIRLWIND) || herod->FindCurrentSpellBySpellId(SPELL_HEROD_WHIRLWIND));
}

inline bool IsCastingHazard(Unit* boss, uint32 spellId)
{
    return boss && boss->IsAlive() && boss->GetMapId() == SM_MAP_ID && boss->FindCurrentSpellBySpellId(spellId);
}
}  // namespace SMShared
#endif
