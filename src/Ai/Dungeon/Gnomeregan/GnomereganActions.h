/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GNOMEREGANACTIONS_H
#define PLAYERBOTS_GNOMEREGANACTIONS_H

#include "AttackAction.h"

class GnomereganFocusAction : public AttackAction
{
public:
    GnomereganFocusAction(PlayerbotAI* botAI) : AttackAction(botAI, "gnomeregan focus") {}
    bool isUseful() override;
    bool Execute(Event event) override;

private:
    Unit* ValidatedTarget();
};
class GnomereganEscapeAction : public MovementAction
{
public:
    GnomereganEscapeAction(PlayerbotAI* botAI) : MovementAction(botAI, "gnomeregan escape") {}
    bool isUseful() override;
    bool Execute(Event event) override;

private:
    uint32 _lastAttemptMs = 0;
    Position _destination;
};
class GnomereganButtonAction : public MovementAction
{
public:
    GnomereganButtonAction(PlayerbotAI* botAI) : MovementAction(botAI, "gnomeregan button") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};
#endif
