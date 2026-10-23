/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BLACKFATHOMSHARED_H
#define PLAYERBOTS_BLACKFATHOMSHARED_H

#include "ObjectGuid.h"

class PlayerbotAI;
class Unit;

namespace Blackfathom
{
constexpr uint32 MAP_ID = 48;
bool HasExplicitTarget(PlayerbotAI* botAI);
ObjectGuid FindFocus(PlayerbotAI* botAI);
Unit* GetFocus(PlayerbotAI* botAI);
bool CanFocus(PlayerbotAI* botAI, Unit* target);
}  // namespace Blackfathom
#endif
