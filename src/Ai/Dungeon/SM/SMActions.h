/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SMACTIONS_H
#define PLAYERBOTS_SMACTIONS_H

#include "AttackAction.h"
#include "MovementActions.h"
#include "ObjectGuid.h"
#include "Position.h"
#include "SMShared.h"

class ScarletAvoidanceAction : public MovementAction
{
public:
    ScarletAvoidanceAction(PlayerbotAI* botAI, std::string const name, SMShared::Encounter encounter)
        : MovementAction(botAI, name), _encounter(encounter)
    {
    }
    bool isUseful() override;
    bool Execute(Event event) override;

private:
    SMShared::Encounter _encounter;
    ObjectGuid _escapeBoss;
    Position _escapeDestination;
    uint32 _lastAttemptMs = 0;
};

class ArcanistDoanMoveOutOfDetonationAction : public ScarletAvoidanceAction
{
public:
    ArcanistDoanMoveOutOfDetonationAction(PlayerbotAI* botAI)
        : ScarletAvoidanceAction(botAI, "arcanist doan move out of detonation", SMShared::DOAN)
    {
    }
};

class HerodWhirlwindEscapeAction : public ScarletAvoidanceAction
{
public:
    HerodWhirlwindEscapeAction(PlayerbotAI* botAI)
        : ScarletAvoidanceAction(botAI, "herod whirlwind escape", SMShared::HEROD)
    {
    }
};

class ThalnosFireNovaEscapeAction : public ScarletAvoidanceAction
{
public:
    ThalnosFireNovaEscapeAction(PlayerbotAI* botAI)
        : ScarletAvoidanceAction(botAI, "thalnos fire nova escape", SMShared::THALNOS)
    {
    }
};

class IronspinePoisonEscapeAction : public ScarletAvoidanceAction
{
public:
    IronspinePoisonEscapeAction(PlayerbotAI* botAI)
        : ScarletAvoidanceAction(botAI, "ironspine poison escape", SMShared::IRONSPINE)
    {
    }
};

class ScarletFocusAction : public AttackAction
{
public:
    ScarletFocusAction(PlayerbotAI* botAI) : AttackAction(botAI, "scarlet focus healer") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};

class ScarletDispelDominateAction : public Action
{
public:
    ScarletDispelDominateAction(PlayerbotAI* botAI) : Action(botAI, "scarlet dispel dominated ally") {}
    bool isUseful() override;
    bool isPossible() override;
    bool Execute(Event event) override;
};
class ScarletProtectAllyAction : public Action
{
public:
    ScarletProtectAllyAction(PlayerbotAI* botAI) : Action(botAI, "scarlet protect dominated ally") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};

class ScarletGroundEscapeAction : public MovementAction
{
public:
    ScarletGroundEscapeAction(PlayerbotAI* botAI) : MovementAction(botAI, "scarlet ground damage") {}
    std::string const getName() override { return "scarlet ground damage"; }
    bool isUseful() override;
    bool Execute(Event event) override;

private:
    Position _destination;
    uint32 _lastAttemptMs = 0;
};

#endif
