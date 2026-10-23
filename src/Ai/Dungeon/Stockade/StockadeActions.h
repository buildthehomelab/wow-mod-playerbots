/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_STOCKADEACTIONS_H
#define PLAYERBOTS_STOCKADEACTIONS_H

#include "AttackAction.h"

class StockadeFocusAction : public AttackAction
{
public:
    StockadeFocusAction(PlayerbotAI* botAI) : AttackAction(botAI, "stockade focus") {}
    bool isUseful() override;
    bool Execute(Event event) override;

private:
    Unit* ValidatedTarget();
};
#endif
