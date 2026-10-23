# Classic dungeon mechanics

The first complete dungeon source pass covers Scarlet Monastery's four wings,
including boss, rare, trash, event and generic-class behavior. See
[the coverage matrix and live test checklist](SCARLET-MONASTERY-MECHANICS.md) and
[the actor inventory](SCARLET-MONASTERY-ACTORS.csv).

The implementation remains opt-in and requires final PR review and live validation.
A source audit is not a claim of deployed or fully tested dungeon behavior.

Future dungeon passes: Deadmines, Wailing Caverns, Shadowfang Keep, Gnomeregan,
Uldaman and Zul'Farrak. Verify realm scripts/spell data and existing generic AI for
each before adding encounter code. Dungeon completion alone is not mechanic coverage.
