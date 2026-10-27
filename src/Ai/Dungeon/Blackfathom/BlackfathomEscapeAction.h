/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_BLACKFATHOMESCAPEACTION_H
#define PLAYERBOTS_BLACKFATHOMESCAPEACTION_H

#include "AttackAction.h"
#include "MovementActions.h"

class BlackfathomEscapeAction : public MovementAction
{
public:
    BlackfathomEscapeAction(PlayerbotAI* botAI) : MovementAction(botAI, "blackfathom escape") {}
    bool isUseful() override;
    bool Execute(Event event) override;

private:
    uint32 _lastAttemptMs = 0;
    Position _destination;
};
#endif
