# The Stockade: complete instance mechanics audit

Audit: October 22, 2026. Scope is map 34, not the outdoor Stormwind prison riot.
This is source/readonly-data coverage, not certification of a completed bot run.

## Evidence and version boundaries

Authenticated deployment manifest readback and freshly fetched sources:

| Component | Revision |
| --- | --- |
| Playerbots evermore, implementation base | `22e730d480d5b29ece25ad7aafa0bbb58d46fe4d` |
| Evermore core | `fccbd096804b33d71a36d7a3920f1b3256ff8cdd` |
| Individual Progression main | `188f697a79a8d129816fd892bd44df962228d676` |
| Dungeon Clear evermore | `c3722424ebe3bdf1d772a62c8ec7d3e3a3368bcd` |

References: [core instance script](https://github.com/buildthehomelab/wow-azerothcore/blob/fccbd096804b33d71a36d7a3920f1b3256ff8cdd/src/server/scripts/EasternKingdoms/TheStockade/instance_the_stockade.cpp),
[progression replacement](https://github.com/buildthehomelab/wow-mod-individual-progression/blob/188f697a79a8d129816fd892bd44df962228d676/data/sql/world/base/dungeon_stockade.sql),
[core base SmartAI](https://github.com/buildthehomelab/wow-azerothcore/blob/fccbd096804b33d71a36d7a3920f1b3256ff8cdd/data/sql/base/db_world/smart_scripts.sql),
[DC pull planner](https://github.com/buildthehomelab/wow-mod-dungeon-clear/blob/c3722424ebe3bdf1d772a62c8ec7d3e3a3368bcd/src/Ai/Dungeon/DungeonClear/Util/DcPullPlanner.cpp).

Readonly live world queries returned 93 fixed spawns, 11 creature templates,
18 objects across seven object templates, and no object SmartAI or summon-group
rows for the reviewed owners. All 11 actors bind SmartAI; the instance binds
`instance_the_stockade`, whose script has no encounter/door state machine.
The selected live inventory contains 49 creature rows and 11 prisoner action-list
rows. Five unrelated Sergeant Kan'ren list rows from the broad numeric query are
excluded. See `STOCKADE-ACTORS.csv` for every actor, spawn count and base/live spell IDs.

Spell effects were read from the saved Evermore z80 reference MPQ's Spell.dbc;
this is not a readback of the DBC loaded by worldserver. No live spell_dbc overrides
were found for observed spells or Ward's triggered fear. Base SQL alone is not
authoritative: progression adds Targorr Thrash, additional trash Rend/Backhand,
and replaces Inmate 6547 with Rend 11977. Pending-update filename screening
found no Stockade-named update; this is not exhaustive SQL migration replay.
No running-server binary revision was established by these queries.

## Complete response matrix

| Actor/content | Actual mechanics | Response ownership and decision |
| --- | --- | --- |
| Dextren Ward 1663 | Battle Stance 7165; instant Frightening Shout 19134; Strike 11976; assist flee below 15% | Generic tank/healing and legal fear recovery. Shout applies fear/root and triggers fear 29544; dispel type 0, so do not invent friendly magic dispel or interrupt. Existing Fear Ward, berserker rage, racial and selected tremor strategy remain conditional. New nearby DPS runner focus. |
| Kam Deepfury 1666 | Defensive Stance 7164; Shield Slam 8242 damage/stun; Improved Blocking 3419; assist flee | Tank holds frontal melee, generic healing/recovery. Instant Slam is not a cast to interrupt. New runner focus. |
| Targorr the Dread 1696 | Dual Wield 674; Thrash 3391; Enrage 8599 below 30% | Tank/heal; generic hunter enrage tranquilizing shot when learned/usable. No assist-flee row; explicitly excluded from runner whitelist. |
| Bazil Thredd 1716 | Dual Wield; Smoke Bomb 7964; Battle Shout 9128; assist flee | Smoke Bomb is instant magic stun (aura 12), not a dynamic damage floor. Generic friendly magic dispels/recovery; no floor escape or cast avoidance added. New runner focus. |
| Hamhock 1717 | Chain Lightning 421; Bloodlust 6742 | Preserve generic current-target/healer-target interrupts and hostile magic dispel/purge/spellsteal/learned tranquilizing shot. Maintain ordinary ranged/healer spacing and LOS; no invented floor or mandatory stack point. No assist flee. |
| Bruegal Ironknuckle 1720 (optional rare) | 70% AI-init despawn; melee; assist flee | Spawn randomness is server-owned. Tank/heal; new runner focus if actually present and fleeing. No invented stun/cleave phases. |
| Defias Prisoner 1706 (13 spawns) | Casting-victim Kick 1766; Disarm 6713; assist flee | Existing tank/heal, recovery and spell selection; no speculative kick-bait or cast cancellation. New runner focus. Map-restricted outdoor list discussed below. |
| Defias Captive 1707 (19) | Behind-target Backstab 7159; Infected Wound 3427 (disease); Rend 11977; assist flee | Tank faces attackers; generic disease cures when learned/usable; ordinary healing for bleed. New runner focus, preserving CC. |
| Defias Inmate 1708 (33) | Battle Stance; Rend 11977; assist flee | Ordinary tank/heal, physical bleed is not dispellable poison/magic. New runner focus. |
| Defias Convict 1711 (12) | Dual Wield; Backhand 6253 stun; Rend; Demoralizing Shout 13730; assist flee | Ordinary tank/heal/recovery; no cast/floor dodge. New runner focus. |
| Defias Insurgent 1715 (10) | Battle Shout; Demoralizing Shout; Backhand; assist flee | Ordinary tank/heal/recovery; new runner focus. |
| Summons/adds | No verified map-34 combat summon action/spell among these actors | Acquire engaged adds through generic threat/attacker AI; no fabricated priority add. |
| Prisoner list 170600 / guard 37063 | Four Stormwind City Guard creature summons, follow/data/equipment/path and despawn logic | Prisoner initiating event is map-0 restricted (respawn type 1, map parameter 0; flag 512 permits charmed events); do not activate outdoor riot in instance. Guard AI was recursively checked. Creature ID 37063 is not spell 37063 Void Zone. |
| Objects/optional loot | Large Solid Chest 75298; Large Iron Bound Chest 75295; five seasonal decoration templates | Chest Data7 event is zero, no bound AI/scripts; no summoning encounter or door puzzle. Leave normal loot/locks to existing AI. Decoration templates have no mechanic. |
| Dungeon navigation/events | Empty core instance script; DC route sweep and advanced pull planning for adjacent cells | Preserve existing route/pull ownership. No custom route, boss activation, quest interaction, wall polygon or door action. |

There are no other fixed actor types in live map 34. Seasonal decorations do not
establish a seasonal boss. Quest kill objectives share these actors; no separately
verified optional map-34 quest-summoned combat actor was found in the reviewed
spell, object and summon-group data. This is bounded to the inspected current
source and readonly world data, not an assertion about arbitrary future scripts.

Generic class response references: `GenericTriggers.cpp` interrupt checks;
`PlayerbotAI::IsInterruptableSpellCasting` and `HasAuraToDispel`;
`GenericHunterStrategy.cpp` enrage/magic tranquilizing shot;
warrior, priest, shaman, mage, paladin, druid and racial strategies. Spell rank,
level, cooldown, immunity, range, LOS and active strategy decide availability.
No spells are granted and no duplicate class rotations are introduced.

## Implemented confirmed gap

`Playerbots.EnableStockadeMechanics = 0` gates the complete feature. Enabled map-34
DPS bots select an engaged, nearby, legal assist runner from the existing cached
attackers list. The whitelist is the nine actors with verified action 25. Numeric
GUID focus caches refresh every 500ms; triggers do no grid/name/DB scans. Late
CC, role, explicit skull/prioritized orders, alive/world/map/combat validity, LOS
and melee 12-yard or normal ranged spell range are revalidated by the action.
Explicit skull/stored priorities block only while the resolved target satisfies
AttackersValue::IsValidTarget. A 500ms bool cache keeps priority-list iteration out
of triggers; actions recheck immediately. Dead/despawned/stale GUIDs do not disable
focus for the remainder of a fight.
Rejected CC focus invalidates the cache so other attack acquisition is released.

Tanks retain aggro work; healers retain healing. The multiplier suppresses only
competing automatic AttackAction acquisition, allowing manual attack commands to
record their priority, duel selection, melee, class spells, interrupts,
dispels, recovery and healing. Assistance movement may be passive; passive fear
fallback is excluded. No new snare/CC spell selection or movement planner is added.
The strategy is added and removed through ApplyInstanceStrategies for map 34.
Hooks register unconditionally so config reload works on existing engines.
Runtime config gates keep disabled hooks inert; default-off setup adds normal
registration and cheap gated checks, with no candidate scans.

References to other dungeon PRs are design context, not dependencies. The local
WC runner implementation supplies the same GPLv2 focus pattern; this branch
contains its own implementation and does not require WC, RFC, SFK or Deadmines
PRs to merge.

## Validation and acceptance

Run `python3 tests/test_stockade.py`, module codestyle and clang-format dry-run on
new encounter/fixture C++. The harness compiles and executes actual source against
engine doubles: all nine allowed entries, excluded Hamhock/Targorr/guard, toggle,
map, roles, explicit orders (including unsuppressed manual attack before its priority
is recorded), late CC/range/LOS/validity changes, passive assistant
movement and generic attack/melee/spell priority preservation. This checks behavior
contracts, not core ABI, server collision or real cast legality.

No full core/module build (repository policy), pmon profile or game-client route
has been performed. No enablement, merge, SQL/config writes, deployment or restart.

Before enablement, compare a five-minute `playerbot pmon stack` baseline with
`BotActiveAlone=100`, `botActiveAloneSmartScale=0`, monitoring after bots log in.
Repeat enabled for a representative Stockade party; verify no new map-thread DB
work or scans and inspect total/encounter costs. Whole-route client acceptance:

1. Tank/healer/melee/ranged party completes both wings and all six named actors
   when rare is present; repeat rare absent. Confirm existing cell pulls do not
   acquire neighboring rooms through new chase behavior.
2. Trigger all nine assist-runner types. DPS focus only within legal range/LOS,
   leave tank/healer targets intact, stop competing acquisitions, preserve CC and
   obey late skull/prioritized commands. Verify live attackers cache sees passive
   assistance movement. Test no assistant, fear fallback, death and despawn.
3. Observe Hamhock interrupt and hostile buff removal with eligible classes;
   repeat without eligible class/rank. Test disease cure, Smoke Bomb stun removal,
   Ward fear recovery/prevention, Targorr enrage removal and ordinary healing.
4. Check roots/stuns/LOS failures, narrow cells, leash, pets, multi-runner packs,
   range changes, manual control, map exit and config reload. No new pet mechanic
   is supplied; conditional class responses may be unavailable at dungeon level.
5. Check chest interaction and seasonal decorations; no outdoor riot activation.

Implementation PR must complete independent exact-revision review and formal
Codex PR review. Keep unmerged until explicit authorization. After an authorized
merge, resolve its actual reachable evermore commit and prepare a separately
reviewed wow-server module-pin PR. A proposed branch SHA is not a deployment pin.
