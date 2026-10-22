/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SMVALUECONTEXT_H
#define PLAYERBOTS_SMVALUECONTEXT_H

#include "NamedObjectContext.h"
#include "SMShared.h"
#include "Value.h"

class ScarletEncounterTargetsValue : public CalculatedValue<SMShared::EncounterTargets>
{
public:
    ScarletEncounterTargetsValue(PlayerbotAI* botAI)
        : CalculatedValue<SMShared::EncounterTargets>(botAI, "scarlet encounter targets", SMShared::TARGET_CACHE_MS)
    {
    }

protected:
    SMShared::EncounterTargets Calculate() override { return SMShared::CollectEncounterTargets(botAI); }
};

class ScarletFocusTargetValue : public ObjectGuidCalculatedValue
{
public:
    ScarletFocusTargetValue(PlayerbotAI* botAI) : ObjectGuidCalculatedValue(botAI, "scarlet focus target", 500) {}

protected:
    ObjectGuid Calculate() override { return SMShared::FindFocusTarget(botAI); }
};

class ScarletDominatedAllyValue : public ObjectGuidCalculatedValue
{
public:
    ScarletDominatedAllyValue(PlayerbotAI* botAI)
        : ObjectGuidCalculatedValue(botAI, "scarlet dominated ally", SMShared::TARGET_CACHE_MS)
    {
    }

protected:
    ObjectGuid Calculate() override { return SMShared::FindDominatedAlly(botAI); }
};

class ScarletCharmedPartyValue : public ObjectGuidListCalculatedValue
{
public:
    ScarletCharmedPartyValue(PlayerbotAI* botAI)
        : ObjectGuidListCalculatedValue(botAI, "scarlet charmed party", SMShared::TARGET_CACHE_MS)
    {
    }

protected:
    GuidVector Calculate() override { return SMShared::CharmedPartyMembers(botAI); }
};

class ScarletGroundDamageValue : public CalculatedValue<SMShared::GroundArea>
{
public:
    ScarletGroundDamageValue(PlayerbotAI* botAI)
        : CalculatedValue<SMShared::GroundArea>(botAI, "scarlet ground damage", SMShared::TARGET_CACHE_MS)
    {
    }

protected:
    SMShared::GroundArea Calculate() override { return SMShared::CollectGroundArea(botAI); }
};

class VanillaDungeonScarletMonasteryValueContext : public NamedObjectContext<UntypedValue>
{
public:
    VanillaDungeonScarletMonasteryValueContext()
    {
        creators["scarlet ground damage"] = &ground_damage;
        creators["scarlet charmed party"] = &charmed_party;
        creators["scarlet encounter targets"] = &encounter_targets;
        creators["scarlet focus target"] = &focus_target;
        creators["scarlet dominated ally"] = &dominated_ally;
    }

private:
    static UntypedValue* ground_damage(PlayerbotAI* botAI) { return new ScarletGroundDamageValue(botAI); }
    static UntypedValue* charmed_party(PlayerbotAI* botAI) { return new ScarletCharmedPartyValue(botAI); }
    static UntypedValue* encounter_targets(PlayerbotAI* botAI) { return new ScarletEncounterTargetsValue(botAI); }
    static UntypedValue* focus_target(PlayerbotAI* botAI) { return new ScarletFocusTargetValue(botAI); }
    static UntypedValue* dominated_ally(PlayerbotAI* botAI) { return new ScarletDominatedAllyValue(botAI); }
};
#endif
