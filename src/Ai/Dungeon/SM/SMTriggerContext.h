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
        creators["scarlet ground damage"] = &ground_damage;
        creators["scarlet protect dominated ally"] = &protect_ally;
        creators["herod whirlwind"] = &VanillaDungeonScarletMonasteryTriggerContext::herod_whirlwind;
        creators["thalnos fire nova"] = &thalnos_fire_nova;
        creators["ironspine poison cloud"] = &ironspine_poison_cloud;
        creators["scarlet focus healer"] = &scarlet_focus_healer;
        creators["scarlet dominated ally"] = &scarlet_dominated_ally;
    }

private:
    static Trigger* ground_damage(PlayerbotAI* botAI) { return new ScarletGroundDamageTrigger(botAI); }
    static Trigger* protect_ally(PlayerbotAI* botAI) { return new ScarletProtectAllyTrigger(botAI); }
    static Trigger* thalnos_fire_nova(PlayerbotAI* botAI)
    {
        return new ScarletHazardTrigger(botAI, "thalnos fire nova", SMShared::THALNOS);
    }
    static Trigger* ironspine_poison_cloud(PlayerbotAI* botAI)
    {
        return new ScarletHazardTrigger(botAI, "ironspine poison cloud", SMShared::IRONSPINE);
    }
    static Trigger* scarlet_focus_healer(PlayerbotAI* botAI) { return new ScarletFocusTrigger(botAI); }
    static Trigger* scarlet_dominated_ally(PlayerbotAI* botAI) { return new ScarletDominateTrigger(botAI); }

    static Trigger* herod_whirlwind(PlayerbotAI* botAI) { return new HerodWhirlwindTrigger(botAI); }

    static Trigger* arcanist_doan_boss_casting_detonation(PlayerbotAI* botAI)
    {
        return new ArcanistDoanBossCastingDetonationTrigger(botAI);
    }
};

#endif
