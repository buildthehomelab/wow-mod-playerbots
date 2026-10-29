/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "WailingCavernsShared.h"

#include "Creature.h"
#include "Group.h"
#include "MotionMaster.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "TargetValue.h"

namespace
{
constexpr uint32 NPC_SKUM = 3674;
constexpr uint32 NPC_DRUID_OF_THE_FANG = 3840;
constexpr float MELEE_FOCUS_RANGE = 12.0f;

bool FocusAllowed(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    if (!sPlayerbotAIConfig.EnableWailingCavernsMechanics || bot->GetMapId() != WailingCaverns::MAP_ID ||
        !bot->IsInCombat() || PlayerbotAI::IsTank(bot) || PlayerbotAI::IsHeal(bot) ||
        !botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets")->Get().empty())
        return false;
    Group* group = bot->GetGroup();
    return !group || !group->GetTargetIcon(7);
}

bool IsAssistRunner(Unit* target)
{
    if (!target || !target->IsCreature())
        return false;
    if (target->GetEntry() != NPC_SKUM && target->GetEntry() != NPC_DRUID_OF_THE_FANG)
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

namespace WailingCaverns
{
ObjectGuid FindFocus(PlayerbotAI* botAI)
{
    if (!FocusAllowed(botAI))
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
    if (!FocusAllowed(botAI))
        return nullptr;
    Unit* target = botAI->GetUnit(botAI->GetAiObjectContext()->GetValue<ObjectGuid>("wailing caverns focus")->Get());
    if (!target || !target->IsCreature() || !target->IsAlive() || !target->IsInWorld() ||
        target->GetMap() != botAI->GetBot()->GetMap() || !target->IsInCombat() ||
        !botAI->GetBot()->IsValidAttackTarget(target) || !IsAssistRunner(target) ||
        target->HasBreakableByDamageCrowdControlAura() || !botAI->GetBot()->IsWithinLOSInMap(target))
        return nullptr;
    float const range = PlayerbotAI::IsRanged(botAI->GetBot()) ? botAI->GetRange("spell") : MELEE_FOCUS_RANGE;
    return botAI->GetBot()->IsWithinCombatRange(target, range) ? target : nullptr;
}

bool CanFocus(PlayerbotAI* botAI, Unit* target) { return FocusAllowed(botAI) && FocusSelector(botAI).Eligible(target); }
}  // namespace WailingCaverns
