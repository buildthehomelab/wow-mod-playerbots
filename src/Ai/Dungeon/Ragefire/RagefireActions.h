/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_RAGEFIREACTIONS_H
#define PLAYERBOTS_RAGEFIREACTIONS_H

#include "AttackAction.h"
#include "MovementActions.h"

class RagefireFocusAction : public AttackAction
{
public:
    RagefireFocusAction(PlayerbotAI* botAI) : AttackAction(botAI, "ragefire focus") {}
    bool isUseful() override;
    bool Execute(Event event) override;

private:
    Unit* ValidatedTarget();
};
class RagefireEscapeAction : public MovementAction
{
public:
    RagefireEscapeAction(PlayerbotAI* botAI) : MovementAction(botAI, "ragefire escape") {}
    bool isUseful() override;
    bool Execute(Event event) override;

private:
    uint32 _lastAttemptMs = 0;
    Position _destination;
};
#endif
