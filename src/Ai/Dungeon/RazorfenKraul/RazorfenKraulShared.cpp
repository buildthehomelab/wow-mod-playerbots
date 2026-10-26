/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RazorfenKraulShared.h"

#include "AttackersValue.h"
#include "Creature.h"
#include "Group.h"
#include "MotionMaster.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "TargetValue.h"

namespace
{
bool CanRunForAssist(uint32 entry)
{
    switch (entry)
    {
        case 4427:  // Ward Guardian
        case 4435:  // Razorfen Warrior
        case 4436:  // Razorfen Quilguard
        case 4437:  // Razorfen Warden
        case 4438:  // Razorfen Spearhide
        case 4440:  // Razorfen Totemic
        case 4442:  // Razorfen Defender
        case 4515:  // Death's Head Acolyte
        case 4516:  // Death's Head Adept
        case 4517:  // Death's Head Priest
        case 4518:  // Death's Head Sage
        case 4519:  // Death's Head Seer
        case 4520:  // Razorfen Geomancer
        case 4522:  // Razorfen Dustweaver
        case 4523:  // Razorfen Groundshaker
        case 4525:  // Razorfen Earthbreaker
        case 4530:  // Razorfen Handler
        case 4531:  // Razorfen Beast Trainer
        case 4532:  // Razorfen Beastmaster
        case 4623:  // Quilguard Champion
        case 4842:  // Earthcaller Halmgar
        case 6035:  // Razorfen Stalker
            return true;
        default:
            return false;
    }
}
constexpr float MELEE_FOCUS_RANGE = 12.0f;

bool FocusAllowed(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    return sPlayerbotAIConfig.EnableRazorfenKraulMechanics && bot->GetMapId() == RazorfenKraul::MAP_ID &&
           bot->IsInCombat() && !PlayerbotAI::IsTank(bot) && !PlayerbotAI::IsHeal(bot);
}

bool IsAssistRunner(Unit* target)
{
    if (!target || !target->IsCreature())
        return false;
    if (!CanRunForAssist(target->GetEntry()))
        return false;
    MovementGeneratorType const movement = target->GetMotionMaster()->GetCurrentMovementGeneratorType();
    // Seeking a nearby assistant is passive movement, not the fear-style fallback.
    if (movement == ASSISTANCE_MOTION_TYPE)
        return true;
    return target->ToCreature()->GetReactState() != REACT_PASSIVE &&
           (movement == FLEEING_MOTION_TYPE || movement == TIMED_FLEEING_MOTION_TYPE);
}

bool IsEngagedTotem(PlayerbotAI* botAI, Unit* target)
{
    if (!target)
        return false;
    switch (target->GetEntry())
    {
        case 2630:  // Earthbind Totem
        case 2992:  // Healing Ward V
        case 6016:  // Elemental Protection Totem
        case 6017:  // Lava Spout Totem
        case 6066:  // Earthgrab Totem
            break;
        default:
            return false;
    }
    Unit* owner = botAI->GetUnit(target->GetOwnerGUID());
    Player* bot = botAI->GetBot();
    if (!owner || !owner->IsAlive() || !owner->IsInWorld() || owner->GetMap() != bot->GetMap() ||
        !owner->IsInCombat() || !bot->IsValidAttackTarget(owner))
        return false;
    Unit* victim = owner->GetVictim();
    Player* player = victim ? victim->GetCharmerOrOwnerPlayerOrPlayerItself() : nullptr;
    return player && (player == bot || (bot->GetGroup() && bot->GetGroup()->IsMember(player->GetGUID())));
}

class FocusSelector : public FindNonCcTargetStrategy
{
public:
    FocusSelector(PlayerbotAI* botAI) : FindNonCcTargetStrategy(botAI) {}
    bool Eligible(Unit* target)
    {
        Player* bot = botAI->GetBot();
        if (!target || !target->IsCreature() || !target->IsAlive() || !target->IsInWorld() ||
            target->GetMap() != bot->GetMap() || (!target->IsInCombat() && !IsEngagedTotem(botAI, target)) ||
            !bot->IsValidAttackTarget(target) || target->HasBreakableByDamageCrowdControlAura() || IsCcTarget(target) ||
            !bot->IsWithinLOSInMap(target))
            return false;
        float const range = PlayerbotAI::IsRanged(bot) ? botAI->GetRange("spell") : MELEE_FOCUS_RANGE;
        return bot->IsWithinCombatRange(target, range) && (IsAssistRunner(target) || IsEngagedTotem(botAI, target));
    }
    void CheckAttacker(Unit* target, ThreatManager* /*threatMgr*/) override
    {
        if (!Eligible(target))
            return;
        if (!result || target->GetGUID() < result->GetGUID())
            result = target;
    }
};
}  // namespace

namespace RazorfenKraul
{
bool HasExplicitTarget(PlayerbotAI* botAI)
{
    if (!FocusAllowed(botAI))
        return false;
    Player* bot = botAI->GetBot();
    auto valid = [botAI, bot](ObjectGuid guid)
    {
        Unit* target = botAI->GetUnit(guid);
        return target && AttackersValue::IsValidTarget(target, bot);
    };
    if (Group* group = bot->GetGroup())
        if (valid(group->GetTargetIcon(7)))
            return true;
    GuidVector const priorities = botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets")->Get();
    for (ObjectGuid const guid : priorities)
        if (valid(guid))
            return true;
    return false;
}

ObjectGuid FindFocus(PlayerbotAI* botAI)
{
    if (!FocusAllowed(botAI) || HasExplicitTarget(botAI))
        return ObjectGuid::Empty;
    FocusSelector selector(botAI);
    GuidVector const attackers = botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get();
    for (ObjectGuid const guid : attackers)
        selector.CheckAttacker(botAI->GetUnit(guid), nullptr);
    if (!selector.GetResult())
    {
        GuidVector const possible = botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets")->Get();
        for (ObjectGuid const guid : possible)
        {
            Unit* candidate = botAI->GetUnit(guid);
            if (IsEngagedTotem(botAI, candidate))
                selector.CheckAttacker(candidate, nullptr);
        }
    }
    Unit* target = selector.GetResult();
    return target ? target->GetGUID() : ObjectGuid::Empty;
}

Unit* GetFocus(PlayerbotAI* botAI)
{
    if (!FocusAllowed(botAI) || botAI->GetAiObjectContext()->GetValue<bool>("razorfen kraul explicit target")->Get())
        return nullptr;
    Unit* target = botAI->GetUnit(botAI->GetAiObjectContext()->GetValue<ObjectGuid>("razorfen kraul focus")->Get());
    if (!target || !target->IsCreature() || !target->IsAlive() || !target->IsInWorld() ||
        target->GetMap() != botAI->GetBot()->GetMap() || (!target->IsInCombat() && !IsEngagedTotem(botAI, target)) ||
        !botAI->GetBot()->IsValidAttackTarget(target) || (!IsAssistRunner(target) && !IsEngagedTotem(botAI, target)) ||
        target->HasBreakableByDamageCrowdControlAura() || !botAI->GetBot()->IsWithinLOSInMap(target))
        return nullptr;
    float const range = PlayerbotAI::IsRanged(botAI->GetBot()) ? botAI->GetRange("spell") : MELEE_FOCUS_RANGE;
    return botAI->GetBot()->IsWithinCombatRange(target, range) ? target : nullptr;
}

bool CanFocus(PlayerbotAI* botAI, Unit* target)
{
    return FocusAllowed(botAI) && !HasExplicitTarget(botAI) && FocusSelector(botAI).Eligible(target);
}
}  // namespace RazorfenKraul
