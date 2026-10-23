/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GNOMEREGANSHARED_H
#define PLAYERBOTS_GNOMEREGANSHARED_H

#include <vector>

#include "ObjectGuid.h"
#include "Position.h"

class PlayerbotAI;
class Unit;
class GameObject;

namespace Gnomeregan
{
constexpr uint32 MAP_ID = 90;
struct Hazard
{
    Position center;
    float radius;
};
using Hazards = std::vector<Hazard>;
bool Enabled(PlayerbotAI* botAI);
Hazards CollectHazards(PlayerbotAI* botAI);
bool InHazard(PlayerbotAI* botAI);
bool BlockApproach(PlayerbotAI* botAI);
bool SafeProgress(Hazards const& hazards, Position const& from, Position const& to, bool requireProgress = true);
ObjectGuid FindButton(PlayerbotAI* botAI);
GameObject* GetButton(PlayerbotAI* botAI, ObjectGuid guid);
bool HasReflect(PlayerbotAI* botAI);
bool HasExplicitTarget(PlayerbotAI* botAI);
ObjectGuid FindFocus(PlayerbotAI* botAI);
Unit* GetFocus(PlayerbotAI* botAI);
bool CanFocus(PlayerbotAI* botAI, Unit* target);
}  // namespace Gnomeregan
#endif
