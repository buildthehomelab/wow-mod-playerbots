# Shadowfang Keep mechanics audit

October 22, 2026. Map 33 coverage includes 46 fixed/summoned actor definitions,
217 fixed spawn records, 173 object records (53 templates), all ordinary bosses,
Deathsworn Captain, both Sever definitions, quest interactions and the seasonal
Apothecary encounter. SHADOWFANG-ACTORS.csv gives per-actor base/live spell IDs,
bindings, counts and complete selected SmartAI behavior. Inventory records include
seasonal/progression actors; they do not prove that every actor is currently active.
This is source/data coverage, not a whole-route gameplay certification.

## Evidence

Fetched current authenticated Evermore sources:

- Playerbots evermore `22e730d480d5b29ece25ad7aafa0bbb58d46fe4d`.
- Core evermore `fccbd096804b33d71a36d7a3920f1b3256ff8cdd`.
- Individual Progression main `188f697a79a8d129816fd892bd44df962228d676`.
- Dungeon Clear evermore `c3722424ebe3bdf1d772a62c8ec7d3e3a3368bcd`.

Authenticated wow-server main manifests match these four revisions. These are
manifest/source references; loaded server code has not been identified by a build
receipt. Selected Playerbots/core/IP files were additionally read through the
GitHub connector. Realm readonly queries verified actor/GO bindings, progression
SmartAI, summons, seasonal addons and targeting conditions. Follow-up spell_dbc
query found no overrides for the new response IDs and selected seasonal chain.
Raw source/query snapshots remain in the workspace artifact, not in this module.
No SQL writes, configuration changes or server commands were performed.

References:

- [Core Shadowfang scripts](https://github.com/buildthehomelab/wow-azerothcore/tree/fccbd096804b33d71a36d7a3920f1b3256ff8cdd/src/server/scripts/EasternKingdoms/ShadowfangKeep).
- [Core world data](https://github.com/buildthehomelab/wow-azerothcore/tree/fccbd096804b33d71a36d7a3920f1b3256ff8cdd/data/sql/base/db_world).
- [Progression replacements](https://github.com/buildthehomelab/wow-mod-individual-progression/blob/188f697a79a8d129816fd892bd44df962228d676/data/sql/world/base/dungeon_shadowfang_keep.sql).
- [Progression Scourge invasion](https://github.com/buildthehomelab/wow-mod-individual-progression/blob/188f697a79a8d129816fd892bd44df962228d676/data/sql/world/base/si.sql): Sever clone 614682; si_creatures.sql places him in map 33.
- [Dungeon Clear events](https://github.com/buildthehomelab/wow-mod-dungeon-clear/blob/c3722424ebe3bdf1d772a62c8ec7d3e3a3368bcd/src/Ai/Dungeon/DungeonClear/Data/Events/ShadowfangKeepEvents.cpp).

Spell effects use the saved Evermore z80 reference Spell.dbc, inspected with the
current SpellEntry layout and a SpellMgr ID screen. This is not readback of the
worldserver's loaded DBC. Runtime radius uses CalcRadius/GetRadius, never DBC
radius indexes as yards. Cast index 1 denotes instant in this reference: Fel Stomp,
Screams of the Past and Thundershock do not receive fictitious cast escapes.

## Complete encounter and trash coverage

| Actor/content | Verified mechanics and response |
| --- | --- |
| Rethilgore 3914 | Soul Drain 7295, magic aura/stun and drain; generic legal interrupts/dispels and healing. Death sets prisoner data for both faction events. |
| Razorclaw 3886 | Butcher Drain 7485, victim drain; generic interrupts/healing, not a floor. |
| Silverlaine 3887 | Veil of Shadow 7068 is a curse reducing healing; existing decurse/heal logic. No retail spirit-boss summon mechanic in this version. |
| Springvale 4278 | Holy Light 1026 self/friendly healing and Divine Protection 498. Generic current/off-target healer interrupts and legal spell responses. IP removes base Hammer of Justice; do not script around labels/base alone. |
| Odo 4279 | Calls for help 40yd on aggro; Howling Rage 7481/7483/7484 increase physical output/attack speed at health thresholds. Tank/heal, normal CC/interrupt eligibility; not a harmful floor or cast to flee. |
| Fenrus 4274 | Toxic Saliva 7125 is poison DoT/slow; generic poison removal and recovery. Death starts voidwalker door sequence. |
| Nandos 3927 | Health-triggered calls 7487/7488/7489 bring Bleak Worg, Slavering Worg and Lupine Horror; generic tank add pickup, class CC and healing. Death opens Arugal's Lair. |
| Arugal 4275 | Void Bolt 7588, instant Thundershock 7803, Shadow Ports 7136/7586/7587, Curse 7621 (curse, charm, transform). Preserve interrupt/normal pursuit after ports. New local ally-damage protection and conditional legal curse recovery; no forced teleport navigation. |
| Deathsworn Captain 3872 | Rare AI-init despawn roll, Battle Stance 7165, Cleave 15496, Hamstring 9080. Ordinary tank/behind melee, recovery; no invented timed cleave dodge. |
| Sever 14682/614682 | Optional invasion variants. Live original has Diseased Spit 17745, Frenzy 8269, Intimidating Roar 16508; progression clone has Spit/Frenzy. Generic disease cleansing, healing, tank acquisition, available fear recovery/CC. No assumption both spawn simultaneously. |
| Son of Arugal 2529 | Arugal's Gift 7124 is a curse DoT, not charm; generic decurse. |
| Whitescalp 3851 | Frost Armor 12544; normal combat and legal class responses. |
| Moonwalker 3853 | Anti-Magic Shield 7121; retain class spell immunity/cast checks, melee, interrupts. No aura-name attack override. |
| Wolfguard 3854/Worg 5058 | Summon 7107 and Dark Restore 7106; generic interrupts and add acquisition. |
| Darksoul 3855 | Befuddlement 8140, Shadow Word: Pain 970; generic curse/magic removal and interrupts. |
| Glutton 3857 | Blood Tap 7122 victim leech; ordinary healing/combat. |
| Ragetooth 3859 | Wild Rage 7072 at low health; ordinary tank/heal, class CC. |
| Bleak Worg 3861 | Wavering Will 7127 magic movement/attack/cast slowing aura; generic dispel/recovery. |
| Slavering Worg 3862 | No bound damaging spell AI in selected live rows; normal combat/add pickup. |
| Lupine Horror 3863/Delusion 5097 | Summon 7132 produces 5097; delusion despawns below 99%/on death. Ordinary tank/DPS/CC; no persistent summon tracking or override of assigned CC. |
| Fel Steed 3864 | Instant Fel Stomp 7139 area stun/damage modifier; generic recovery/healing/positioning. |
| Vile Bat 3866/Blood Seeker 3868 | Diving Sweep 7145, Disarm 6713, Expose Weakness 7140. Ordinary role combat, legal magic dispels and recovery. |
| Tormented Officer 3873 | Forsaken Skills 7054 curse/periodic dummy; core script randomizes skill auras 7038..7053 (excludes 7052). Generic curse removal. No floor escape for a unit aura. |
| Haunted Servitor 3875 | Haunting Spirits 7057 curse; core script periodically casts 7067 from aura amount 7067, summoning Haunting Spirit 4958. Generic decurse/add response. No false ground hazard. |
| Haunting Spirit 4958 | Summoned by 7067; ordinary tank/add acquisition and class responses; live/base binding recorded in actor CSV. |
| Wailing Guardsman 3877 | Screams of the Past 7074 instant area magic silence; generic legal dispels/recovery. |
| Arugal's Voidwalker 4627 | Four summons, instant Dark Offering 7154 friendly heal and attack-on-reset; generic add pickup, class CC and healing. First death opens Sorcerer's Gate. |
| Hummel 36296/Baxter 36565 | Seasonal staged activation, perfume/cologne passive/spray and Chain Reaction 68821. New cast escape for Chain Reaction, preserve generic interrupts. Normal tank faces sprays away from group via existing role positioning; neutralizer immunities remain player/ordinary item decisions. |
| Frye 36272/Vial Bunny 36530 | Lingering Fumes 68704 triggers 68965, selects bunny, validates 68644, throws perfume/cologne and creates 68798/68614 dynamic areas. New escape from actually applied dynamic periodic-dummy areas, skipping matching 68529 perfume/68530 cologne immunity. |
| Crazed Apothecary 36568/Generator 36212 | Instance generators cast 68610 summons 36568; SmartAI attack player then instant Unstable Reaction 68957 near 3yd, despawn on evade. Ordinary add acquisition/damage/healing; no fabricated cast window. |
| Crown Apothecary 36885 | Despawns when Hummel activates; no bound combat AI in live template. |
| Vincent 4444 | Scripted feigned corpse/invincibility 1%, faction transition; ordinary interactions, no forced kill/priority. |
| Adamant 3849/Ashcrombe 3850 | Faction-specific freed-prisoner gossip/waypoints/timed door opening, retained DC events. |
| Arugal 10000 | Intro apparition visibility/dialogue/teleport/despawn; not final boss or damage target. |
| Landen 17822 | Gossip-triggered delayed spell/faction/attack; preserve player quest choice and ordinary combat if activated. |
| Rat 2110/Stalker 23033/Fezzen 38208 | Critter, invisible trigger and seasonal quest NPC; no invented combat response. |

## Doors, quests and whole-route behavior

Existing DC Alliance lever 18901/cell 18936/Ashcrombe gossip 21213 and Horde
lever 18900/cell 18934/Adamant gossip 21214 open Courtyard Door 18895 through timed
prisoner scripts, then persist courtyard state. Do not directly click bypass doors.
Fenrus death sets encounter state and data on 4275; timed list 427500 summons four
4627, which must engage before their 60s out-of-combat despawn. Existing DC waits
on their ring, clears them and confirms Sorcerer's Gate 18972; first death opens it.
Nandos death sets encounter 2 DONE and opens Lair 18971. Instance OnGOCreate restores
completed door states after grid/save reload. All five GO SmartAI lever rows were
inspected, including 18899's Lair target; no new lever bypass is introduced.

Object templates include Arugal's Focus 18973 (gossip quest object), Book of Ur 36738,
Jordan's Hammer 91138, Crate of Ingots 182011, supply/loot chests, neutralizers 202947/
202948, forge/anvils/fires and decorative furniture. Preserve player quest and loot
choices. Hummel gossip 10847/quest 14488 starts the seasonal event and summons Baxter/
Frye; boss death/feign and LFG completion stay in the core. No event auto-activation.

Roles retain generic tank acquisition, melee behind, ranged/healer positioning,
legal dispels, snares, CC, emergency healing and interrupt selection. No selected
map 33 actor has an assist-flee SmartAI action requiring new runner focus. No new
health-percentage runner guesses, kill order, wall/lava polygons or class rotation.
Existing DC multi-floor geometry/event-door fixes remain authoritative. Stairs,
balconies, Arugal ports, crowded rooms and pet navigation still require client tests.

## Implementation, cost and limits

Playerbots.EnableShadowfangMechanics defaults 0; set before instance initialization
with normal instance strategies enabled. Map entry/exit adds/removes vanilla-shadowfang
in balance. New decisions and multipliers are inert with the switch off.

A 250ms numeric hazard cache scans existing cached attackers for an active 68821 cast,
uses loaded radius+3yd and rejects other-floor casters by 3D distance. It inspects
all actually applied PERIODIC_DUMMY dynobj auras for 68798/68614. These are genuine
player hazards: core DynObjAura::FillTargetMap applies hostile units by 3D radius/LOS
without the source 13 entry-condition list; affected players self-cast 68927/68934.
Read source conditions alone and one would incorrectly conclude only bunnies hurt.
Applied aura inspection honors actual application and avoids any new NPC/grid scan.
All exposed overlapping areas contribute to the enclosing escape objective.

Existing melee/ranged flee planners and forced-priority normal-path MoveTo accept
partial outward progress under the generic FleeDistance cap. Existing progressing
splines are preserved; failed requests throttle 500ms. Root/control defers before
CastStop; planner/path failure preserves casts/movement state. Accepted cast-gated
PointMovement clears the cast only after acceptance and launches next core update.
Generic spells/interrupts/dispels/heals retain priority; escape 39 is below interrupts 40.
Competing movement yields while exposed. Pets receive ally-damage protection, not
new hazard recall. Protection checks generic/channel/auto-repeat casts and current
victim/target/pet victim; positive/dispel casts remain available. Local harmful area
protection conservatively checks loaded effect radius around caster/unit target in
3D, not every area spell anywhere on map. It is not a complete spell collision/
chain-jump/ground-destination simulation; such residual cases need client validation.

Cursed party GUIDs and candidate recovery GUID are cached 250ms, never raw pointers.
Explicit core target legality, actual dispellable aura, alive/group/map/range/LOS and
spell ownership are checked. Generic HasAuraToDispel skips negative effects on a
hostile charmed ally; recovery supplies that missing candidate only when legal.
Reference 475/2782/51886 target allies: a mutually hostile charmed player fails core
IsValidAssistTarget. The feature does not bypass that restriction or promise that
an ordinary decurse will remove an active hostile charm. Ally protection, recovery
when core permits it, and normal expiry remain; no granted spells or server rule
changes. No guaranteed interrupt/dispel availability is claimed for low-level groups.

## Validation and acceptance

Run python 3 tests/test_shadowfang.py. It compiles/executes the actual encounter code
with engine doubles: switch/map/creators, curse group/range/LOS/late loss/illegal
target, local safe vs dangerous area casts, legal recovery, bot/pet protection,
instant-spell exclusion, roots, failed planning/path preservation, throttle,
cast-deferred next-update launch, preserved splines, three capped 5yd steps,
actually applied spills, matching immunity and overlapping areas. These tests do
not prove core ABI, collision, faction state, spell legality or gameplay.
Run module codestyle, touched encounter/fixture clang-format and git diff --check.
Independent and formal final-revision PR reviews are required and tracked on the PR.

No full server build (repository forbids builds unless requested), pmon or game
client run was performed. Whole-route acceptance must cover every table row,
all rare, optional and event actors, both factions' prisoner choices, voidwalker delay/
wipe/re-entry, Nandos dogs, Arugal ports/charm/expiry, legal curse attempts vs rejected
hostile decurse, pet/autoshot/channel safety, nearby/distant AoE, dispels/interrupts,
seasonal three-boss activation/feign, sprays/neutralizers, overlapping spills,
Chain Reaction, roots/blocked paths and switch-off baseline. Profile with
BotActiveAlone=100 and botActiveAloneSmartScale=0; pmon toggle after login, stack
five minutes later, compare final Total. Loaded-code and whole-route validation
are separate from readonly world-data verification.

Keep unmerged, disabled and undeployed until authorized. After an authorized
implementation merge, a separate reviewed wow-server PR must pin the actual
reachable resulting commit. No branch SHA is a deployment pin or live receipt.
