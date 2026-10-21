/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SMTRIGGERCONTEXT_H
#define PLAYERBOTS_SMTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "SMTriggers.h"

class VanillaDungeonScarletMonasteryTriggerContext : public NamedObjectContext<Trigger>
{
public:
    VanillaDungeonScarletMonasteryTriggerContext()
    {
        creators["arcanist doan boss casting detonation"] =
            &VanillaDungeonScarletMonasteryTriggerContext::arcanist_doan_boss_casting_detonation;
        creators["herod whirlwind"] = &VanillaDungeonScarletMonasteryTriggerContext::herod_whirlwind;
    }

private:
    static Trigger* herod_whirlwind(PlayerbotAI* botAI) { return new HerodWhirlwindTrigger(botAI); }

    static Trigger* arcanist_doan_boss_casting_detonation(PlayerbotAI* botAI)
    {
        return new ArcanistDoanBossCastingDetonationTrigger(botAI);
    }
};

#endif
