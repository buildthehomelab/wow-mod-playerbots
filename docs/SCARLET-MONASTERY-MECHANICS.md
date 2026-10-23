# Scarlet Monastery mechanics audit and implementation

October 21, 2026. Playerbots base: `907e90213b8248108eaf94b7491bff62bf3a024e`.
Core source: `45a31ad06fffc4662ba8e0b10ce8337b83d2ffeb`.
Dungeon Clear source inspected: `b8547a0` on evermore.
This is a source audit and implementation, not certification of a running realm.

## Enablement and cost

`Playerbots.EnableScarletMonasteryMechanics = 1` opts into the full pass in map 189.
It includes Herod avoidance; `EnableHerodWhirlwindAvoidance` remains a separate
Herod-only switch. Both default off. Set before startup/rebuilding instance strategies.
Existing Doan avoidance remains active, with the movement/cache corrections below.

A shared 250 ms GUID cache scans the bot's threat references once for all four
hazards, rather than rescanning Unicode names per trigger/multiplier. Focus selection
runs at most every 500 ms from the existing attacker cache. Charm and harmful floor
state use 250 ms caches. Live aura/channel/validity checks release behavior promptly.
Ground detection scans until the first actionable negative dynamic area, so an
ignored overlapping aura cannot mask it; the escape uses the selected geometry.
Cached targets store GUIDs, not raw pointers. Action execution revalidates targets.
No synchronous queries, new dialogue, SQL, DBC or addon changes are introduced.
When the full switch is off, the additional focus/charm/floor triggers and multipliers
are not registered; encounter values are created on demand.

## Bosses and rares

| Wing / actor | Mechanic | Coverage after this change |
| --- | --- | --- |
| Graveyard — Vishas | Shadow Word: Pain | Existing class magic dispels; no duplicate encounter action. |
| Graveyard — Thalnos | Shadow Bolt, Flame Shock, Flame Spike | Existing interrupts/magic dispels and harmful floor avoidance when represented by a dynamic object. |
| Graveyard — Thalnos | Fire Nova 12470 | New cast response: stop casting and attempt escape to 14 yards. Release on cast completion/interruption. |
| Graveyard — Fallen Champion | Cleave, Strike, Rend | Existing melee-behind/tank-facing and healing. Bleed is not presented as a removable magic debuff. |
| Graveyard — Ironspine | Curse of Weakness | Existing curse dispels. |
| Graveyard — Ironspine | Poison Cloud 3815 | New cast response to 12 yards; existing poison removal handles applications. This applies an area poison; do not invent a persistent ground cloud or hold all DPS for its full 15-second debuff. |
| Graveyard — Azshir | Call of the Grave, Terrify, Soul Siphon | Existing interrupts where legal, healing and available fear/CC breaks. No verified missing dedicated decision. |
| Graveyard — Scorn | Frost Nova, Frostbolt Volley, Mind Flay, Lich Slap | Existing interrupt/magic-dispel/role positioning; summoned-event availability remains a server concern. |
| Library — Loksey | Hounds and Bloodlust | Existing tank/add acquisition and hostile magic removal when the class/spell permits it. |
| Library — Doan | Polymorph, Silence, Arcane Explosion | Existing magic dispels and class positioning. Silence recovery depends on available non-silenced party members. |
| Library — Doan | Arcane Bubble / Detonation | Existing escape now uses the GUID cache, preempts an old combat approach and preserves an already-issued escape spline. Existing 36-yard boundary retained. |
| Armory — Herod | Whirlwind | New aura/channel response: every role escapes to 12 yards, waits out the channel and resumes movement afterward. |
| Armory — Herod | Cleave, Enrage, trainee wave | Existing tank-facing/melee-behind, healing and add acquisition. Enrage is not treated as an ordinary magic buff. |
| Cathedral — Mograine/Whitemane | Room pre-clear, initial pull, fake death and resurrection | Dungeon Clear retains its existing pre-clear/roster/event orchestration. Focus only selects a valid engaged enemy and excludes passive resurrection behavior. Tanks retain their own acquisition logic. |
| Cathedral — Whitemane | Heal, Shield, Holy Smite | Existing interrupts/purge; new nearby DPS focus favors her when active, including after resurrection. |
| Cathedral — Whitemane | Dominate Mind 14515 | New core-eligible Dispel Magic/Purge targeting for dominated party members. Stop bot/pet attacks and harmful casts into them; exclude them from normal damage targeting and suppress harmful AoE while charm is present. Dispels remain allowed. |
| Cathedral — Fairbanks | Heal, Shield, Curse of Blood | Existing interrupts/purge/curse dispels; included in nearby healer focus. |

Fire Nova is a two-second, ten-yard spell and Poison Cloud has an eight-yard area
with a one-second cast in the external spell references. The chosen distances add
four yards. Escape is an attempt, not immunity: latency, roots, a late AI reaction or
an obstructed path can still cause damage. Source references:
[Fire Nova](https://www.wowhead.com/spell=12470/fire-nova),
[Poison Cloud](https://www.wowhead.com/spell=3815/poison-cloud),
[Whirlwind damage](https://www.wowhead.com/wotlk/spell=15578/whirlwind).
Realm spell overrides have not been verified.

## Trash and navigation

The accompanying `SCARLET-MONASTERY-ACTORS.csv` screens 41 actors: fixed map-189
spawns plus named rare/event/summoned actors. It records 287 relevant SmartAI rows
and timed action-list rows. It includes critters/friendly actors to keep the scope
explicit; it is not an exhaustive resolver of every spell-created summon.

- **Protectors, Adepts, Chaplains and Abbots:** new deterministic nearby DPS healer
  focus, with Whitemane taking precedence. Existing interrupts and purges remain.
- **Evokers, Sorcerers, Diviners, Conjurors and Wizards:** existing cast interrupts;
  new opt-in harmful dynamic-area detection and existing flee-position movement also work
  for autonomous groups. The core Sorcerer's row labelled “Blizzard” currently casts
  spell 9672, also used by its Frostbolt row. Labels alone do not establish a Blizzard
  hazard; this pass does not alter core encounter data.
- **Defenders, Monks, Guardsmen, Gallants, Champions, Myrmidons, Centurions and
  Soldiers:** existing tank/melee positioning, class CC, damage and healing. Player
  spell availability and legal dispel/interrupt checks still govern responses.
- **Beastmasters, Torturers, hounds, trainees and undead trash/summons:** existing
  add acquisition, class dispels, snares and crowd control; no confirmed reason to
  duplicate generic class behavior. Verify pack control and runners in live tests.
- **Explicit orders win:** a skull or prioritized target disables autonomous focus.
  Tanks/healers do not acquire the DPS focus. Focus selection excludes controlled
  or CC-marked enemies and limits melee switches to 12 yards; the action rechecks
  range, LOS, attackability and CC marks even while the candidate is cached.
- **No persistent kill-order state is written:** focus uses an encounter action and
  multiplier, so it cannot leave stale prioritized targets after a wipe or map exit.
- **Doors, routing and event progression:** retained in Dungeon Clear; this PR does
  not rewrite its paths or change database completion bits.

## Validation and practical limits

`python3 tests/test_scarlet_monastery.py` compiles the actual SM sources against
minimal engine doubles, including named creator tables and value-cache classes.
It checks shared cache refresh, active cast/aura/death/map boundaries, an existing
movement wait, preserving a running spline, retry throttling, all four hazard
families, healer ordering, late CC marks/range/LOS changes, tank/healer/manual-order
exemptions, charm eligibility, ongoing bot/pet casts, attack cancellation, dispel
exemption, AoE suppression/release, harmful floor avoidance including oversized/whitelisted overlapping auras, both configuration
switches and every trigger-to-action creator. The older Herod test entry point
runs the same expanded scenarios. These doubles do not validate core ABI,
Spell::CheckCast, threat propagation, collision/pathfinding, DPS or latency.

Before deployment: review the final revision and run required CI. No full build,
live pmon measurement, realm spell-data validation or game-client testing was
performed. Keep unmerged until review completes. After an authorized merge, the
wow-server manifest must pin the actual resulting evermore commit in its own
reviewed PR; no proposed branch SHA should be called deployed.

Live acceptance requires all four wings with level-appropriate bots and:

1. Normal fights, approach during a hazard cast, roots, blocked routes and wipes.
2. A caster/healer and melee in each party; verify escape, safe healing and return.
3. Rares when available; confirm poison/curse/bleed handling is appropriate.
4. Cathedral pre-clear, both resurrection phases, heals/interrupts and mind control.
5. Mind control during an existing bot/pet cast, during AoE, and without an eligible
   dispeller; confirm no new direct attacks/AoE into the ally. Existing periodic
   damage already applied cannot be undone by this AI change.
6. Skull/manual orders, late moon/CC marks, mixed-faction dispel eligibility and a
   full autonomous party without a human master.
7. Trash healer focus, caster packs, harmful floor areas, runners and added patrols.
8. Both switches off as baseline. Collect `playerbot pmon stack` before/after using
   BotActiveAlone=100 and botActiveAloneSmartScale=0, five minutes after
   `playerbot pmon toggle`, and compare the final Total line.

Pets have direct ally-attack/cast protection here, but do not gain a dedicated
Whirlwind/Detonation recall controller. Room-safe collision/path selection remains
in the existing movement engine. Those limits must be checked in the live run;
this audit does not claim flawless human play or certify every encounter complete.
