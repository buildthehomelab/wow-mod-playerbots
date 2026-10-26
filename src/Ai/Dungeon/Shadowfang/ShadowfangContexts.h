/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SHADOWFANGCONTEXTS_H
#define PLAYERBOTS_SHADOWFANGCONTEXTS_H

#include "NamedObjectContext.h"
#include "ShadowfangActions.h"
#include "ShadowfangShared.h"
#include "Trigger.h"
#include "Value.h"

class ShadowfangHazardValue : public CalculatedValue<Shadowfang::Hazard>
{
public:
    ShadowfangHazardValue(PlayerbotAI* botAI)
        : CalculatedValue<Shadowfang::Hazard>(botAI, "shadowfang hazard", Shadowfang::CACHE_MS)
    {
    }

protected:
    Shadowfang::Hazard Calculate() override { return Shadowfang::CollectHazard(botAI); }
};
class ShadowfangCursedPartyValue : public CalculatedValue<GuidVector>
{
public:
    ShadowfangCursedPartyValue(PlayerbotAI* botAI) : CalculatedValue<GuidVector>(botAI, "shadowfang cursed party", 250)
    {
    }

protected:
    GuidVector Calculate() override { return Shadowfang::CursedParty(botAI); }
};
class ShadowfangDecurseTargetValue : public ObjectGuidCalculatedValue
{
public:
    ShadowfangDecurseTargetValue(PlayerbotAI* botAI)
        : ObjectGuidCalculatedValue(botAI, "shadowfang decurse target", 250)
    {
    }

protected:
    ObjectGuid Calculate() override { return Shadowfang::FindDecurseTarget(botAI); }
};
class ShadowfangValueContext : public NamedObjectContext<UntypedValue>
{
public:
    ShadowfangValueContext()
    {
        creators["shadowfang hazard"] = &hazard;
        creators["shadowfang cursed party"] = &party;
        creators["shadowfang decurse target"] = &target;
    }

private:
    static UntypedValue* hazard(PlayerbotAI* botAI) { return new ShadowfangHazardValue(botAI); }
    static UntypedValue* target(PlayerbotAI* botAI) { return new ShadowfangDecurseTargetValue(botAI); }
    static UntypedValue* party(PlayerbotAI* botAI) { return new ShadowfangCursedPartyValue(botAI); }
};
class ShadowfangHazardTrigger : public Trigger
{
public:
    ShadowfangHazardTrigger(PlayerbotAI* botAI) : Trigger(botAI, "shadowfang hazard") {}
    bool IsActive() override { return Shadowfang::InHazard(botAI); }
};
class ShadowfangDecurseTrigger : public Trigger
{
public:
    ShadowfangDecurseTrigger(PlayerbotAI* botAI) : Trigger(botAI, "shadowfang decurse") {}
    bool IsActive() override { return Shadowfang::DecurseTarget(botAI) != nullptr; }
};
class ShadowfangProtectTrigger : public Trigger
{
public:
    ShadowfangProtectTrigger(PlayerbotAI* botAI) : Trigger(botAI, "shadowfang protect") {}
    bool IsActive() override { return Shadowfang::NeedsProtection(botAI); }
};
class ShadowfangTriggerContext : public NamedObjectContext<Trigger>
{
public:
    ShadowfangTriggerContext()
    {
        creators["shadowfang hazard"] = &hazard;
        creators["shadowfang protect"] = &protect;
        creators["shadowfang decurse"] = &decurse;
    }

private:
    static Trigger* protect(PlayerbotAI* botAI) { return new ShadowfangProtectTrigger(botAI); }
    static Trigger* hazard(PlayerbotAI* botAI) { return new ShadowfangHazardTrigger(botAI); }
    static Trigger* decurse(PlayerbotAI* botAI) { return new ShadowfangDecurseTrigger(botAI); }
};
class ShadowfangActionContext : public NamedObjectContext<Action>
{
public:
    ShadowfangActionContext()
    {
        creators["shadowfang escape"] = &escape;
        creators["shadowfang protect"] = &protect;
        creators["shadowfang decurse"] = &decurse;
    }

private:
    static Action* escape(PlayerbotAI* botAI) { return new ShadowfangEscapeAction(botAI); }
    static Action* decurse(PlayerbotAI* botAI) { return new ShadowfangDecurseAction(botAI); }
    static Action* protect(PlayerbotAI* botAI) { return new ShadowfangProtectAction(botAI); }
};
#endif
