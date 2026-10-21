/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SMACTIONCONTEXT_H
#define PLAYERBOTS_SMACTIONCONTEXT_H

#include "NamedObjectContext.h"
#include "SMActions.h"

class VanillaDungeonScarletMonasteryActionContext : public NamedObjectContext<Action>
{
public:
    VanillaDungeonScarletMonasteryActionContext()
    {
        creators["arcanist doan move out of detonation"] =
            &VanillaDungeonScarletMonasteryActionContext::arcanist_doan_move_out_of_detonation;
        creators["scarlet ground damage"] = &ground_damage;
        creators["scarlet protect dominated ally"] = &protect_ally;
        creators["herod whirlwind escape"] = &VanillaDungeonScarletMonasteryActionContext::herod_whirlwind_escape;
        creators["thalnos fire nova escape"] = &thalnos_fire_nova_escape;
        creators["ironspine poison escape"] = &ironspine_poison_escape;
        creators["scarlet focus healer"] = &scarlet_focus_healer;
        creators["scarlet dispel dominated ally"] = &scarlet_dispel_dominated_ally;
    }

private:
    static Action* ground_damage(PlayerbotAI* botAI) { return new ScarletGroundEscapeAction(botAI); }
    static Action* protect_ally(PlayerbotAI* botAI) { return new ScarletProtectAllyAction(botAI); }
    static Action* thalnos_fire_nova_escape(PlayerbotAI* botAI) { return new ThalnosFireNovaEscapeAction(botAI); }
    static Action* ironspine_poison_escape(PlayerbotAI* botAI) { return new IronspinePoisonEscapeAction(botAI); }
    static Action* scarlet_focus_healer(PlayerbotAI* botAI) { return new ScarletFocusAction(botAI); }
    static Action* scarlet_dispel_dominated_ally(PlayerbotAI* botAI) { return new ScarletDispelDominateAction(botAI); }

    static Action* herod_whirlwind_escape(PlayerbotAI* botAI) { return new HerodWhirlwindEscapeAction(botAI); }

    static Action* arcanist_doan_move_out_of_detonation(PlayerbotAI* botAI)
    {
        return new ArcanistDoanMoveOutOfDetonationAction(botAI);
    }
};

#endif
