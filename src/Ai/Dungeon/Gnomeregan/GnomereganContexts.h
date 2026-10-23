/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_GNOMEREGANCONTEXTS_H
#define PLAYERBOTS_GNOMEREGANCONTEXTS_H

#include "GnomereganActions.h"
#include "GnomereganShared.h"
#include "NamedObjectContext.h"
#include "Trigger.h"
#include "Value.h"

class GnomereganFocusValue : public ObjectGuidCalculatedValue
{
public:
    GnomereganFocusValue(PlayerbotAI* botAI) : ObjectGuidCalculatedValue(botAI, "gnomeregan focus", 500) {}

protected:
    ObjectGuid Calculate() override { return Gnomeregan::FindFocus(botAI); }
};
class GnomereganExplicitTargetValue : public BoolCalculatedValue
{
public:
    GnomereganExplicitTargetValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "gnomeregan explicit target", 500) {}

protected:
    bool Calculate() override { return Gnomeregan::HasExplicitTarget(botAI); }
};
class GnomereganHazardsValue : public CalculatedValue<Gnomeregan::Hazards>
{
public:
    GnomereganHazardsValue(PlayerbotAI* botAI) : CalculatedValue(botAI, "gnomeregan hazards", 250) {}

protected:
    Gnomeregan::Hazards Calculate() override { return Gnomeregan::CollectHazards(botAI); }
};
class GnomereganHazardValue : public BoolCalculatedValue
{
public:
    GnomereganHazardValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "gnomeregan hazard", 250) {}

protected:
    bool Calculate() override { return Gnomeregan::InHazard(botAI); }
};
class GnomereganApproachValue : public BoolCalculatedValue
{
public:
    GnomereganApproachValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "gnomeregan block approach", 250) {}

protected:
    bool Calculate() override { return Gnomeregan::BlockApproach(botAI); }
};
class GnomereganButtonValue : public ObjectGuidCalculatedValue
{
public:
    GnomereganButtonValue(PlayerbotAI* botAI) : ObjectGuidCalculatedValue(botAI, "gnomeregan button", 500) {}

protected:
    ObjectGuid Calculate() override { return Gnomeregan::FindButton(botAI); }
};
class GnomereganReflectValue : public BoolCalculatedValue
{
public:
    GnomereganReflectValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "gnomeregan reflect", 500) {}

protected:
    bool Calculate() override { return Gnomeregan::HasReflect(botAI); }
};
class GnomereganValueContext : public NamedObjectContext<UntypedValue>
{
public:
    GnomereganValueContext()
    {
        creators["gnomeregan focus"] = &focus;
        creators["gnomeregan hazards"] = &hazards;
        creators["gnomeregan hazard"] = &hazard;
        creators["gnomeregan block approach"] = &approach;
        creators["gnomeregan button"] = &button;
        creators["gnomeregan reflect"] = &reflect;
        creators["gnomeregan explicit target"] = &explicitTarget;
    }

private:
    static UntypedValue* approach(PlayerbotAI* botAI) { return new GnomereganApproachValue(botAI); }
    static UntypedValue* hazards(PlayerbotAI* botAI) { return new GnomereganHazardsValue(botAI); }
    static UntypedValue* hazard(PlayerbotAI* botAI) { return new GnomereganHazardValue(botAI); }
    static UntypedValue* button(PlayerbotAI* botAI) { return new GnomereganButtonValue(botAI); }
    static UntypedValue* reflect(PlayerbotAI* botAI) { return new GnomereganReflectValue(botAI); }
    static UntypedValue* focus(PlayerbotAI* botAI) { return new GnomereganFocusValue(botAI); }
    static UntypedValue* explicitTarget(PlayerbotAI* botAI) { return new GnomereganExplicitTargetValue(botAI); }
};
class GnomereganFocusTrigger : public Trigger
{
public:
    GnomereganFocusTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gnomeregan focus") {}
    bool IsActive() override { return Gnomeregan::GetFocus(botAI) != nullptr; }
};
class GnomereganHazardTrigger : public Trigger
{
public:
    GnomereganHazardTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gnomeregan hazard") {}
    bool IsActive() override
    {
        return Gnomeregan::Enabled(botAI) && context->GetValue<bool>("gnomeregan hazard")->Get();
    }
};
class GnomereganButtonTrigger : public Trigger
{
public:
    GnomereganButtonTrigger(PlayerbotAI* botAI) : Trigger(botAI, "gnomeregan button") {}
    bool IsActive() override
    {
        return Gnomeregan::Enabled(botAI) && bool(context->GetValue<ObjectGuid>("gnomeregan button")->Get());
    }
};
class GnomereganTriggerContext : public NamedObjectContext<Trigger>
{
public:
    GnomereganTriggerContext()
    {
        creators["gnomeregan focus"] = &focus;
        creators["gnomeregan hazard"] = &hazard;
        creators["gnomeregan button"] = &button;
    }

private:
    static Trigger* hazard(PlayerbotAI* botAI) { return new GnomereganHazardTrigger(botAI); }
    static Trigger* button(PlayerbotAI* botAI) { return new GnomereganButtonTrigger(botAI); }
    static Trigger* focus(PlayerbotAI* botAI) { return new GnomereganFocusTrigger(botAI); }
};
class GnomereganActionContext : public NamedObjectContext<Action>
{
public:
    GnomereganActionContext()
    {
        creators["gnomeregan focus"] = &focus;
        creators["gnomeregan escape"] = &escape;
        creators["gnomeregan button"] = &button;
    }

private:
    static Action* escape(PlayerbotAI* botAI) { return new GnomereganEscapeAction(botAI); }
    static Action* button(PlayerbotAI* botAI) { return new GnomereganButtonAction(botAI); }
    static Action* focus(PlayerbotAI* botAI) { return new GnomereganFocusAction(botAI); }
};
#endif
