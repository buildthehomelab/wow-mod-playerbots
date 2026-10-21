# Classic dungeon mechanics: first pass

Source inspection on October 21, 2026 finds Scarlet Monastery as the only Classic
strategy registered in DungeonStrategyContext. Its existing mechanic is Doan's
Detonation escape. This does not establish that other dungeons have no generic
interrupt, dispel, or hazard responses. Dungeon Clear separately handles routing,
objectives, doors and escorts; those are not combat mechanic certification.

## Herod pilot

With `Playerbots.EnableHerodWhirlwindAvoidance = 1`, bots in Scarlet Monastery
react to Herod's active Whirlwind aura or channel (8989). All roles, including
tanks, cancel their current cast and escape to 12 yards. The escape clears the
previous movement wait so an existing approach cannot delay it. Ordinary movement
and movement-to-cast actions are suppressed during the channel; casting resumes
outside the escape boundary, while ordinary movement resumes when the channel ends.
No new trigger or multiplier is registered when this option is disabled (default).
Set it before server startup; changing it requires instance strategies to be rebuilt.

Core SmartAI for creature 3975 casts 8989 on self, initially at 15 seconds, then
30–40 seconds. Reference: Evermore core `data/sql/base/db_world/smart_scripts.sql`.
The eight-yard damage radius comes from the spell-data reference for triggered
Whirlwind 15578, with four yards of clearance:
https://www.wowhead.com/wotlk/spell=15578/whirlwind
Realm DBC overrides and collision/pathfinding still need live validation. Pets do
not gain a dedicated recall mechanic in this change. Existing Doan logic is preserved.

## Verification

`python3 tests/test_herod_whirlwind.py` compiles the actual changed functions with
minimal engine doubles. It checks channel/aura detection, dead/missing/wrong-map
bosses, boundary behavior, preemption of an existing move wait, suppression during
the channel, safe-distance casting and release after the channel ends.
This is not a full module build, pathfinding test or game simulation.

Before enablement, use a level-appropriate tank, healer and three DPS, including a
caster and melee. Trigger Whirlwind both while already attacking and during the
tank's approach. Verify escape paths remain inside the room, healers heal once
safe, nobody walks back early, and tanks reacquire Herod afterward. Repeat with
the option off, after a wipe, and while rooted. Compare `playerbot pmon stack`
before/after with BotActiveAlone=100 and botActiveAloneSmartScale=0, collecting
five minutes after `playerbot pmon toggle`. No live measurements are claimed.

## Next audit candidates, not verified defects

- Scarlet Monastery: Whitemane healing/phase targeting; trash heals and crowd control.
- Deadmines: Smite's stun/weapon phases and add control.
- Wailing Caverns: sleep/poison responses and protection during the Naralex escort.
- Shadowfang Keep: Arugal positioning, line of sight and curse responses.
- Gnomeregan: Thermaplugg bombs and button assignments.
- Uldaman and Zul'Farrak: activated adds and event positioning.

For each, verify the current realm scripts/SmartAI, spell data and generic bot
coverage before writing encounter-specific logic. Dungeon completion alone is
insufficient: record the bot's response to each relevant mechanic.
