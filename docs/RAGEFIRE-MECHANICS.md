# Ragefire Chasm mechanics audit

October 22, 2026. Complete map 389 source and readonly world-data coverage:
14 creature actors (132 fixed spawns plus summoned Voidwalker 8996 and quest summon Zelemar 17830), all four
bosses, trash, adds, quest NPC, objects, events, role positioning, interrupts,
dispels, CC, runners and hazards. No rare actor exists in the verified spawn
inventory. Source/data coverage is not gameplay certification.

## Evidence

Fetched Playerbots evermore `a8c7980054fc56d47942360ad086fe758b432295`, core
evermore `fccbd096804b33d71a36d7a3920f1b3256ff8cdd`, Individual Progression main
`70c49ec8c0d254c4a184e2b021e23235c52d7775` and Dungeon Clear evermore
`34533610d73e1dee404a0f826439b89246b9bca4`. Authenticated wow-server main
manifests record these same four pins. Selected core/IP/Playerbots files were
also authenticated through GitHub after source fetches.

References:

- [Core world data](https://github.com/buildthehomelab/wow-azerothcore/tree/fccbd096804b33d71a36d7a3920f1b3256ff8cdd/data/sql/base/db_world): creature/templates, SmartAI, objects/templates.
- [Instance script](https://github.com/buildthehomelab/wow-azerothcore/blob/fccbd096804b33d71a36d7a3920f1b3256ff8cdd/src/server/scripts/Kalimdor/RagefireChasm/instance_ragefire_chasm.cpp): empty InstanceScript, no boss phase/door/summoning state machine.
- [Progression replacements](https://github.com/buildthehomelab/wow-mod-individual-progression/blob/70c49ec8c0d254c4a184e2b021e23235c52d7775/data/sql/world/base/dungeon_ragefire_chasm.sql).
- [Generic interrupts](https://github.com/buildthehomelab/wow-mod-playerbots/blob/a8c7980054fc56d47942360ad086fe758b432295/src/Ai/Base/Value/EnemyHealerTargetValue.cpp): off-target healing casts, then in-range harmful casts for true interrupts/silences.
- [Dungeon Clear navigation](https://github.com/buildthehomelab/wow-mod-dungeon-clear/blob/34533610d73e1dee404a0f826439b89246b9bca4/src/Ai/Dungeon/DungeonClear/Data/DcNavPenaltyRegistry.cpp): existing three-leg RFC funnel wall.

Readonly realm queries returned 132 spawns, 14 selected templates, 18 creature SmartAI
rows, one GO SmartAI row, 14 objects and zero spell_dbc overrides for the observed actor cast IDs.
Live SmartAI matches progression behavior, including percentage-health Shaman
healing and both runners. Summon 12746 creates Voidwalker 8996, whose live/base
AI casts Shadowstrike 33914. RAGEFIRE-ACTORS.csv records live spell IDs, behaviors
and fixed counts. Raw query output/source snapshots remain in the workspace
artifact. No DB writes, config changes or server commands were performed.

Spell effects were read from the saved Evermore z80 Spell.dbc reference archive,
using current core SpellEntry field layout and a core SpellMgr spell-ID screen.
This is not readback of the worldserver's loaded DBC. Fire Nova 11970 has cast
index 5, damage effect 2, caster-area targets 22/15, radius index 13 and
InterruptFlags 9. Runtime geometry uses CalcRadius(enemy), never indexes as yards.
No guessed cast duration or guaranteed interrupt eligibility is added.

## Coverage

| Actor / feature | Verified mechanic and response |
| --- | --- |
| Oggleflint 11517 | Instant Cleave 40505; normal tank acquisition and generic melee behind positioning. No fictitious cast dodge. |
| Taragaman 11520 | Uppercut 18072 is a victim hit/knockback; ordinary tank/healer recovery and environmental positioning. Casted area Fire Nova 11970 receives a new escape attempt, preserving generic interrupts. |
| Jergosh 11518 | Curse of Weakness 18267 (curse), Immolate 20800 (magic DoT/direct hit). Generic legal curse/magic removal and interrupts. Neither creates a persistent floor. |
| Bazzalan 11519 | Poison 744 (poison dispel), Sinister Strike 14873. Generic tank/heal/cleansing. Live Poison differs from base Deadly Poison. |
| Trogg 11318 | Strike 11976; ordinary tank/DPS/heal. |
| Shaman 11319 | Lightning Bolt 9532, Healing Wave 11986, assist flee at 15%. Existing current/off-target interrupts and snares; new nearby DPS focus while actually fleeing. |
| Earthborer 11320 | Acid 18070 is targeted armor reduction (aura 22, misc 1, no dispel type), not a poison cloud. Preserve tank/heal response. |
| Molten Elemental 11321 | No SmartAI or bound script in live template; ordinary combat, no invented explosion. |
| Cultist 11322 | Curse of Agony 18266; generic curse removal. |
| Enforcer 11323 | Shield Slam 8242 includes target stun/direct hit. Generic stun recovery/healing; no area dodge. |
| Warlock 11324 | Shadow Bolt 20791, summon 12746, assist flee at 15%. Generic interrupts, add acquisition and CC; new nearby runner focus. |
| Voidwalker 8996 | Summoned guardian, Shadowstrike 33914; ordinary add pickup/healing, no kill-order override. |
| Maur Grimtotem 11834 | Noncombat quest corpse, no combat script; preserve existing quest interaction. |
| Blood Filled Orb 182024 / Zelemar 17830 | Live/base GO gossip SmartAI summons Zelemar at the orb. Live creature template has no bound combat script or SmartAI. Preserve player quest interaction, ordinary tank pickup/combat; do not auto-activate the orb. |
| Objects/events | Other objects are braziers/signs. No gauntlet/cannon/phase event in instance script. Preserve DC orchestration/navigation and quest interactions. |
| Lava/ledges | Existing navigation and DC funnel wall retained. No new hardcoded lava polygons or claim of whole-route lava safety. Collision/path validation remains required. |
| Roles/CC | Retain class snares/CC, behind movement, ranged spacing, tank acquisition and healer positioning. Focus respects active/assigned CC, role, skull/prioritized orders and current range/LOS. No new class spell selection. |

## Behavior and costs

Default-off Playerbots.EnableRagefireMechanics opts into vanilla-ragefire with
normal ApplyInstanceStrategies enabled. Set before instance initialization.
Map entry/exit adds/removes the local strategy in balance.

A 250 ms cache scans existing cached attackers for active numeric Fire Nova IDs,
using loaded radius plus three yards. Concurrent cast areas are enclosed together.
No name/grid scan, synchronous DB query, persistent kill state or cross-tick raw
Unit pointer. No floor scan is added because no verified RFC spell creates a
harmful persistent dynamic area.

A 500 ms GUID cache selects actual nearby runners. Tanks/healers keep ordinary
targets; DPS selection is within spell range (ranged) or 12 yards (melee). Actions
revalidate combat/alive/passive state, manual orders, LOS, range and late CC marks;
rejected CC focus clears its cache. Existing generic pursuit still governs
movement after selection; this is not a global chase leash or snare replacement.

Escape is below interrupt relevance and above ordinary movement. Competing
movement/target switches are suppressed; all generic spell priorities remain,
including interrupts, dispels and emergency healing. Roots/control defer before
CastStop. Outward escape splines are preserved; capped FleeDistance steps chain,
nonprogressing endpoints stop, failed requests throttle to 500 ms. Short Nova
casts may finish before escape; higher generic responses may delay escape. Pets
have no new dedicated hazard recall. Existing route pathfinding governs lava.

## Validation and acceptance

python3 tests/test_ragefire.py compiles/executes actual encounter sources against
engine doubles: switch/map gates, creator wiring, role/manual/CC exclusions,
late range/LOS/CC, runner start/stop, cast ID/radius/end, interrupt/spell priority
preservation, root recovery, failed-path throttle, spline preservation and three
chained five-yard steps from a twelve-yard objective. Doubles do not certify ABI,
cast legality, collision or gameplay. Codestyle, encounter/fixture clang-format,
whitespace and wiring checks are required; reviews are tracked on the PR.
Independent review's generic-interrupt suppression finding was fixed by preserving
spell multipliers and yielding escape relevance. Deadmines #21 remains open at
ee6392f46d as reference, not a dependency.

No full server build (repository restriction), pmon profile or client run was
performed. Run the entire route at appropriate levels with tank/healer/melee/
ranged/pets in human-led and autonomous groups. Verify all four bosses, every
trash family, quest corpse, Shaman healing/off-target interrupts, Warlock pets,
actual runners and late CC, legal dispels, Nova interrupt/escape/failed escape,
Uppercut near ledges, lava, roots/blocked paths, overlapping casts, wipe/re-entry
and switch-off baseline. Measure BotActiveAlone=100, botActiveAloneSmartScale=0:
pmon toggle after login, pmon stack five minutes later; compare final Total.

Keep unmerged/disabled until authorized. After an authorized implementation merge,
prepare a separate reviewed wow-server PR pinning the actual reachable merge
commit. No proposed branch SHA establishes deployment or loaded server code.
