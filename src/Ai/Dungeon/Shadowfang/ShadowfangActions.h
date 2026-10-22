/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SHADOWFANGACTIONS_H
#define PLAYERBOTS_SHADOWFANGACTIONS_H

#include "AttackAction.h"
#include "MovementActions.h"

class ShadowfangEscapeAction : public MovementAction
{
public:
    ShadowfangEscapeAction(PlayerbotAI* botAI) : MovementAction(botAI, "shadowfang escape") {}
    bool isUseful() override;
    bool Execute(Event event) override;

private:
    uint32 _lastAttemptMs = 0;
    Position _destination;
};
class ShadowfangDecurseAction : public Action
{
public:
    ShadowfangDecurseAction(PlayerbotAI* botAI) : Action(botAI, "shadowfang decurse") {}
    bool isUseful() override;
    bool isPossible() override;
    bool Execute(Event event) override;
};
class ShadowfangProtectAction : public Action
{
public:
    ShadowfangProtectAction(PlayerbotAI* botAI) : Action(botAI, "shadowfang protect") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};
#endif
