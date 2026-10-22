/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SHADOWFANGSHARED_H
#define PLAYERBOTS_SHADOWFANGSHARED_H

#include <vector>

#include "ObjectGuid.h"
#include "Position.h"

class PlayerbotAI;
class Unit;
class SpellInfo;

namespace Shadowfang
{
constexpr uint32 MAP_ID = 33;
constexpr uint32 CACHE_MS = 250;
struct Hazard
{
    bool active = false;
    Position center;
    float radius = 0.0f;
};
Hazard CollectHazard(PlayerbotAI* botAI);
bool InHazard(PlayerbotAI* botAI);
constexpr uint32 SPELL_ARUGALS_CURSE = 7621;
bool Enabled(PlayerbotAI* botAI);
bool IsCursedAlly(PlayerbotAI* botAI, Unit* target);
std::vector<ObjectGuid> CursedParty(PlayerbotAI* botAI);
uint32 DecurseSpell(PlayerbotAI* botAI);
ObjectGuid FindDecurseTarget(PlayerbotAI* botAI);
Unit* DecurseTarget(PlayerbotAI* botAI);
bool CanDecurse(PlayerbotAI* botAI, Unit* target, uint32 spellId);
bool NeedsProtection(PlayerbotAI* botAI);
bool AreaThreatensAlly(PlayerbotAI* botAI, Unit* caster, Unit* target, SpellInfo const* info);
bool UnsafeCast(PlayerbotAI* botAI, Unit* caster);
}  // namespace Shadowfang
#endif
