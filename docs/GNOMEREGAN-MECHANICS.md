# Gnomeregan mechanics coverage — October 23, 2026

## Evidence and scope

The readonly map90 inventory contains **404 fixed spawns,33 fixed actor types,87
objects and30 object templates**. GNOMEREGAN-ACTORS.csv covers all33 plus six
recursively confirmed summon-only actors: Grubbis7361,Chomper6215,Burning
Servant7738,Ember7266,Walking Bomb7915 and Dark Iron Land Mine8035 (39 total).
Supplemental queries of outdoor6208/6210 are not counted as instance actors.
No pools were returned for the fixed map90 GUIDs. The selected creature/object
and recursive SmartAI set contains229 rows; this includes supplemental outdoor
rows, which are separate from instance coverage. Large unrelated timed-action
ranges from the initial query are not treated as Gnomeregan mechanics.

Fresh fetched baselines: Playerbots7779c91e70bf7f26547126850cadf7e881e71faa,
Dungeon Clear initially1f1632d640ba9264798b43939b4bba8f4adee0da; refreshed
companion target5d49ee7ab0595de4031c73f6a2b41e17b371d13c (unrelated BWL hold),
core48500d15204fc758894417ff1c06557babe35ad0,
Individual Progression188f697a79a8d129816fd892bd44df962228d676.
Authenticated wow-server main still pins Playerbots22e730d4/DCc3722424 and the
same IP revision; branch HEADs and recorded pins are different facts. Source
verification includes current core instance/SpellEffects/SpellInfoCorrections,
base SmartAI and IP dungeon_gnomeregan.sql, plus readonly world template,AI,
object,group,quest,condition,linked-spell,target-position and spell_dbc reads.

Spell effects use the saved Evermore z80 Spell.dbc reference. The observed client
spell IDs have no readonly spell_dbc overrides. **11518 is a server-only spell**:
live spell_dbc supplies Activate Bomb01 with a gameobject target and activation
effect;11511 links11518 and has a map90 destination. Faces listen for11518,
open and repeatedly summon7915; buttons send data1=0 to their matching face.
This closes the actual link, rather than guessing from spell comments. Loaded
server DBC/binary provenance and runtime cast/path behavior remain unverified.
The dungeonencounter_dbc override table has no relevant replacement rows;
Dungeon Clear reads the actual loaded379 record at runtime, never guesses its
completion-mask bit. No configuration,DB or server writes were performed.

Earlier Deadmines21,WC27,RFC28,SFK29 and Stockade30 PRs were rechecked open and
unmerged. Their sources are reference patterns, never branch dependencies.
Blackfathom remains independently owned and is not a dependency.

## Encounter, trash and roles

The CSV is the per-actor matrix. Grubbis/Chomper use ordinary tank/add combat;
the missing escort/navigation objective is supplied in companion Dungeon Clear.
Viscous Fallout's Toxic Volley has poison type; a learned legal poison removal
remains class-owned. Cloud10341 is persistent-area effect27,not a spell to be
removed by magic cleanse. Its normal casts are covered by generic dynamic-object
avoidance; IP ADD_AURA10341 alone is not proof a harmful floor was instantiated.
Electrocutioner's Shock/Megavolt/Chain Bolt retain generic legal interrupts,
healing and role positioning. Pummeler's frontal/instant attacks retain tank
facing/rear melee; no fictional interruptible windup is added. Thermaplugg retains
generic taunting/threat recovery for Knock Away; bot encounter logic closes
active bomb faces and deals with real summoned bombs.

Dark Iron Ambassador's present readonly AI includes despawn500ms on initialization.
Its Fireball/shields,10870 servants and recursive10869 embers are audited as
source definitions, not claimed live rare spawns. Actor7897 is Alarm-a-bomb2600,
not Walking Bomb7915. Trigger13876 has no harmful aura AI. No mechanic is added
for those actors without an actual live hazard state.

All seven verified runner entries are listed in the local switch and CSV. Only
actual ASSISTANCE_MOTION_TYPE (including passive seeking-assist) or active flee
movement qualifies; generic faction/CC/range/LOS/role/order checks still apply.
The stationary Mobile Alert System is separately focused before its repeated
reinforcements. Ranged DPS may focus nearby Walking Bombs. Tanks/healers keep
class roles and explicit valid skull/prioritized orders win over encounter focus.
Manual AttackMyTarget,duel acquisition and shared charm cleanup are exempt from
the acquisition multiplier; ordinary melee and class spells retain priorities.

Net6533 is physical dispel-type0; other magic roots/Pacify/Corrosive Ooze use
generic legal cleanse/recovery. Frenzy8269,Supercharge and Tune Up are not
invented dispellable enrages/magic buffs. Core legality,learned ranks,resources,
range,LOS,cooldowns and immunities still determine interrupts,CC and dispels.
There is no duplicate per-class spell selection.

## Objects, events and quest content

All30 fixed object templates are audited: six faces142208-142213 and six
buttons142214-142219; cave-ins146085/146086; Workshop Door90858 and Final
Chamber142207; elevator85556; mailbox144179; punchographs142475/142476/142696;
Sparklematics142487/175084/175085; quest containers19020/142344/142477;
food/chest3659/3695/74448. The saved inventory contains87 placements. Forty-four object SmartAI rows cover the faces,
buttons,punch-card creation and Sparklematic animations. Other listed containers
and elevator do not introduce creature combat AI.

Buttons are matched by verified numeric data targets,never name search:
142214->142211,142215->142210,142216->142209,142217->142208,
142218->142213,142219->142212. The nearest eligible bot DPS handles each
reachable nearby button,with GUID tie-break,without assigning tanks/healers or
human players. Actions recheck ready button/open face,range,LOS,map,life and
config and use the actual GameObject::Use path; no direct boss-state writes.
Escape supersedes the button; straight approach segments entering known hazards
are rejected. The real curved navmesh path still requires client validation.

Emi's gossip1080 option0 starts path7998. Her groups1-3 summon Ambushers and
group4 summons Grubbis/Chomper. Persistent Dungeon Clear escort uses existing
protect/engage/gossip/recovery machinery and stays through the loaded Grubbis
kill bit. The core instance saves Grubbis and cave/door state; it does not expose
GetData,so the implementation does not use an ineffective instance-data gate.

The map133 quest category and explicit A Fine Mess quest were inspected. Core
Kernobee follows the accepting player and credits2904 near the entrance; no
new quest acceptance or reward automation is introduced. Punch-card stages,
Sparklematic grime/ring hand-ins,Parts/Extrapolator/Safe and kill/loot quests
retain generic/manual quest/item interactions. Techbot and Matrix Punchograph A
are exterior content,not fabricated map90 spawns. No outdoor combat event is
installed or enabled by this change. Quest sample extraction and use/hand-in
completion remain client acceptance items,not a claim of automated completion.

## Implementation, checks and limits

Playerbots.EnableGnomereganMechanics defaults0. Unconditional contexts and
map90 instance strategy hooks permit config reload with cheap runtime guards.
Focus/buttons/reflect use500ms GUID/bool caches; hazards/approach use250ms caches
over existing cached lists. No new DB queries,name searches,grid-traversal implementation,cross-map state or
cross-tick raw creature pointers are introduced. Existing nearest-unit/object values can refresh their grid cache
when requested; the enabled collection cost includes those refreshes. Shared reflect logic is reused
only while enabled on map90 and a cached real reflect exists.

Escape uses existing capped flee planners,requires progress against every
currently overlapping hazard,rejects known hazard-crossing approach segments,
preserves a useful spline,throttles failures500ms and defers before CastStop when
immobile. Failed planner/path requests preserve casts/movement. Cast-gated
accepted PointMovement is allowed to launch on next update; stale spline data is
not read as the new path. Generic approach movement is blocked while threatened
or when moving toward a hostile current target inside a known area; safe healing/
resurrection movement toward other allies is retained; shared escape,casts,attacks,
manual acquisition and charm cleanup remain available.

Source-execution tests compile actual encounter code against engine doubles with
-Wall -Wextra -Werror. Companion event-authoring tests execute actual table code
against data/builder doubles,including missing/wrong-map/out-of-range DBC records
and a non-hardcoded completion bit. These do not establish core ABI,actual
pathfinding,spell hit behavior,escort timeout behavior or whole-route completion.
Repository rules prohibit an unrequested full server build; full gtest runner
also configures/builds the core and is not run. Full build,pmon and game-client
validation remain pending. Independent and formal reviews are recorded separately
in the workspace report; no pending review is represented as complete.

## Acceptance protocol (requires separate runtime authorization)

1. Baseline and enabled runs at level32 with tank/healer,melee/ranged and pets,
   both human-led and autonomous. Clear every named actor and available rare,
   optional Grubbis route,all trash floors and every quest/object branch.
2. Check Fallout/slime clouds,magic/physical roots,poison removal,healing and legal
   interrupts; overlap two mines,bombs and Hail Storm. Test failed paths,roots,
   capped steps,cast-delayed launch,narrow ramps/ledges and stacked floors.
3. Test Alert reinforcements,all seven runner entries,passive assistance,late CC,
   stale skull/priorities,manual attack/duel and generic charm/pet cleanup. Test
   ranged bomb attacks and all-melee fallback without promising pet avoidance.
4. Activate/close all six faces,simultaneous DPS ownership,healer/tank exclusion,
   state change before execution,path blocked and wipe/reset/death cleanup.
5. Start Emi,protect every wave and both final summons,wipe during each phase,
   recover/reset/re-enter,verify the kill mask and proceed after Grubbis death.
   Test saved completed instance with Emi absent and map/config transitions.
6. Validate elevator,doors,cave-ins,containers,all punch stages,Sparklematic,
   Kernobee following/exit credit and outdoor Techbot separation in the client.
7. Compare playerbot pmon baseline/enabled using BotActiveAlone100,
   botActiveAloneSmartScale0; toggle after login and stack after five minutes.
   No live performance,loaded-binary,pet-recall,whole-path or whole-route proof
   is inferred from passing source checks.

Keep unmerged,disabled and undeployed. After explicitly authorized implementation
merge,prepare separately reviewed wow-server pin PRs to actual reachable merged
commits. A proposed branch SHA is not a deployment pin.
