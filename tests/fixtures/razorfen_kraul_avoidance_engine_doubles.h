// Minimal engine doubles for executing the real RazorfenKraul sources; not a game simulation.
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
    bool operator==(Position const&) const = default;
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
namespace G3D
{
struct Vector3
{
    float x = 0, y = 0, z = 0;
};
}  // namespace G3D
struct Spline
{
    bool done = true;
    G3D::Vector3 destination;
    bool Finalized() { return done; }
    G3D::Vector3 FinalDestination() { return destination; }
};
class Unit;
struct SpellEffectInfo
{
    float radius = 5;
    float CalcRadius(Unit*) const { return radius; }
};
struct SpellInfo
{
    bool positive = false, area = false, dispel = false;
    uint32 Id = 8422;
    std::array<SpellEffectInfo, 3> Effects;
    bool HasEffect(int) const { return dispel; }
    bool IsPositive() const { return positive; }
    bool IsAffectingArea() const { return area; }
};
struct SpellManager
{
    std::map<uint32, SpellInfo> spells;
    SpellInfo const* GetSpellInfo(uint32 id)
    {
        auto i = spells.find(id);
        return i == spells.end() ? nullptr : &i->second;
    }
} spellManager;
auto sSpellMgr = &spellManager;
struct DynamicObject : Position
{
    bool world = true;
    float radius = 5;
    Position const& GetPosition() { return *this; }
    bool IsInWorld() { return world; }
    float GetRadius() { return radius; }
};
struct Aura
{
    uint32 id = 14515;
    bool removed = false, expired = false;
    int type = 1;
    DynamicObject* area = nullptr;
    SpellInfo info;
    uint32 GetId() { return id; }
    bool IsRemoved() { return removed; }
    bool IsExpired() { return expired; }
    int GetType() { return type; }
    SpellInfo const* GetSpellInfo() { return &info; }
    DynamicObject* GetDynobjOwner() { return area; }
} charmAura;
enum AuraType
{
    SPELL_AURA_PERIODIC_DAMAGE,
    SPELL_AURA_PERIODIC_DAMAGE_PERCENT,
    SPELL_AURA_PERIODIC_TRIGGER_SPELL,
    SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE,
    SPELL_AURA_DUMMY
};
struct AuraEffect
{
    Aura* aura;
    Aura* GetBase() { return aura; }
};
using DispelChargesList = std::vector<std::pair<Aura*, uint8>>;
[[maybe_unused]] constexpr int REACT_PASSIVE = 0, REACT_AGGRESSIVE = 2;
enum CurrentSpellTypes
{
    CURRENT_GENERIC_SPELL,
    CURRENT_CHANNELED_SPELL
};

struct SpellTargets
{
    ObjectGuid target;
    ObjectGuid GetUnitTargetGUID() { return target; }
};
struct Spell
{
    SpellInfo const* m_spellInfo;
    SpellTargets m_targets;
};
class Unit;
class Player;
struct Creature;
struct Pet;
struct Group;
struct ThreatReference
{
    Unit* owner;
    Unit* GetOwner() { return owner; }
};
struct ThreatManager
{
    std::map<ObjectGuid, ThreatReference*> refs;
    int reads = 0;
    auto const& GetThreatenedByMeList()
    {
        ++reads;
        return refs;
    }
};
enum MovementGeneratorType
{
    IDLE_MOTION_TYPE,
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
    bool world = true;
    bool IsInWorld() { return world; }
    uint32 GetMap() { return map; }
    Position GetPosition() { return *this; }
    MotionMaster motion;
    MotionMaster* GetMotionMaster() { return &motion; }
    ObjectGuid guid;
    uint32 entry = 0, map = 189;
    bool alive = true, combat = true, creature = false, player = false, valid = true, los = true, cc = false,
         markedCc = false, dispelEligible = true;
    int reaction = REACT_AGGRESSIVE;
    std::set<uint32> auras, casts;
    std::map<AuraType, std::vector<AuraEffect*>> effects;
    auto const& GetAuraEffectsByType(AuraType type) { return effects[type]; }
    ThreatManager threat;
    Unit* victim = nullptr;
    Spell* currentSpell[2] = {};
    Spell* GetCurrentSpell(CurrentSpellTypes slot) { return currentSpell[slot]; }
    explicit Unit(unsigned id = 1) : guid(id) {}
    virtual ~Unit() = default;
    bool IsAlive() { return alive; }
    uint32 GetMapId() { return map; }
    uint32 GetEntry() { return entry; }
    ObjectGuid GetGUID() { return guid; }
    bool IsInCombat() { return combat; }
    bool IsCreature() { return creature; }
    bool IsPlayer() { return player; }
    bool HasAura(uint32 id) { return auras.contains(id); }
    bool FindCurrentSpellBySpellId(uint32 id) { return casts.contains(id); }
    bool HasBreakableByDamageCrowdControlAura() { return cc; }
    Creature* ToCreature();
    Player* ToPlayer();
    ThreatManager& GetThreatMgr() { return threat; }
    Unit* GetVictim() { return victim; }
    float GetExactDist2d(Unit* unit) { return std::hypot(x - unit->x, y - unit->y); }
    void GetDispellableAuraList(Unit*, uint32, DispelChargesList& list, SpellInfo const*)
    {
        if (dispelEligible && auras.contains(14515))
            list.push_back({&charmAura, 1});
    }
};
struct Creature : Unit
{
    explicit Creature(unsigned id = 1) : Unit(id) { creature = true; }
    int GetReactState() { return reaction; }
};
struct Pet : Creature
{
    int stops = 0, castStops = 0;
    void CastStop() { ++castStops; }
    void AttackStop()
    {
        ++stops;
        victim = nullptr;
    }
};
class Player : public Unit
{
public:
    bool tank = false, heal = false, ranged = false, moving = false;
    bool movementCast = false, deferredMove = false;
    G3D::Vector3 deferredDestination;
    bool IsMovementPreventedByCasting() { return movementCast; }
    void MovementUpdate()
    {
        if (deferredMove && !movementCast)
        {
            moving = true;
            spline.done = false;
            spline.destination = deferredDestination;
            deferredMove = false;
        }
    }
    Spline spline;
    Spline* movespline = &spline;
    Pet* pet = nullptr;
    Group* group = nullptr;
    int castStops = 0, attackStops = 0;
    std::set<uint32> spells;
    ObjectGuid selection;
    explicit Player(unsigned id = 1) : Unit(id) { player = true; }
    Group* GetGroup() { return group; }
    bool IsValidAttackTarget(Unit* u) { return u && u->alive && u->valid && u->map == map; }
    bool IsWithinLOSInMap(Unit* u) { return u->los; }
    float GetDistance(DynamicObject* area) { return std::hypot(x - area->x, y - area->y); }
    bool IsWithinCombatRange(Unit* u, float range) { return GetExactDist2d(u) <= range; }
    bool IsInSameRaidWith(Player* p);
    bool HasSpell(uint32 id) { return spells.contains(id); }
    bool isMoving() { return moving; }
    void StopMoving()
    {
        moving = false;
        spline.done = true;
    }
    void CastStop()
    {
        ++castStops;
        movementCast = false;
    }
    void AttackStop()
    {
        ++attackStops;
        victim = nullptr;
    }
    void SetSelection(ObjectGuid id) { selection = id; }
    Pet* GetPet() { return pet; }
};
Creature* Unit::ToCreature() { return dynamic_cast<Creature*>(this); }
Player* Unit::ToPlayer() { return dynamic_cast<Player*>(this); }
struct GroupReference
{
    Player* source = nullptr;
    GroupReference* nextMember = nullptr;
    Player* GetSource() { return source; }
    GroupReference* next() { return nextMember; }
};
struct Group
{
    ObjectGuid skull;
    GroupReference* first = nullptr;
    ObjectGuid GetTargetIcon(unsigned i) { return i == 7 ? skull : ObjectGuid{}; }
    GroupReference* GetFirstMember() { return first; }
};
bool Player::IsInSameRaidWith(Player* p) { return group && group == p->group; }
struct LastMovement
{
    bool waiting = true;
    int clears = 0;
    void clear()
    {
        waiting = false;
        ++clears;
    }
};
uint32 clockNow = 1000;
uint32 getMSTime() { return clockNow; }
uint32 getMSTimeDiff(uint32 a, uint32 b) { return b - a; }
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
struct GameObjectDisplayInfoEntry
{
    float minX = -3, maxX = 3, minY = -2, maxY = 2;
};
struct DisplayStore
{
    bool present = false;
    GameObjectDisplayInfoEntry info;
    GameObjectDisplayInfoEntry const* LookupEntry(uint32) { return present ? &info : nullptr; }
} sGameObjectDisplayInfoStore;
struct GameObjectInfo
{
    uint32 displayId = 1;
};
struct GameObject : Position
{
    uint32 entry = 69431, map = 47;
    bool world = true, spawned = true;
    bool IsInWorld() { return world; }
    bool isSpawned() { return spawned; }
    uint32 GetMap() { return map; }
    uint32 GetEntry() { return entry; }
    float scale = 1.0f;
    GameObjectInfo info;
    GameObjectInfo const* GetGOInfo() { return &info; }
    float GetObjectScale() { return scale; }
    float GetObjectSize() { return 0.5f; }
    bool IsInRange3d(float a, float b, float c, float radius)
    {
        if (sGameObjectDisplayInfoStore.present)
        {
            auto const& d = sGameObjectDisplayInfoStore.info;
            return a - x < d.maxX * scale + radius && a - x > d.minX * scale - radius &&
                   b - y < d.maxY * scale + radius && b - y > d.minY * scale - radius && std::abs(c - z) < radius;
        }
        return GetExactDist(a, b, c) < radius + GetObjectSize();
    }
    Position const& GetPosition() { return *this; }
};
class PlayerbotAI
{
public:
    Player* bot;
    Context context{this};
    std::map<ObjectGuid, Unit*> units;
    LastMovement movement;
    bool canMove = true;
    bool CanMove() { return canMove; }
    bool canCast = true, castSucceeds = true, moveBlocked = false, pathRejected = false;
    uint32 castId = 0;
    Unit* castTarget = nullptr;
    int moves = 0;
    float fleeDistance = 5.0f;
    explicit PlayerbotAI(Player* p) : bot(p) {}
    Player* GetBot() { return bot; }
    Context* GetAiObjectContext() { return &context; }
    std::map<ObjectGuid, GameObject*> objects;
    GameObject* GetGameObject(ObjectGuid id)
    {
        auto i = objects.find(id);
        return i == objects.end() ? nullptr : i->second;
    }
    Unit* GetUnit(ObjectGuid id)
    {
        auto i = units.find(id);
        return i == units.end() ? nullptr : i->second;
    }
    float GetRange(std::string) { return 30; }
    static bool IsTank(Player* p) { return p->tank; }
    static bool IsHeal(Player* p) { return p->heal; }
    static bool IsRanged(Player* p) { return p->ranged; }
    static bool IsMelee(Player* p) { return !p->ranged; }
    bool CanCastSpell(uint32 id, Unit*) { return id && canCast; }
    bool CastSpell(uint32 id, Unit* t)
    {
        castId = id;
        castTarget = t;
        return id && castSucceeds;
    }
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
enum class MovementPriority
{
    MOVEMENT_NORMAL,
    MOVEMENT_FORCED
};
struct MovementAction : Action
{
    using Action::Action;
    Position BestPositionForMeleeToFlee(Position pos, float radius)
    {
        if (botAI->moveBlocked)
            return {};
        float distance = std::min(radius + 1.0f, botAI->fleeDistance);
        float angle = std::atan2(bot->y - pos.y, bot->x - pos.x);
        Position result;
        result.Relocate(bot->x + std::cos(angle) * distance, bot->y + std::sin(angle) * distance, bot->z);
        return result;
    }
    Position BestPositionForRangedToFlee(Position pos, float radius) { return BestPositionForMeleeToFlee(pos, radius); }
    bool MoveTo(uint32, float x, float y, float z, bool, bool, bool, bool, MovementPriority priority)
    {
        if (botAI->pathRejected || (botAI->movement.waiting && priority != MovementPriority::MOVEMENT_FORCED))
            return false;
        ++botAI->moves;
        if (bot->IsMovementPreventedByCasting())
        {
            bot->deferredMove = true;
            bot->deferredDestination = {x, y, z};
            botAI->movement.waiting = true;
            return true;
        }
        bot->moving = true;
        bot->spline.done = false;
        bot->spline.destination = {x, y, z};
        botAI->movement.waiting = true;
        return true;
    }
    bool FleePosition(Position pos, float radius, uint32 = 1000)
    {
        Unit center;
        center.x = pos.x;
        center.y = pos.y;
        // Match the generic helper's capped step, rather than jumping straight out.
        return MoveAway(&center, std::min(radius + 1.0f, botAI->fleeDistance));
    }
    bool MoveAway(Unit* u, float distance)
    {
        if (botAI->movement.waiting || botAI->moveBlocked)
            return false;
        ++botAI->moves;
        bot->moving = true;
        bot->spline.done = false;
        float angle = std::atan2(bot->y - u->y, bot->x - u->x);
        bot->spline.destination = {bot->x + std::cos(angle) * distance, bot->y + std::sin(angle) * distance, bot->z};
        botAI->movement.waiting = true;
        return true;
    }
};
struct AvoidAoeAction : MovementAction
{
    explicit AvoidAoeAction(PlayerbotAI* ai) : MovementAction(ai, "avoid aoe") {}
    bool AvoidAuraWithDynamicObj()
    {
        auto aura = context->GetValue<Aura*>("area debuff")->Get();
        if (!aura || !aura->area)
            return false;
        Unit center;
        center.x = aura->area->x;
        center.y = aura->area->y;
        return MoveAway(&center, aura->area->radius + 2 - bot->GetExactDist2d(&center));
    }
};
struct AttackAction : MovementAction
{
    using MovementAction::MovementAction;
    bool Attack(Unit* target)
    {
        context->GetValue<Unit*>("current target")->Set(target);
        bot->victim = target;
        return true;
    }
};
struct MeleeAction : AttackAction
{
    using AttackAction::AttackAction;
};
struct CastSpellAction : Action
{
    std::string spell;
    CastSpellAction(PlayerbotAI* a, std::string s) : Action(a, s), spell(s) {}
    std::string getSpell() { return spell; }
};
struct CastEscapeArtistAction : CastSpellAction
{
    explicit CastEscapeArtistAction(PlayerbotAI* ai) : CastSpellAction(ai, "escape artist") {}
};
struct CastReachTargetSpellAction : CastSpellAction
{
    using CastSpellAction::CastSpellAction;
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
enum class TargetValueExclusionType
{
    TankTarget,
    DpsTarget,
    DebuffTarget,
    Aoe
};
[[maybe_unused]] constexpr float ACTION_INTERRUPT = 40;
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
    virtual bool HasTargetExclusions() const { return false; }
    virtual void AppendTargetExclusions(GuidSet&, TargetValueExclusionType) {}
};
struct Config
{
    bool EnableRazorfenKraulMechanics = false;
    bool EnableHerodWhirlwindAvoidance = false, EnableScarletMonasteryMechanics = false;
    std::set<uint32> AoeAvoidSpellWhitelist;
    float MaxAoeAvoidRadius = 20;
    Config(bool a = false, bool b = false) : EnableHerodWhirlwindAvoidance(a), EnableScarletMonasteryMechanics(b) {}
} sPlayerbotAIConfig;
[[maybe_unused]] constexpr float ACTION_EMERGENCY = 90, ACTION_RAID = 60;
#define AI_VALUE(type, name) (context->GetValue<type>(name)->Get())
#define AI_VALUE2(type, name, qualifier) (context->GetValue<type>(name, qualifier)->Get())
// The last-movement value is a reference in the real engine.
template <>
Value<LastMovement&>* Context::GetValue<LastMovement&>(std::string const&, std::string const&);
template <>
struct Value<LastMovement&> : UntypedValue
{
    LastMovement& value;
    explicit Value(LastMovement& v) : value(v) {}
    LastMovement& Get() { return value; }
};
template <>
Value<LastMovement&>* Context::GetValue<LastMovement&>(std::string const&, std::string const&)
{
    std::string key = "movement";
    if (!values.contains(key))
        values[key] = std::make_unique<Value<LastMovement&>>(ai->movement);
    return static_cast<Value<LastMovement&>*>(values[key].get());
}
