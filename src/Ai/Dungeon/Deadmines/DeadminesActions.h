/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_DEADMINESACTIONS_H
#define PLAYERBOTS_DEADMINESACTIONS_H

#include "AttackAction.h"
#include "MovementActions.h"

class DeadminesFocusAction : public AttackAction
{
public:
    DeadminesFocusAction(PlayerbotAI* botAI) : AttackAction(botAI, "deadmines focus") {}
    bool isUseful() override;
    bool Execute(Event event) override;

private:
    Unit* ValidatedTarget();
};
class DeadminesEscapeAction : public MovementAction
{
public:
    DeadminesEscapeAction(PlayerbotAI* botAI) : MovementAction(botAI, "deadmines escape") {}
    bool isUseful() override;
    bool Execute(Event event) override;

private:
    uint32 _lastAttemptMs = 0;
    Position _destination;
};
#endif
