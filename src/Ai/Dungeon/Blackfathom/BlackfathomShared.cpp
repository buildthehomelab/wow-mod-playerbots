/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BlackfathomShared.h"

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
        case 4805:  // Blackfathom Sea Witch
        case 4807:  // Blackfathom Myrmidon
        case 4809:  // Twilight Acolyte
        case 4810:  // Twilight Reaver
        case 4811:  // Twilight Aquamancer
        case 4812:  // Twilight Loreseeker
        case 4813:  // Twilight Shadowmage
        case 4814:  // Twilight Elementalist
        case 4815:  // Murkshallow Snapclaw
        case 4818:  // Blindlight Murloc
        case 4819:  // Blindlight Muckdweller
        case 4820:  // Blindlight Oracle
        case 4821:  // Skittering Crustacean
        case 4823:  // Barbed Crustacean
            return true;
        default:
            return false;
    }
}
constexpr float MELEE_FOCUS_RANGE = 12.0f;

bool FocusAllowed(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    return sPlayerbotAIConfig.EnableBlackfathomMechanics && bot->GetMapId() == Blackfathom::MAP_ID &&
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

class FocusSelector : public FindNonCcTargetStrategy
{
public:
    FocusSelector(PlayerbotAI* botAI) : FindNonCcTargetStrategy(botAI) {}
    bool Eligible(Unit* target)
    {
        Player* bot = botAI->GetBot();
        if (!target || !target->IsCreature() || !target->IsAlive() || !target->IsInWorld() ||
            target->GetMap() != bot->GetMap() || !target->IsInCombat() || !bot->IsValidAttackTarget(target) ||
            target->HasBreakableByDamageCrowdControlAura() || IsCcTarget(target) || !bot->IsWithinLOSInMap(target))
            return false;
        float const range = PlayerbotAI::IsRanged(bot) ? botAI->GetRange("spell") : MELEE_FOCUS_RANGE;
        return bot->IsWithinCombatRange(target, range) && IsAssistRunner(target);
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

namespace Blackfathom
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
    Unit* target = selector.GetResult();
    return target ? target->GetGUID() : ObjectGuid::Empty;
}

Unit* GetFocus(PlayerbotAI* botAI)
{
    if (!FocusAllowed(botAI) || botAI->GetAiObjectContext()->GetValue<bool>("blackfathom explicit target")->Get())
        return nullptr;
    Unit* target = botAI->GetUnit(botAI->GetAiObjectContext()->GetValue<ObjectGuid>("blackfathom focus")->Get());
    if (!target || !target->IsCreature() || !target->IsAlive() || !target->IsInWorld() ||
        target->GetMap() != botAI->GetBot()->GetMap() || !target->IsInCombat() ||
        !botAI->GetBot()->IsValidAttackTarget(target) || !IsAssistRunner(target) ||
        target->HasBreakableByDamageCrowdControlAura() || !botAI->GetBot()->IsWithinLOSInMap(target))
        return nullptr;
    float const range = PlayerbotAI::IsRanged(botAI->GetBot()) ? botAI->GetRange("spell") : MELEE_FOCUS_RANGE;
    return botAI->GetBot()->IsWithinCombatRange(target, range) ? target : nullptr;
}

bool CanFocus(PlayerbotAI* botAI, Unit* target)
{
    return FocusAllowed(botAI) && !HasExplicitTarget(botAI) && FocusSelector(botAI).Eligible(target);
}
}  // namespace Blackfathom
