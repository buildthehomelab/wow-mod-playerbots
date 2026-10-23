/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Minimal engine doubles for executing the real Blackfathom sources; not a game simulation.
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>
using uint8 = uint8_t;
using uint32 = uint32_t;
struct ObjectGuid
{
    unsigned id = 0;
    ObjectGuid(unsigned value = 0) : id(value) {}
    explicit operator bool() const { return id != 0; }
    auto operator<=>(ObjectGuid const&) const = default;
    static ObjectGuid const Empty;
};
ObjectGuid const ObjectGuid::Empty{};
using GuidVector = std::vector<ObjectGuid>;
using GuidSet = std::set<ObjectGuid>;
struct Position
{
    float x = 0, y = 0, z = 0;
    void Relocate(float a, float b, float c)
    {
        x = a;
        y = b;
        z = c;
    }
    float GetPositionX() const { return x; }
    float GetPositionY() const { return y; }
    float GetPositionZ() const { return z; }
    float GetExactDist2d(float a, float b) const { return std::hypot(x - a, y - b); }
    float GetExactDist(float a, float b, float c) const { return std::hypot(std::hypot(x - a, y - b), z - c); }
};
class Player;
struct Creature;
struct Group;
struct ThreatManager
{
};
[[maybe_unused]] constexpr int REACT_PASSIVE = 0, REACT_AGGRESSIVE = 2;
enum MovementGeneratorType
{
    IDLE_MOTION_TYPE,
    ASSISTANCE_MOTION_TYPE,
    FLEEING_MOTION_TYPE,
    TIMED_FLEEING_MOTION_TYPE
};
struct MotionMaster
{
    MovementGeneratorType type = IDLE_MOTION_TYPE;
    MovementGeneratorType GetCurrentMovementGeneratorType() { return type; }
};
class Unit : public Position
{
public:
    ObjectGuid guid;
    uint32 entry = 0, map = 48;
    bool alive = true, combat = true, creature = false, valid = true, world = true, los = true, cc = false,
         markedCc = false;
    int reaction = REACT_AGGRESSIVE;
    MotionMaster motion;
    explicit Unit(unsigned id = 1) : guid(id) {}
    virtual ~Unit() = default;
    bool IsAlive() { return alive; }
    bool IsInWorld() { return world; }
    uint32 GetMap() { return map; }
    uint32 GetMapId() { return map; }
    uint32 GetEntry() { return entry; }
    ObjectGuid GetGUID() { return guid; }
    bool IsInCombat() { return combat; }
    bool IsCreature() { return creature; }
    bool HasBreakableByDamageCrowdControlAura() { return cc; }
    MotionMaster* GetMotionMaster() { return &motion; }
    Creature* ToCreature();
};
struct Creature : Unit
{
    explicit Creature(unsigned id = 1) : Unit(id) { creature = true; }
    int GetReactState() { return reaction; }
};
Creature* Unit::ToCreature() { return dynamic_cast<Creature*>(this); }
struct Group
{
    ObjectGuid skull;
    ObjectGuid GetTargetIcon(unsigned i) { return i == 7 ? skull : ObjectGuid{}; }
};
class Player : public Unit
{
public:
    bool tank = false, heal = false, ranged = false;
    Group* group = nullptr;
    Group* GetGroup() { return group; }
    bool IsValidAttackTarget(Unit* u) { return u && u->alive && u->valid && u->map == map; }
    bool IsWithinLOSInMap(Unit* u) { return u->los; }
    bool IsWithinCombatRange(Unit* u, float range) { return std::hypot(x - u->x, y - u->y) <= range; }
};
struct AttackersValue
{
    static bool IsValidTarget(Unit* target, Player* bot)
    {
        return target && target->world && bot->IsValidAttackTarget(target) &&
               bot->IsWithinCombatRange(target, 100.0f) && bot->IsWithinLOSInMap(target);
    }
};
uint32 clockNow = 1000;
class PlayerbotAI;
struct UntypedValue
{
    virtual ~UntypedValue() = default;
};
template <class T>
struct Value : UntypedValue
{
    T value{};
    virtual T Get() { return value; }
    virtual void Set(T v) { value = v; }
};
template <class T>
struct CalculatedValue : Value<T>
{
    PlayerbotAI* botAI;
    uint32 interval, last = 0;
    CalculatedValue(PlayerbotAI* ai, std::string, uint32 delay) : botAI(ai), interval(delay) {}
    T Get() override
    {
        if (!last || clockNow - last >= interval)
        {
            this->value = Calculate();
            last = clockNow;
        }
        return this->value;
    }
    virtual T Calculate() = 0;
};
using BoolCalculatedValue = CalculatedValue<bool>;
using ObjectGuidCalculatedValue = CalculatedValue<ObjectGuid>;
using ObjectGuidListCalculatedValue = CalculatedValue<GuidVector>;
template <class T>
struct NamedObjectContext
{
    std::map<std::string, T* (*)(PlayerbotAI*)> creators;
};
struct Context
{
    PlayerbotAI* ai;
    std::map<std::string, std::unique_ptr<UntypedValue>> values;
    explicit Context(PlayerbotAI* a) : ai(a) {}
    UntypedValue* Create(std::string const&);
    template <class T>
    Value<T>* GetValue(std::string const& name, std::string const& qualifier = "")
    {
        auto key = name + "::" + qualifier;
        if (!values.contains(key))
        {
            if (auto p = Create(name))
                values[key].reset(p);
            else
                values[key] = std::make_unique<Value<T>>();
        }
        auto p = dynamic_cast<Value<T>*>(values[key].get());
        assert(p);
        return p;
    }
};
class PlayerbotAI
{
public:
    Player* bot;
    Context context{this};
    std::map<ObjectGuid, Unit*> units;
    explicit PlayerbotAI(Player* p) : bot(p) {}
    Player* GetBot() { return bot; }
    Context* GetAiObjectContext() { return &context; }
    Unit* GetUnit(ObjectGuid id)
    {
        auto i = units.find(id);
        return i == units.end() ? nullptr : i->second;
    }
    float GetRange(std::string) { return 30; }
    static bool IsTank(Player* p) { return p->tank; }
    static bool IsHeal(Player* p) { return p->heal; }
    static bool IsRanged(Player* p) { return p->ranged; }
};
using AiObjectContext = Context;
struct Event
{
};
struct Action
{
    PlayerbotAI* botAI;
    Player* bot;
    Context* context;
    std::string name;
    Action(PlayerbotAI* a, std::string n = "") : botAI(a), bot(a->bot), context(&a->context), name(n) {}
    virtual ~Action() = default;
    virtual std::string const getName() { return name; }
    virtual bool isUseful() { return true; }
    virtual bool isPossible() { return true; }
    virtual bool Execute(Event) { return true; }
    virtual Unit* GetTarget() { return context->GetValue<Unit*>("current target")->Get(); }
};
struct AttackAction : Action
{
    using Action::Action;
    bool Attack(Unit* target)
    {
        context->GetValue<Unit*>("current target")->Set(target);
        return true;
    }
};
struct AttackMyTargetAction : AttackAction
{
    using AttackAction::AttackAction;
};
struct AttackDuelOpponentAction : AttackAction
{
    using AttackAction::AttackAction;
};
struct MeleeAction : AttackAction
{
    using AttackAction::AttackAction;
};
struct Trigger : Action
{
    using Action::Action;
    virtual bool IsActive() { return false; }
};
struct Multiplier : Action
{
    using Action::Action;
    virtual float GetValue(Action*) { return 1; }
};
struct FindNonCcTargetStrategy
{
    PlayerbotAI* botAI;
    Unit* result = nullptr;
    explicit FindNonCcTargetStrategy(PlayerbotAI* a) : botAI(a) {}
    virtual ~FindNonCcTargetStrategy() = default;
    virtual void CheckAttacker(Unit*, ThreatManager*) = 0;
    bool IsCcTarget(Unit* u) { return u->markedCc; }
    Unit* GetResult() { return result; }
};
struct NextAction
{
    std::string name;
    float score;
    NextAction(std::string n, float s) : name(n), score(s) {}
};
struct TriggerNode
{
    std::string name;
    std::vector<NextAction> actions;
    TriggerNode(std::string n, std::vector<NextAction> a) : name(n), actions(a) {}
};
struct Strategy
{
    PlayerbotAI* botAI;
    explicit Strategy(PlayerbotAI* a) : botAI(a) {}
    virtual ~Strategy() = default;
    virtual std::string const getName() = 0;
    virtual void InitTriggers(std::vector<TriggerNode*>&) {}
    virtual void InitMultipliers(std::vector<Multiplier*>&) {}
};
struct Config
{
    bool EnableBlackfathomMechanics = false;
} sPlayerbotAIConfig;
[[maybe_unused]] constexpr float ACTION_RAID = 60, ACTION_EMERGENCY = 90;
#define AI_VALUE(type, name) (context->GetValue<type>(name)->Get())

struct SharedCharmState
{
    bool active = false;
    int scans = 0, executions = 0;
} sharedCharm;
struct SharedCharmTrigger : Trigger
{
    SharedCharmTrigger(PlayerbotAI* a) : Trigger(a, "shared") {}
    bool IsActive() override
    {
        ++sharedCharm.scans;
        return sharedCharm.active;
    }
};
using RcCharmedMemberTrigger = SharedCharmTrigger;
using RcCharmedInChannelTrigger = SharedCharmTrigger;
using RcCharmedTargetTrigger = SharedCharmTrigger;
struct SharedCharmAction : Action
{
    SharedCharmAction(PlayerbotAI* a) : Action(a, "shared") {}
    bool Execute(Event) override
    {
        ++sharedCharm.executions;
        return sharedCharm.active;
    }
};
using RcControlCharmedAction = SharedCharmAction;
using RcStopChannelOnCharmedAction = SharedCharmAction;
struct RcLeaveCharmedTargetAction : AttackAction
{
    RcLeaveCharmedTargetAction(PlayerbotAI* a) : AttackAction(a, "shared target") {}
    bool Execute(Event) override
    {
        ++sharedCharm.executions;
        return sharedCharm.active;
    }
};
struct SharedCharmMultiplier : Multiplier
{
    SharedCharmMultiplier(PlayerbotAI* a) : Multiplier(a, "shared") {}
    float GetValue(Action*) override { return sharedCharm.active ? 0.0f : 1.0f; }
};
using RcCharmedSelfMultiplier = SharedCharmMultiplier;
using RcCharmedSplashMultiplier = SharedCharmMultiplier;
using RcCharmedDispelMultiplier = SharedCharmMultiplier;
