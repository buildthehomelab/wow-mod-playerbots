/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_WAILINGCAVERNSACTIONS_H
#define PLAYERBOTS_WAILINGCAVERNSACTIONS_H

#include "AttackAction.h"

class WailingCavernsFocusAction : public AttackAction
{
public:
    WailingCavernsFocusAction(PlayerbotAI* botAI) : AttackAction(botAI, "wailing caverns focus") {}
    bool isUseful() override;
    bool Execute(Event event) override;

private:
    Unit* ValidatedTarget();
};
#endif
