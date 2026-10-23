# Blackfathom Deeps: full instance source and readonly-data review

October 23, 2026. Scope is map48, including optional/quest objects and recursive
summons. This is source/data coverage, not proof of a completed client route.

## Verified revisions and boundaries

| Component | Fetched/authenticated revision | Authenticated server-manifest pin |
| --- | --- | --- |
| Playerbots evermore | `89a8ca1a9bc380e336d38e968de3b1f05c3d1c04` | `22e730d480d5b29ece25ad7aafa0bbb58d46fe4d` |
| Dungeon Clear evermore | `1f1632d640ba9264798b43939b4bba8f4adee0da` | `c3722424ebe3bdf1d772a62c8ec7d3e3a3368bcd` |
| Evermore core | `48500d15204fc758894417ff1c06557babe35ad0` | same |
| Individual Progression | `188f697a79a8d129816fd892bd44df962228d676` | same |

Selected current files were independently read through the authenticated GitHub
connector as well as fetched Git. A branch HEAD is distinct from a manifest pin
and neither establishes the binary currently loaded by worldserver.

References: [core instance](https://github.com/buildthehomelab/wow-azerothcore/blob/48500d15204fc758894417ff1c06557babe35ad0/src/server/scripts/Kalimdor/BlackfathomDeeps/instance_blackfathom_deeps.cpp),
[instance enums](https://github.com/buildthehomelab/wow-azerothcore/blob/48500d15204fc758894417ff1c06557babe35ad0/src/server/scripts/Kalimdor/BlackfathomDeeps/blackfathom_deeps.h),
[IP replacement](https://github.com/buildthehomelab/wow-mod-individual-progression/blob/188f697a79a8d129816fd892bd44df962228d676/data/sql/world/base/dungeon_blackfathom_deeps.sql),
[shared charm AI](https://github.com/buildthehomelab/wow-mod-playerbots/blob/89a8ca1a9bc380e336d38e968de3b1f05c3d1c04/src/Ai/Raid/RaidClear/Common/Charmed.cpp).

Read-only live world data:213 fixed creature rows,28 fixed actor types,79 creature
SmartAI rows,50 objects/19 templates,10 object SmartAI rows. Recursion adds six
actor templates and15 AI rows:6047/8996 spell guardians,4977/4978 fire waves,
12876 Baron Aquanis and6729 Morridune. Four GO summon groups contain14 rows.
Lorgus Jett has three pool locations,not three simultaneously guaranteed bosses.
All34 actors are enumerated in BLACKFATHOM-ACTORS.csv. No additional fixed rare
actor was found; optional Old Serra'kis and pooled Lorgus remain covered.

All observed cast IDs plus addon aura9464 and reviewed triggered spell IDs have
no live spell_dbc override. Reference spell effects were extracted from saved
Evermore z80 Spell.dbc,not from the DBC loaded in the server process. Barbed
Crustacean's passive Barbs9464 is an addon damage shield,not a floor. Base SQL
selected99 relevant SmartAI rows; IP replacements and live rows are authoritative
for responses. Pending-update filename screening found no Blackfathom-named
file; this is not exhaustive migration replay. No database/configuration writes.

## Complete mechanics and response matrix

| Actor/content | Verified behavior | Response ownership/decision |
| --- | --- | --- |
| Ghamoo-ra4887 | Instant Trample5568,caster area | Tank/heal and ordinary ranged spacing. No cast-time dodge or invented shell phase. |
| Lady Sarevess4831 | Shoot6660,Forked Lightning8435,instant Frost Nova865,Slow246;range-based movement | Generic interrupts where spell flags allow,magic cleanse/root recovery and tank approach;ordinary ranged/healer spacing. Preserve existing DC submerged approach. No fabricated floor. |
| Gelihast6243 | Net6533,linked30% victim-threat reduction;death unlocks shrine | Generic legal root recovery;dispelless net is not magic cleanse. Tanks reacquire after threat reduction. Shrine interaction remains ordinary object/quest AI. |
| Lorgus Jett12902 | Pooled one-of-three spawn;Lightning Shield12550,Lightning Bolt12167 | Generic interrupt and hostile magic removal,tank/heal;quest kill support. No retail totem mechanic. |
| Old Serra'kis4830 | No bound combat AI or cast rows | Ordinary tank/heal/underwater positioning. No invented frenzy or bleed. |
| Twilight Lord Kelris4832 | OOC channel8734,Sleep8399,Mind Blast15587 | Generic interrupt/magic cleanse. Sleep is caster-centered hostile area aura12 despite SmartAI random target6. New cast-only escape attempt;no distant-target sleep zone. |
| Aku'mai4829 | Poison Cloud3815,Frenzied Rage3490 | New cast-only poison-application escape. Poison is periodic aura applied around caster,not persistent floor;after landing use generic poison cure/healing. Rage dispel category0:do not invent enrage removal. Tank/heal remains necessary. |
| Baron Aquanis12876 | Fathom Stone177964 deactivation summons;Frostbolt15043,Frost Nova14907 | Generic interrupts,magic/root cure and tank/heal. Optional quest event remains player/quest-object owned;not automatically activated by combat strategy. |
| Fallenroot Shadowstalker4798 | Stealth5916,Curse of Weakness6205 | Generic reveal/target acquisition,tank and curse removal. |
| Fallenroot Hellcaller4799 | Shadow Bolt9613,Mana Burn8129 | Generic interrupts,healing/mana management;no runner row. |
| Blackfathom Sea Witch4805 | Frost Armor12544,Nova122,Ward6143,assist flee | Generic hostile magic removal/root cure;new nearby DPS runner focus. |
| Blackfathom Myrmidon4807;Murkshallow Snapclaw4815 | Disarm8379,assist flee | Generic role/recovery;new runner focus. Disarm is not removable magic. |
| Twilight Acolyte4809 | Heal2055,Renew8362,assist flee | Generic healer/current-target interrupts and hostile magic removal;new runner focus. No duplicate healer spell/CC policy. |
| Twilight Reaver4810 | Instant frontal Arcing Smash8374,assist flee | Tank faces melee away from party;generic role positioning/healing;new runner focus. |
| Twilight Aquamancer4811 | Summon Aqua Guardian8372=>6047,Frostbolt9672,assist flee | Generic interrupts/slow removal and add acquisition;new runner focus. |
| Aqua Guardian6047 | Nova865 | Generic add tanking,magic/root cure. No fabricated wave priority. |
| Twilight Loreseeker4812 | Enlarge8365,Slow18972,assist flee | Generic hostile magic removal/friendly magic cleanse;new runner focus. |
| Twilight Shadowmage4813 | Summon Voidwalker12746=>8996,Bolt9613,Dominate Mind7645,assist flee | Generic interrupts;new local reuse of shared Common/Charmed self restraint,party/pet target release,splash/channel safety and mage/paladin CC containment. New runner focus. |
| Voidwalker Minion8996 | Shadowstrike33914 | Generic tank/add acquisition and healing. |
| Twilight Elementalist4814 | Earth Shock13728,Flame Shock15039,Frost Shock12548,Shock11824,assist flee | Generic legal magic cure/healing;instant shocks are not interrupt casts. New runner focus. |
| Blindlight Murloc4818 | Defensive Stance7164,Sunder7405=>58568,assist flee | Generic tank/heal;new runner focus;armor debuff is not poison/magic. |
| Blindlight Muckdweller4819 | Leech Poison8382,assist flee | Generic poison removal;new runner focus. |
| Blindlight Oracle4820 | Bolt9532,Heal6063,Renew8362,assist flee | Generic interrupts/hostile magic removal;new runner focus. |
| Skittering Crustacean4821 | Assist flee | Generic tank/heal;new runner focus. |
| Snapping Crustacean4822;Aku'mai Fisher4824 | No bound combat AI | Generic tank/heal. |
| Barbed Crustacean4823 | Barbs9464 damage shield,assist flee;summoned form zones combat | Generic tank/heal and add acquisition;new runner focus. No damage-ground escape. |
| Aku'mai Snapjaw4825 | Instant Ravage8391 damage/stun;summon zone combat | Generic tank/heal/recovery;not a runner. |
| Deep Pool Threshfin4827 | Tendon Rip3604 | Generic physical snare recovery/healing;not magic cure. |
| Murkshallow Softshell4977 | Summon zone combat | Generic fire-wave add acquisition/tank/heal. |
| Aku'mai Servant4978 | Spell15043(comment says Volley,but effects are single-target Frostbolt),Nova865;summon zone combat | Generic interrupts/slow/root cure,tank/add acquisition. |
| Argent Guard Thaelrid4787 | Questgiver,no bound combat AI | Generic quest acceptance/turn-in;do not attack. |
| Morridune6729 | Altar summons;waypoints,speech,gossip Teleport9268 | Generic NPC/quest interaction;no automatic teleport or invented combat. |
| Four fires21118–21121 | GossipHello filter1 sets fire data1–4 DONE and synchronously summons groups1 | Separate DC event sequentially uses fire,kills entire matching wave,then confirms scripted door. Ordinary UseGO,not ReportUse.14 total summoned actors:4 Snapjaws,4 Softshells,4 Barbed,2 Servants. |
| Portal21117 | Instance opens only when four fires DONE and required summoned deaths reach0 | Separate DC script-only door policy prevents generic click bypass. Completion gate waits for actual open state. |
| Shrines103015/94039;altar103016 | Gelihast death removes shrine NOT_SELECTABLE;linked trap spell8733;altar summons Morridune | Keep ordinary interaction;see existing core/data limitation below. |
| Fathom Stone177964 | Chest loot/deactivation summons Baron Aquanis | Recursive optional quest content included above. No extra outdoor summon imported. |
| Giant Clam19018,Weapon Crate3689,chests13949/75295/75298 | Loot/lock behavior;no extra summon/AI rows | Preserve generic loot/quest/lock handling. |
| Herbs2045/1622;ores1732/1733 | Gathering locks/loot | Preserve profession AI;not combat mechanics. |
| Campfire21327 | go_flames checks players within0.3yd every3sec and summons a trigger casting SPELL_FLAMES | Small ordinary environmental contact hazard;no fabricated room-wide lava/persistent poison polygon. Existing navigation/avoidance remains;client route acceptance must inspect camp contact. |

Quest audit inspected all11 map/zone719 quest rows:Knowledge in the Deeps,
In Search of Thaelrid,Twilight Falls,both Blackfathom Villainy variants,
Researching the Corruption,Trouble in the Deeps,The Essence of Aku'Mai,
both Allegiance to the Old Gods steps,and Amongst the Ruins. Their loot/kill/
NPC targets share the actors/objects above;Baron and Morridune were recursively
inspected rather than inferred from fixed spawns. Outdoor entrance caves on map1
are outside the instance strategy.

## Implementation contracts

Playerbots.EnableBlackfathomMechanics=0 gates the full feature;all engine hooks
register unconditionally for config reload. Cast danger caches250ms,focus/order
caches500ms;triggers do no direct grid/name/DB scans. GUIDs/numeric data are cached,
never long-lived creature pointers. Escape uses loaded CalcRadius plus3yd,
combines overlapping current applications,and revalidates at Execute. Existing
outward paths survive;failed/blocked requests throttle and preserve casts/movement;
accepted cast-gated PointMovement is handled before spline inspection. Partial
outward5yd-style flee progress is allowed. Roots defer before CastStop. Interrupts,
dispels,healing,recovery keep generic relevance;escape scores ACTION_INTERRUPT-1.
After the application finishes there is no invented remaining cloud to escape.

Focus applies only to nearby DPS and14 verified assist-runner entries. It respects
live explicit skull/priorities through AttackersValue::IsValidTarget,CC,roles,
range/LOS,map/life validity and late action revalidation. Passive assistance
movement is supported;passive fear fallback is excluded. Manual attack and duel
selection survive acquisition suppression. Both focus and hazard multipliers
explicitly preserve RcLeaveCharmedTargetAction so overlapping state cannot prevent
its pet cleanup.

Charm behavior delegates to existing Common/Charmed rather than copying spell
selection. Three local250ms calculated values cache the shared trigger predicates;
actions revalidate through shared Execute. Self/splash/dispelling protections are
locally gated. Existing mage Polymorph/paladin Hammer containment and release remain
conditional on learned spells,cooldown,range,LOS and core checks. This is containment,
not a claim of new explicit magic charm recovery:the shared dispel multiplier
intentionally protects party CC while a member is held. Generic legal dispels remain.
No spells/items are granted by these combat changes.

## Existing core/data limitation

Current IP/live Aku'mai death sets instance data5 DONE,while current core defines
TYPE_AKU_MAI_EVENT=5 and TYPE_AKU_MAI=6. This can leave the altar locked. The core
also serializes only encounter slots0–5. These are observed pre-existing source/data
facts,not corrected by combat AI or the fire event. A coordinated core/IP fix and
separate review is required before claiming dependable altar completion across
reload. The new event never force-opens the portal or edits encounter state.

## Validation and acceptance

Run tests/test_blackfathom.py,tests/test_blackfathom_avoidance.py,
tests/test_blackfathom_charm.py and tests/test_blackfathom_integration.py.
They compile and execute actual encounter source against engine/shared-behavior
doubles. The integration harness compiles the full strategy and verifies
runner+hazard+charm-cleanup overlap. Charm adapter tests prove gating,caching,
registration/names and delegation,not the existing shared implementation's ABI or
all class/core behavior. The focus harness isolates its focus strategy section;
the integration test covers the full actual strategy. Module codestyle,
encounter/fixture clang-format and whitespace checks are required.

No full server build(repository instruction),live pmon profile,loaded-binary
readback or whole-route game-client run. No merge,enablement,DB/config writes,
deployment or restart. Engine doubles do not establish ABI,collision,actual
cast legality,creature visibility,real state persistence or complete route success.

Client/pmon acceptance before enabling:

1. Compare five-minute playerbot pmon stack baseline/enabled representative runs
with BotActiveAlone100 and botActiveAloneSmartScale0;inspect total/encounter cost.
2. Tank/healer/melee/ranged complete every boss and side room;repeat pooled
Lorgus positions,optional Old Serra'kis and Baron present/absent. Inspect submerged
Lady Sarevess approach,water transitions,pets,ledges,campfire contact and LOS.
3. Exercise all14 runners,passive assistance/fear fallback,late skull/manual
priority,stale orders,CC,role/range changes,multiple runners and no assistant.
4. Verify actual Poison Cloud/Sleep cast windows,legal interrupts/dispels and
healing with/without eligible classes;roots,path rejection,cast-gated movement,
overlapping applications and short casts. Do not expect movement to cure landed
poison or interrupt instant Trample/Nova/shocks.
5. Shadowmage controls a bot/player while pets attack;test target/splash/channel
cleanup,containment/release,cooldowns,insufficient mana,missing eligible class,
movement hazard+runner overlap,config reload/map exit and original party orders.
6. Separate DC feature:kill Kelris,use all four fires sequentially,kill all14
summons and confirm real door opens. Exercise manual partial completion,combat
pauses,reload/wipe,unreachable or despawned waves and disabled config. Never
force-open a stalled event. Check all quest objects,shrines,Baron and Morridune;
record altar/persistence limitation separately from route and bot results.

Each implementation PR needs independent exact-final-tree and formal Codex PR
review. Keep unmerged/default-off/undeployed. After explicit merge authorization,
prepare separately reviewed wow-server pins to actual resulting reachable commits.
Other unmerged dungeon PRs are design references,not dependencies.
