/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_STOCKADESHARED_H
#define PLAYERBOTS_STOCKADESHARED_H

#include "ObjectGuid.h"

class PlayerbotAI;
class Unit;

namespace Stockade
{
constexpr uint32 MAP_ID = 34;
bool HasExplicitTarget(PlayerbotAI* botAI);
ObjectGuid FindFocus(PlayerbotAI* botAI);
Unit* GetFocus(PlayerbotAI* botAI);
bool CanFocus(PlayerbotAI* botAI, Unit* target);
}  // namespace Stockade
#endif
