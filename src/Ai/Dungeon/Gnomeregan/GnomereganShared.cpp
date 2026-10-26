/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GnomereganShared.h"

#include "AttackersValue.h"
#include "Common/SpellReflect.h"
#include "Creature.h"
#include "GameObject.h"
#include "Group.h"
#include "MotionMaster.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "TargetValue.h"

namespace
{
bool CanRunForAssist(uint32 entry)
{
    switch (entry)
    {
        case 6206:  // Caverndeep Burrower
        case 6207:  // Caverndeep Ambusher
        case 6211:  // Caverndeep Reaver
        case 6391:  // Holdout Warrior
        case 6392:  // Holdout Medic
        case 6407:  // Holdout Technician
        case 9676:  // Tink Sprocketwhistle
            return true;
        default:
            return false;
    }
}
constexpr float MELEE_FOCUS_RANGE = 12.0f;

bool FocusAllowed(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    return sPlayerbotAIConfig.EnableGnomereganMechanics && bot->GetMapId() == Gnomeregan::MAP_ID && bot->IsInCombat() &&
           !PlayerbotAI::IsTank(bot) && !PlayerbotAI::IsHeal(bot);
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

uint32 FocusPriority(PlayerbotAI* botAI, Unit* target)
{
    if (!target || !target->IsCreature())
        return 0;
    if (target->GetEntry() == 7849)  // Mobile Alert System summons reinforcements.
        return 3;
    if (target->GetEntry() == 7915 && PlayerbotAI::IsRanged(botAI->GetBot()))
        return 2;
    return IsAssistRunner(target) ? 1 : 0;
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
        return bot->IsWithinCombatRange(target, range) && FocusPriority(botAI, target);
    }
    void CheckAttacker(Unit* target, ThreatManager* /*threatMgr*/) override
    {
        if (!Eligible(target))
            return;
        if (!result || FocusPriority(botAI, target) > FocusPriority(botAI, result) ||
            (FocusPriority(botAI, target) == FocusPriority(botAI, result) && target->GetGUID() < result->GetGUID()))
            result = target;
    }
};
}  // namespace

namespace Gnomeregan
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
    if (!FocusAllowed(botAI) || botAI->GetAiObjectContext()->GetValue<bool>("gnomeregan explicit target")->Get())
        return nullptr;
    Unit* target = botAI->GetUnit(botAI->GetAiObjectContext()->GetValue<ObjectGuid>("gnomeregan focus")->Get());
    if (!target || !target->IsCreature() || !target->IsAlive() || !target->IsInWorld() ||
        target->GetMap() != botAI->GetBot()->GetMap() || !target->IsInCombat() ||
        !botAI->GetBot()->IsValidAttackTarget(target) || !FocusPriority(botAI, target) ||
        target->HasBreakableByDamageCrowdControlAura() || !botAI->GetBot()->IsWithinLOSInMap(target))
        return nullptr;
    float const range = PlayerbotAI::IsRanged(botAI->GetBot()) ? botAI->GetRange("spell") : MELEE_FOCUS_RANGE;
    return botAI->GetBot()->IsWithinCombatRange(target, range) ? target : nullptr;
}

bool CanFocus(PlayerbotAI* botAI, Unit* target)
{
    return FocusAllowed(botAI) && !HasExplicitTarget(botAI) && FocusSelector(botAI).Eligible(target);
}
}  // namespace Gnomeregan

namespace Gnomeregan
{
bool Enabled(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    return sPlayerbotAIConfig.EnableGnomereganMechanics && bot->GetMapId() == MAP_ID && bot->IsAlive();
}

Hazards CollectHazards(PlayerbotAI* botAI)
{
    if (!Enabled(botAI))
        return {};
    Player* bot = botAI->GetBot();
    Hazards result;
    for (ObjectGuid const guid : botAI->GetAiObjectContext()->GetValue<GuidVector>("possible targets no los")->Get())
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsCreature() || !unit->IsAlive() || !unit->IsInWorld() || unit->GetMap() != bot->GetMap() ||
            !bot->IsWithinDistInMap(unit, 30.0f) || std::abs(unit->GetPositionZ() - bot->GetPositionZ()) > 5.0f)
            continue;
        uint32 spellId = 0;
        switch (unit->GetEntry())
        {
            case 8035:  // Avoid the mine throughout its ten-second arming period as well.
                spellId = 4043;
                break;
            case 7915:
                spellId = 11504;
                break;
            case 6227:
                if (unit->HasAura(10737))
                    spellId = 10734;
                break;
            default:
                break;
        }
        SpellInfo const* spell = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
        if (!spell)
            continue;
        float const radius = spell->Effects[EFFECT_0].CalcRadius(unit) + 3.0f;
        if (radius > 3.0f && bot->IsWithinLOSInMap(unit))
            result.push_back({unit->GetPosition(), radius});
    }
    return result;
}

bool InHazard(PlayerbotAI* botAI)
{
    if (!Enabled(botAI))
        return false;
    Position const position = botAI->GetBot()->GetPosition();
    for (Hazard const& area : botAI->GetAiObjectContext()->GetValue<Hazards>("gnomeregan hazards")->Get())
        if (area.center.GetExactDist2d(position.GetPositionX(), position.GetPositionY()) < area.radius)
            return true;
    return false;
}

bool BlockApproach(PlayerbotAI* botAI)
{
    if (!Enabled(botAI))
        return false;
    if (InHazard(botAI))
        return true;
    Unit* target = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    if (!target || !target->IsInWorld() || target->GetMap() != botAI->GetBot()->GetMap())
        return false;
    for (Hazard const& area : botAI->GetAiObjectContext()->GetValue<Hazards>("gnomeregan hazards")->Get())
        if (area.center.GetExactDist2d(target->GetPositionX(), target->GetPositionY()) < area.radius)
            return true;
    return false;
}

bool SafeProgress(Hazards const& hazards, Position const& from, Position const& to, bool requireProgress)
{
    bool progress = false;
    for (Hazard const& area : hazards)
    {
        float const before = area.center.GetExactDist2d(from.GetPositionX(), from.GetPositionY());
        float const after = area.center.GetExactDist2d(to.GetPositionX(), to.GetPositionY());
        if (before < area.radius)
        {
            float const outward =
                (from.GetPositionX() - area.center.GetPositionX()) * (to.GetPositionX() - from.GetPositionX()) +
                (from.GetPositionY() - area.center.GetPositionY()) * (to.GetPositionY() - from.GetPositionY());
            if (after + 0.01f < before || outward < -0.01f)
                return false;
            progress = progress || after > before + 0.1f;
        }
        else
        {
            float const dx = to.GetPositionX() - from.GetPositionX();
            float const dy = to.GetPositionY() - from.GetPositionY();
            float const length = dx * dx + dy * dy;
            float const projection = length > 0.0f
                                         ? std::clamp(((area.center.GetPositionX() - from.GetPositionX()) * dx +
                                                       (area.center.GetPositionY() - from.GetPositionY()) * dy) /
                                                          length,
                                                      0.0f, 1.0f)
                                         : 0.0f;
            if (area.center.GetExactDist2d(from.GetPositionX() + projection * dx,
                                           from.GetPositionY() + projection * dy) < area.radius)
                return false;
        }
    }
    return progress || !requireProgress;
}

namespace
{
uint32 FaceForButton(uint32 entry)
{
    switch (entry)
    {
        case 142214:
            return 142211;
        case 142215:
            return 142210;
        case 142216:
            return 142209;
        case 142217:
            return 142208;
        case 142218:
            return 142213;
        case 142219:
            return 142212;
        default:
            return 0;
    }
}
bool CanUseButton(PlayerbotAI* botAI)
{
    Player* bot = botAI->GetBot();
    return Enabled(botAI) && bot->IsInCombat() && !PlayerbotAI::IsTank(bot) && !PlayerbotAI::IsHeal(bot) &&
           botAI->CanMove();
}
}  // namespace

GameObject* GetButton(PlayerbotAI* botAI, ObjectGuid guid)
{
    if (!CanUseButton(botAI))
        return nullptr;
    Player* bot = botAI->GetBot();
    GameObject* button = botAI->GetGameObject(guid);
    if (!button || !button->IsInWorld() || !button->isSpawned() || button->GetMap() != bot->GetMap() ||
        !FaceForButton(button->GetEntry()) || !bot->IsWithinDistInMap(button, 40.0f) ||
        !bot->IsWithinLOSInMap(button) || button->GetGoState() != GO_STATE_READY)
        return nullptr;
    for (ObjectGuid const faceGuid :
         botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest game objects no los")->Get())
    {
        GameObject* face = botAI->GetGameObject(faceGuid);
        if (face && face->IsInWorld() && face->isSpawned() && face->GetMap() == bot->GetMap() &&
            face->GetEntry() == FaceForButton(button->GetEntry()) && face->GetGoState() == GO_STATE_ACTIVE)
            return button;
    }
    return nullptr;
}

ObjectGuid FindButton(PlayerbotAI* botAI)
{
    if (!CanUseButton(botAI))
        return ObjectGuid::Empty;
    Player* bot = botAI->GetBot();
    GameObject* selected = nullptr;
    for (ObjectGuid const guid :
         botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest game objects no los")->Get())
    {
        GameObject* button = GetButton(botAI, guid);
        if (!button)
            continue;
        bool assigned = true;
        if (Group* group = bot->GetGroup())
            for (GroupReference* member = group->GetFirstMember(); member; member = member->next())
            {
                Player* other = member->GetSource();
                PlayerbotAI* otherAI = other ? GET_PLAYERBOT_AI(other) : nullptr;
                if (!otherAI || other == bot || other->GetMap() != bot->GetMap() || !CanUseButton(otherAI) ||
                    !other->IsWithinLOSInMap(button))
                    continue;
                float const distance = other->GetExactDist(button);
                float const ours = bot->GetExactDist(button);
                if (distance < ours || (distance == ours && other->GetGUID() < bot->GetGUID()))
                    assigned = false;
            }
        if (assigned && (!selected || bot->GetExactDist(button) < bot->GetExactDist(selected)))
            selected = button;
    }
    return selected ? selected->GetGUID() : ObjectGuid::Empty;
}

bool HasReflect(PlayerbotAI* botAI)
{
    if (!Enabled(botAI) || !botAI->GetBot()->IsInCombat())
        return false;
    Unit* current = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    if (current && current->IsAlive() && current->IsInWorld() && current->GetMap() == botAI->GetBot()->GetMap() &&
        RaidClear::Reflect::HasReflect(current))
        return true;
    for (ObjectGuid const guid : botAI->GetAiObjectContext()->GetValue<GuidVector>("attackers")->Get())
        if (Unit* target = botAI->GetUnit(guid))
            if (target->IsAlive() && target->GetMap() == botAI->GetBot()->GetMap() &&
                RaidClear::Reflect::HasReflect(target))
                return true;
    return false;
}
}  // namespace Gnomeregan
