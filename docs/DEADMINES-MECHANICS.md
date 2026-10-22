# Deadmines mechanics review

October 21, 2026. Scope: the complete Classic map 36 instance, including every
boss and rare, both Sneed phases, trash, spell summons, cannon adds and route events. The outside
Westfall mine is map 0 and is outside this instance strategy.

Source review and implementation are complete for the coverage described below;
this is not certification of live encounter behavior. Nothing is enabled by this PR.

## Source basis

Fetched Playerbots evermore `a0de40f17f4fe01af8a7dda95bedc61bde08e6c0`;
core evermore `45a31ad06fffc4662ba8e0b10ce8337b83d2ffeb`;
Dungeon Clear evermore `b8547a0c828f77072f02362d0c73d73028ae3750`.
Authenticated wow-server main manifest records that core and Dungeon Clear,
Playerbots `907e90213b8248108eaf94b7491bff62bf3a024e` and Individual Progression
`46ab14dce6be2b6c83435220a5f190573f516bf4`. The IP Deadmines SQL at that pin is
identical to fetched main `70c49ec8c0d254c4a184e2b021e23235c52d7775`.
These are source/pin facts, not readback of a running worldserver.

References:

- [Core SmartAI](https://github.com/buildthehomelab/wow-azerothcore/blob/45a31ad06fffc4662ba8e0b10ce8337b83d2ffeb/data/sql/base/db_world/smart_scripts.sql), map-36 creature spawns/templates and linked cannon action list.
- [Smite script](https://github.com/buildthehomelab/wow-azerothcore/blob/45a31ad06fffc4662ba8e0b10ce8337b83d2ffeb/src/server/scripts/EasternKingdoms/Deadmines/boss_mr_smite.cpp) and instance door/save script.
- [IP replacements](https://github.com/buildthehomelab/wow-mod-individual-progression/blob/46ab14dce6be2b6c83435220a5f190573f516bf4/data/sql/world/base/dungeon_deadmines.sql).
- [Dungeon Clear events/roster](https://github.com/buildthehomelab/wow-mod-dungeon-clear/blob/b8547a0c828f77072f02362d0c73d73028ae3750/src/Ai/Dungeon/DungeonClear/Data/Events/DeadminesEvents.cpp).

`DEADMINES-ACTORS.csv` inventories 26 actors from 213 fixed spawns plus verified
spell summons (Sneed 643, Blackguard 636, Golem 2520, Companion 3450) and cannon
Squallshapers. It separates base spell IDs from effective IP replacements.
Behavior text preserves source comments; spell IDs and effect data take precedence.
Cannon GO 16398 and its linked action list are reviewed separately from actors.
A name-based pending-core-update screen found no additional Deadmines rows; this
is not an exhaustive SQL migration simulator or a query of live overrides.

Spell effects were read from the saved Evermore z80 candidate Spell.dbc previously
used for client-patch work. That file is a reference snapshot, not the loaded realm
DBC. Verified effects include 11829 persistent-area periodic damage; 6306 caster-area
poison; 5159 caster-area periodic damage/slow; 5213 single-target periodic damage/slow;
5174 self-heal; 6432 instant caster-area stun; 5141/5200/3605 summons of 643/636/2520.
The runtime implementation reads `SpellEffectInfo::CalcRadius(caster)` and the live
DynamicObject radius. It does not interpret a DBC radius index as yards or guess a
fixed cast duration. Short casts can complete before a bot reacts.

## Enablement and processing

`Playerbots.EnableDeadminesMechanics = 1` opts into this strategy. Default is 0.
Set before instance strategy initialization; ordinary ApplyInstanceStrategies must
also be enabled. Map entry/exit adds/removes `vanilla-deadmines` in balance.

Hazard state caches for 250 ms. It examines active aura categories for actionable
negative dynamic areas, then the existing cached attackers for active
6306/5159 casts. Focus caches a GUID for 500 ms from the existing attackers value.
Triggers/multipliers read caches and bounded live state; no names/grid searches,
synchronous DB queries, raw cross-tick Unit pointers or persistent kill markers are added.
Focus actions revalidate CC assignments, LOS, range, combat, roles and explicit
skull/prioritized orders. Tanks/healers retain their normal targeting. Selection
is limited to current spell range for ranged DPS and 12 yards for melee DPS.
Once selected, ordinary class movement still governs pursuit; this is not a new
hard leash on the existing combat engine.

Escape cancels the bot's cast, clears an old movement wait, and uses the existing
FleePosition engine. It preserves its in-progress spline when its destination is
outside the enclosing circle of currently overlapping hazards, retries failed/replacement paths at most twice per
second, and blocks conflicting movement/spells while cached danger persists.
New destinations inside that enclosing circle are stopped rather than left running.
Hazard release can lag up to 250 ms. Cast avoidance uses loaded radius plus three
yards; ground avoidance uses loaded area radius and normal FleePosition clearance.
Existing generic AvoidAoe behavior remains available for human-led groups. The
new local action covers autonomous groups too and skips non-actionable overlapping
auras rather than letting the generic first-area value mask another hazard.

## Bosses and rare

| Actor | Source mechanics and response |
| --- | --- |
| Rhahk'Zor 644 | Slam 6304 is an instant target hit/stun. Keep tank acquisition, healing and ordinary positioning; no fictitious telegraphed AoE escape. Death opens the factory door and sets instance data. |
| Miner Johnson 3586 | Pierce Armor 12097; retain tank/healing. Base/IP initialization can despawn the rare in normal difficulty; no guarantee that a run contains him. |
| Sneed's Shredder 642 | Terrify 7399 is magic fear; available learned dispels/fear breaks remain generic. Distracting Pain 3603 is not magic-dispellable in the reference DBC. Death casts Eject Sneed 5141. |
| Sneed 643 | Disarm 6713; retain existing role/class responses and target reacquisition after Shredder dies. DC already anchors the encounter on Shredder because Sneed lacks a fixed spawn. Sneed's death opens the mast-room door. |
| Gilnid 1763 | Molten Metal 5213 is a single-target DoT/slow with no ordinary magic/poison dispel type. Heal through it and interrupt where legal/available; do not invent a floor cloud or automatic dispel. Death opens the foundry door. |
| Mr. Smite 646 | Script casts instant Stomp 6432 at 67/34%, pacifies/moves him to the chest, swaps weapons, then resumes combat. Preserve generic stun recovery, healing, melee positioning and add pickup; no reactive dodge of an instant stomp. Third-phase Slam 6435 targets the victim. |
| Cookie 645 | New escape attempt during caster-area Acid Splash 6306, retaining generic poison removal for applications already present. Cooking 5174 is a self-heal: existing current-target/nearby-healer interrupt machinery handles eligible learned interrupts. IP also gives Cookie a 15% flee response; local runner focus applies when eligible. |
| Captain Greenskin 647 | Cleave 40505: existing tank-facing/melee-behind behavior. Poisoned Harpoon 5208: available poison removal/healing, interrupts if legal. No invented ground hazard. |
| Edwin VanCleef 639 | IP adds Thrash 3391 and summons Blackguards with 5200 at 50%. New nearby DPS focus favors an engaged Blackguard attacking this bot or another non-tank party member; tank retains boss/add acquisition. CC and manual priorities win. No unconditional switch to unengaged/hidden adds. |

## Trash, adds, events and hazards

| Actors / event | Coverage |
| --- | --- |
| Miner 598, Strip Miner 4416 | Pierce Armor, 15% assist runners. Existing tank/healing and class snares/stuns; new nearby DPS runner focus. |
| Overseer 634 | IP replaces base 6016 with Battle Command 5115; existing eligible hostile dispel and assist-runner response. Do not infer spell behavior from base comment alone. |
| Watchman 1725, Taskmaster 4417 | Shoot; Taskmaster Piercing Shot 6685. Generic ranged engagement/tank pickup; Watchman has an IP runner response. Existing CC/interrupt legality still applies. |
| Evoker 1729, Wizard 4418 | Existing Fireball/Chains of Ice interrupts and magic removal when learned/eligible. Frost Armor/Quick Flame Ward remain generic offensive-dispel targets. New actionable dynamic-floor avoidance covers Evoker Flamestrike 11829 for autonomous groups. |
| Woodcarver 641 | Axe Toss stun and Cleave: generic recovery/healing, tank positioning and melee-behind. IP source has no Woodcarver flee row; do not infer one from base. |
| Engineer 622, Remote-Controlled Golem 2520 | Summon 3605 resolves to 2520. Existing summon interrupt and tank/add acquisition; Engineer assist-runner focus. No unsupported golem explosion response is added. |
| Craftsman 1731 | New cast escape for caster-area Melt Ore 5159, plus existing healing and eligible interrupts. IP assist runner receives local focus. |
| Shipbuilder 3947 | Assist runner; generic CC/snares and new local focus. |
| Pirate 657, Companion 3450 | Companion created by 5172; existing pack/add acquisition, tank threat and class damage. No fabricated pirate spell rotation. |
| Blackguard 636 | Faded/Cheap Shot/Rupture source rows retained. New DPS protection focus when an engaged guard attacks a non-tank party member. Bleed is not called an ordinary magic dispel. |
| Squallshaper 1732 | Fire Blast, Frost Armor, assist runner. The row labelled Frost Nova actually casts 4979 Quick Flame Ward, so no fictitious root-specific mechanic is added. Existing purge/CC/interrupt machinery and local runner focus remain. |
| Rat 4075 | Critter explicitly inventoried; no encounter action. |
| Cannon / Iron Clad Door | Existing DC persistent anchored event grants/uses powder through the item-use hook, waits for the door, and preserves progress across summoned-add combat. Core GO SmartAI calls list 1639800, opens the door, summons two Squallshapers, sets cannon instance data. Retain this implementation. |
| Patrols / pack CC | Existing DC route/pulls and generic class CC remain authoritative. Focus excludes breakable CC and group CC assignments, and rechecks them at execution. Real fleeing movement states receive priority; feared/CC-marked mobs are excluded. |
| Foundry / ship geometry | Normal collision/pathfinding and routes remain. No source-confirmed additional persistent floor hazard is invented. Tight walkways, overlapping areas, roots and obstructed escapes need client tests. |

Existing generic capabilities are conditional on class, learned spell rank, resources,
range, LOS, immunity and cast legality. Low-level parties may lack poison/magic removal
or interrupts; this PR does not grant spells or duplicate rotations. Ordinary AoE can
still interfere with CC according to existing class policies; this pass preserves those
policies rather than claiming a global CC-safety rewrite.

## Verification and live acceptance

`python3 tests/test_deadmines.py` compiles/executes the actual new encounter sources
with engine doubles. It covers defaults/wiring, local runner/Blackguard priority,
tank/healer/manual exemptions, late CC/range/LOS, runtime cast radii, targeted-DoT
exclusion, ignored/oversized overlapping ground areas, post-combat floor danger,
escape preemption/spline preservation/retry throttling and multiplier release.
Module codestyle, touched encounter/fixture clang-format and whitespace checks are
required. Doubles do not establish core ABI, cast legality, collision, latency or DPS.

No full server build was run under the repository's build restriction. No live DB/DBC
query, pmon measurement, game-client run, deployment, enablement or restart was done.
Independent and formal final-revision reviews are tracked on the PR. The unmerged SM
PR #19 is context only; this implementation does not depend on its branch.

Before deployment, run the whole route with level-appropriate tank, healer, melee,
ranged and pets, both a human-led and fully autonomous party. Check every boss/rare
when available, Shredder-to-Sneed reacquisition, Smite's two swaps, VanCleef adds on
healer/tank, caster packs, CC marks assigned after cache selection, local runners,
Cookie poison/heal, Craftsman casts, Evoker floor areas, cannon re-entry after wipe,
door/save recovery and ship walkways. Repeat blocked/rooted/overlapping hazards and
switch-off baseline. Pets have no new dedicated hazard recall; existing poison/DoT
applications are not removed by movement. Currently overlapping actionable areas are enclosed together; unseen areas outside
the bot's active auras and future hazards still depend on later decisions. This is
not a global safe-path solver.

Measure per-bot cost with BotActiveAlone=100, botActiveAloneSmartScale=0;
`playerbot pmon toggle` after login, then `playerbot pmon stack` five minutes later;
compare the final Total line against baseline. Merge only after required reviews
and user authorization. After an authorized merge, prepare a separate reviewed
wow-server manifest PR for the actual reachable merged commit. No proposed branch
SHA is a live deployment fact.
