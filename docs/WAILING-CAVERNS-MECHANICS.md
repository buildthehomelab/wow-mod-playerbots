# Wailing Caverns mechanics audit

October 22, 2026. Scope: the complete map-43 instance, all fixed actors,
Disciple summons/creature groups, Naralex ritual and route events. Exterior
Barrens caves (map 1) are inventoried separately below. Source coverage is not
live gameplay certification. The new option defaults off.

## Source basis

Current fetched Evermore core `fccbd096804b33d71a36d7a3920f1b3256ff8cdd`,
Playerbots `a8c7980054fc56d47942360ad086fe758b432295`, Dungeon Clear
`34533610d73e1dee404a0f826439b89246b9bca4`, and Individual Progression (IP)
`70c49ec8c0d254c4a184e2b021e23235c52d7775` match the current authenticated
wow-server main manifest read on this date. The core commit was also verified
through the authenticated GitHub connector. These pins do not establish which
binary or database is running.

- [Core SmartAI](https://github.com/buildthehomelab/wow-azerothcore/blob/fccbd096804b33d71a36d7a3920f1b3256ff8cdd/data/sql/base/db_world/smart_scripts.sql), creature spawns/templates/addons, template spells and creature summon groups.
- [Instance state](https://github.com/buildthehomelab/wow-azerothcore/blob/fccbd096804b33d71a36d7a3920f1b3256ff8cdd/src/server/scripts/Kalimdor/WailingCaverns/instance_wailing_caverns.cpp): four Fanglord death states unlock the Disciple; saved encounter data and numeric GUIDs.
- [IP replacement](https://github.com/buildthehomelab/wow-mod-individual-progression/blob/70c49ec8c0d254c4a184e2b021e23235c52d7775/data/sql/world/base/dungeon_wailing_caverns.sql): replaces selected creature SmartAI, adds Anacondra pooling, and alters exterior rare spawns.
- [Dungeon Clear events](https://github.com/buildthehomelab/wow-mod-dungeon-clear/blob/34533610d73e1dee404a0f826439b89246b9bca4/src/Ai/Dungeon/DungeonClear/Data/Events/WailingCavernsEvents.cpp): persistent Serpentis jump, Verdan return hole/drop and Disciple escort with summon/kill completion.

`WAILING-CAVERNS-ACTORS.csv` inventories 27 actor entries from 312 base fixed
spawns plus verified summons (including Mutanus); it separates base spell IDs
from effective IP spell IDs and retains original comments as evidence, not
spell-effect truth. Effective selection has 127 SmartAI rows (123 base selected
rows, 49 IP rows replacing 45 base rows). IP's Anacondra pool replaces her base
spawn with four possible locations, one active. These are source rows, not live
spawn counts. Ectoplasm display transformations reference templates 5776-5778;
they do not invent additional summoned combat actors.

Spell effects were extracted from the saved Evermore z80 reference MPQ. This is
not a readback of the loaded realm DBC. Cast-time/radius/duration fields in that
snapshot are indexes, not milliseconds or yards. Selected core spell_dbc/script-name/script rows add no overrides. Linked-spell
rows attach Fire/Frost/Nature/Shadow immunity to the Red/Blue/Green/Black
ectoplasm forms and remove the other forms; existing CanCastSpell immunity
checks remain authoritative. Current pending core SQL contains unrelated raid
assistance changes. This is a source screen, not a live migration simulation.
No fixed radius, fabricated
telegraph, or guessed timing is used in this implementation. Core SpellMgr was
screened for encounter spell IDs; loaded overrides and live spell legality still
require realm validation. Source snapshots and selected rows are retained in
`artifacts/wailing-caverns-mechanics/source/` in the workspace.

## Confirmed gap and implementation

The generic DPS selection has no explicit assist-runner preference. A bot may
stay on a healthy target while a nearby Druid of the Fang or Skum flees for help.
`Playerbots.EnableWailingCavernsMechanics = 1` adds only that missing local focus.
It does not duplicate generic interrupts, dispels, CC, class rotations or events.

The map-local strategy caches a numeric target GUID for 500 ms using the existing
`attackers` cache. Only engaged, attackable Skum (3674) and Druids
(3840) with actual assist-seeking or fleeing movement qualify. The core makes
ASSISTANCE_MOTION_TYPE runners passive while traveling to a nearby helper;
that specific passive state is allowed. Passive idle actors are excluded. Tanks and healers retain their
normal targets. Explicit skull/prioritized targets win; breakable CC and group CC
assignments are excluded. Melee selection is within 12 combat yards; ranged
selection uses existing spell range. This is a selection bound, not a new pursuit
leash on ordinary class movement. Dead/despawned, out-of-map, out-of-range and
out-of-LOS candidates are rejected at use time. Actions recheck CC assignments
and invalidate rejected focus caches so ordinary target changes resume. The
multiplier suppresses competing target-switch actions only while a focus is
valid, retaining melee swings, all spell actions, interrupts, heals and dispels.

With the option disabled no triggers/multipliers are added. Strategy entry/exit
is balanced in `ApplyInstanceStrategies` (which must be enabled). Configure
before instance strategy initialization. No grid/name scan, map-thread DB query,
cross-tick raw Unit pointer, hazard suppression or movement action is added.

## Bosses and rare

| Actor | Confirmed mechanics and response |
| --- | --- |
| Lady Anacondra 3671 | IP casts Lightning Bolt 9532, Sleep 700 (comment calls it Druids Slumber), Healing Touch 5187 and Thorns Aura 8148. Retain eligible current-target/nearby-healer interrupts and magic sleep removal. 8148 has dispel type 0 in the snapshot, so do not promise ordinary magic purge. Pool picks one of four positions; no fixed-coordinate encounter detection. Death records Fanglord state 2. |
| Lord Cobrahn 3669 | Lightning Bolt, Druid's Slumber 8040, Healing Touch, serpent form 7965 at 30% and Poison Proc 3616. Retain learned/legal interrupts, magic sleep removal, poison cleansing/healing and ordinary tank/melee-behind positioning. Form is not a floor hazard. Death records state 0. |
| Lord Pythas 3670 | Bolt, Sleep 700, Healing Touch, Thunderclap 8147. Retain interrupts, dispels and role ranges; no reactive movement for a cast-index-1 area hit. Death records state 1. |
| Lord Serpentis 3673 | Bolt, Sleep 700 and stronger Healing Touch 6778. Generic class responses; persistent DC lip/jump reaches his disconnected shelf. Death records state 3. |
| Kresh 3653 | No selected SmartAI or template-spell rotation; ordinary tank threat/healing and damage. Do not add a fabricated shell dodge. |
| Skum 3674 | Chained Bolt 6254 is a targeted damage spell, not a dynamic floor. Preserve eligible interrupts and ordinary ranged positioning/healing. IP flees for assist below 15%; new local DPS runner focus. No new spread solver is claimed. |
| Verdan the Everliving 5775 | Grasping Vines 8142 is caster-area damage/root plus triggered 5164. Retain eligible interrupts, magic root removal/self recovery and healing. It is not persistent ground; no speculative escape action or cast suppression is added. DC already returns through the hole to the lower caverns afterward. |
| Mutanus the Devourer 3654 | Naralex's Nightmare 7967 (magic sleep with health/pacify components), Terrify 7399 and Thundercrack 8150 (area damage plus sleep). Retain magic dispels, fear recovery, eligible interrupts and role positioning/healing. Do not describe 8150 as merely a stun or dodge an instant-index cast. He moves into the chamber/zone combat; death signals Disciple and instance state 4. |
| Deviate Faerie Dragon 5912 | Base SmartAI has an 85% AI-init despawn roll, so availability is conditional. No selected combat casts/template spells; do not add DBM-derived Mana Burn responses unsupported by this source. |

## Trash, adds, events and hazards

| Actor / event | Coverage and response |
| --- | --- |
| Druid of the Fang 3840 | Bolt, 8040 sleep, Healing Touch, serpent form 8041 and out-of-combat Nature Channeling 13236. Existing interrupt/dispels/CC. IP 15% assist flee gets new nearby DPS focus. Unengaged channelers are never pulled by this strategy. |
| Deviate Ravager 3636 | Thrash Proc 8876 triggers 3391; ordinary tank/heal/CC handling. Disciple summons two at waypoint 5, with zone combat. |
| Deviate Guardian 3637 | Calls for help within 40 at 30% health, without a cast or fleeing telegraph. Generic burst/CC/pull isolation; do not falsely call this an interruptible runner or invent threshold polling. |
| Evolving Ectoplasm 3640 | Random timed spells 7943-7946 transform appearance using entries 5776-5778/3640. Linked spells apply Fire/Frost/Nature/Shadow immunity respectively; generic CanCastSpell immunity checks remain. No invented elemental rotation or hazard. |
| Deviate Adder 5048 | Poison Proc 3616 triggers poison 744; available poison removal/healing, add acquisition. Disciple group 1 contains two Adders and a Viper. |
| Deviate Crocolisk 5053 | Tendon Rip 3604 is a target slow with dispel type 0. Generic healing/self recovery and positioning, no invented poison dispel. |
| Deviate Lasher 5055 | Wide Slash 7342 is a frontal weapon effect. Preserve tank facing and melee-behind behavior, plus existing CC. |
| Deviate Dreadfang 5056 | Terrify 7399 is magic fear; generic dispels/fear breaks and healing. A feared creature is not selected merely by a name or health threshold. |
| Deviate Viper 5755 | Localized Toxin 7947 is target poison periodic damage. Generic poison cleansing/healing; movement does not remove it. Zone combat on summon. |
| Deviate Venomwing 5756 | Toxic Spit 7951 is direct target damage with poison dispel metadata but no persistent aura effect. Generic interrupt where legal and healing; no imaginary poison cloud/DoT cleansing. |
| Deviate Shambler 5761 | Wild Regeneration 7948 is a magic HoT. Generic interrupt and eligible offensive magic dispel when learned/legal, ordinary target acquisition. |
| Deviate Python 8886 | No selected scripted spell rotation; retain pack control and tank acquisition. |
| Deviate Moccasin 5762 | Three direct ritual summons; zone combat and death counter 1 on Disciple. Generic add pickup and DC escort protection. |
| Nightmare Ectoplasm 5763 | Six group-2 rows (one duplicate position), zone combat and death counter 2. DC protects escort/engages attackers. No fabricated acid floor. |
| Disciple 3678 / Naralex 3679 | Four Fanglords unlock start gossip; waypoint 5 Ravagers, waypoint 12 snake group, waypoint 25 awakening/three Moccasins, group-2 Ectoplasms, then Mutanus gated on death counters. Mutanus death starts awakening/flight/departure. DC's persistent escort step already follows, engages escort attackers and recovers start gossip; completion waits for Mutanus/encounter bit, not arrival at the ritual. Do not replace it or attack friendly Naralex. |
| Snake 2914 / Biletoad 3835 | Critters inventoried; no encounter action. |
| Ledges / water / roots | DC owns the persistent one-way Serpentis jump and Verdan hole return, including follower handling. Generic collision/pathfinding and root recovery remain; no new movement to interfere with either drop. No selected damaging persistent ground spell was found, so there is no new hazard scanner. |
| Runners / CC / pets | New runner focus checks actual movement and CC; existing class snares/stuns remain conditional. Pet behavior stays with AttackAction/class AI; no new independent recall, global CC-safe AoE policy or pet hazard guarantee. |

Exterior map-1 coverage: Trigore the Lasher 3652 and Boahn 3672 have IP respawn/
wander changes; Boahn casts Bolt/Healing Touch/form and flees at 15%. Mad Magglish
3655 uses Sneak 22766 and Smoke Bomb 7964; IP pools one of three exterior spawns.
Coiler 3630 flees; Stinglash 3631 uses Lash 6607; Creeper 3632 uses Sneak and
Infected Wound 3427 (disease); Slayer 3633 uses Fatal Bite; Stalker 3634 uses
Sneak. These retain generic behavior: map-43 focus intentionally does not run
outside the instance. Exterior variants are not miscounted as instance bosses.

Generic capabilities depend on class, learned rank, resources, immunity, cast
legality, range and LOS. Low-level parties can lack magic/poison removal and
interrupts. This pass neither grants spells nor claims guaranteed prevention.

## Verification and acceptance

`python3 tests/test_wailing_caverns.py` compiles/executes actual encounter code
against small engine doubles. It covers switch-off, creator wiring, deterministic
runner selection, unrelated fleeing mobs, stale GUIDs, late CC/range/LOS changes,
normal-target release, passive assistance movement and its late CC rejection,
roles/manual orders, passive-idle/dead/unloaded targets,
map gating and movement ending. Module codestyle, touched-source/fixture
clang-format and whitespace checks are required. This is not a full server build
or proof of ABI, pathfinding, real cast checks or latency.

No full build (repository forbids unrequested builds), live DB/DBC query, pmon
profile, game-client run, enablement, deployment or restart was performed.
Independent and formal final-revision reviews must complete before merge.

Live acceptance: run the entire route at appropriate level with tank, healer,
melee/ranged and pets, human-led and autonomous. Include each available boss/rare,
Anacondra's four locations, spell interrupts/sleeps/poisons/roots, Guardian help,
Druid/Skum runners, CC assigned after selection, explicit skull overrides, range/
LOS rejection, serpent forms, Shambler regeneration, all escort waves, Disciple
death/restart and wipe/save recovery, Mutanus completion, both drops and water.
Repeat with the switch off and with missing low-level class capabilities. Check
that pets and AoE do not undermine assigned CC under existing class policy.

Measure baseline and enabled per-bot cost with BotActiveAlone=100 and
botActiveAloneSmartScale=0, `playerbot pmon toggle` after login, then
`playerbot pmon stack` five minutes later; compare the final Total line.

Keep unmerged pending authorization. Only after an authorized implementation
merge can a separate reviewed wow-server PR pin the actual reachable merged
commit. A proposed branch SHA is not a deployment fact.
