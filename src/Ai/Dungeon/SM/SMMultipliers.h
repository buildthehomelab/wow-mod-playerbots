/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SMMULTIPLIERS_H
#define PLAYERBOTS_SMMULTIPLIERS_H

#include "Multiplier.h"
#include "SMShared.h"

class ScarletHazardMultiplier : public Multiplier
{
public:
    ScarletHazardMultiplier(PlayerbotAI* botAI, std::string const name, SMShared::Encounter encounter)
        : Multiplier(botAI, name), _encounter(encounter)
    {
    }
    float GetValue(Action* action) override;

private:
    SMShared::Encounter _encounter;
};
class ArcanistDoanDetonationMultiplier : public ScarletHazardMultiplier
{
public:
    ArcanistDoanDetonationMultiplier(PlayerbotAI* botAI)
        : ScarletHazardMultiplier(botAI, "arcanist doan detonation", SMShared::DOAN)
    {
    }
};
class HerodWhirlwindMultiplier : public ScarletHazardMultiplier
{
public:
    HerodWhirlwindMultiplier(PlayerbotAI* botAI) : ScarletHazardMultiplier(botAI, "herod whirlwind", SMShared::HEROD) {}
};
class ScarletFocusMultiplier : public Multiplier
{
public:
    ScarletFocusMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "scarlet focus healer") {}
    float GetValue(Action* action) override;
};
class ScarletCharmSafetyMultiplier : public Multiplier
{
public:
    ScarletCharmSafetyMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "scarlet charm safety") {}
    float GetValue(Action* action) override;
};

class ScarletGroundDamageMultiplier : public Multiplier
{
public:
    ScarletGroundDamageMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "scarlet ground damage") {}
    float GetValue(Action* action) override;
};

#endif
