/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BLACKFATHOMAVOIDANCE_H
#define PLAYERBOTS_BLACKFATHOMAVOIDANCE_H

#include "ObjectGuid.h"
#include "Position.h"

class PlayerbotAI;
class Unit;

namespace Blackfathom
{

constexpr uint32 CACHE_MS = 250;
struct Hazard
{
    bool active = false;
    Position center;
    float radius = 0.0f;
};
Hazard CollectHazard(PlayerbotAI* botAI);
bool InHazard(PlayerbotAI* botAI);
}  // namespace Blackfathom
#endif
