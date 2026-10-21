/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SMTRIGGERS_H
#define PLAYERBOTS_SMTRIGGERS_H

#include "SMShared.h"
#include "Trigger.h"

class ScarletHazardTrigger : public Trigger
{
public:
    ScarletHazardTrigger(PlayerbotAI* botAI, std::string const name, SMShared::Encounter encounter)
        : Trigger(botAI, name), _encounter(encounter)
    {
    }
    bool IsActive() override;

private:
    SMShared::Encounter _encounter;
};

class ArcanistDoanBossCastingDetonationTrigger : public ScarletHazardTrigger
{
public:
    ArcanistDoanBossCastingDetonationTrigger(PlayerbotAI* botAI)
        : ScarletHazardTrigger(botAI, "arcanist doan boss casting detonation", SMShared::DOAN)
    {
    }
};
class HerodWhirlwindTrigger : public ScarletHazardTrigger
{
public:
    HerodWhirlwindTrigger(PlayerbotAI* botAI) : ScarletHazardTrigger(botAI, "herod whirlwind", SMShared::HEROD) {}
};
class ScarletFocusTrigger : public Trigger
{
public:
    ScarletFocusTrigger(PlayerbotAI* botAI) : Trigger(botAI, "scarlet focus healer") {}
    bool IsActive() override;
};
class ScarletDominateTrigger : public Trigger
{
public:
    ScarletDominateTrigger(PlayerbotAI* botAI) : Trigger(botAI, "scarlet dominated ally") {}
    bool IsActive() override;
};
class ScarletProtectAllyTrigger : public Trigger
{
public:
    ScarletProtectAllyTrigger(PlayerbotAI* botAI) : Trigger(botAI, "scarlet protect dominated ally") {}
    bool IsActive() override;
};

class ScarletGroundDamageTrigger : public Trigger
{
public:
    ScarletGroundDamageTrigger(PlayerbotAI* botAI) : Trigger(botAI, "scarlet ground damage") {}
    bool IsActive() override;
};

#endif
