"""Execute actual Blackfathom code against engine doubles; no server build."""
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
DIRECTORY = ROOT / "src/Ai/Dungeon/Blackfathom"


def source(name):
    return "\n".join(line for line in (DIRECTORY / name).read_text().splitlines()
                     if not line.startswith(("#include", "#ifndef", "#define", "#endif")))


def main():
    config = (ROOT / "src/PlayerbotAIConfig.cpp").read_text()
    assert 'GetOption<bool>("Playerbots.EnableBlackfathomMechanics", false)' in config
    assert "Playerbots.EnableBlackfathomMechanics = 0" in (ROOT / "conf/playerbots.conf.dist").read_text()
    instance = (ROOT / "src/Bot/PlayerbotAI.cpp").read_text().split("void PlayerbotAI::ApplyInstanceStrategies", 1)[1]
    assert instance.count('"vanilla-blackfathom"') == 2
    assert 'case 48:' in instance
    for kind in ("Action", "Trigger", "Value"):
        shared = (ROOT / f"src/Bot/Engine/BuildShared{kind}Contexts.cpp").read_text()
        assert f"new Blackfathom{kind}Context()" in shared
    assert 'creators["vanilla-blackfathom"]' in (ROOT / "src/Ai/Dungeon/DungeonStrategyContext.h").read_text()

    files = ["BlackfathomShared.h", "BlackfathomShared.cpp", "BlackfathomActions.h",
             "BlackfathomActions.cpp", "BlackfathomContexts.h", "BlackfathomStrategy.h", "BlackfathomStrategy.cpp"]
    files.remove("BlackfathomStrategy.cpp")
    code = (ROOT / "tests/fixtures/blackfathom_engine_doubles.h").read_text()
    code += "\n" + "\n".join(source(name).replace("    Blackfathom::InitCharmTriggers(triggers);", "").replace("    Blackfathom::InitCharmMultipliers(botAI, multipliers);", "").replace('    triggers.push_back(new TriggerNode("blackfathom hazard", {NextAction("blackfathom escape", ACTION_INTERRUPT - 1)}));', "").replace("    multipliers.push_back(new BlackfathomHazardMultiplier(botAI));", "") for name in files)
    strategy = source("BlackfathomStrategy.cpp")
    strategy = strategy[:strategy.index("class BlackfathomHazardMultiplier")] + strategy[strategy.index("class BlackfathomFocusMultiplier"):]
    strategy = "\n".join(line for line in strategy.splitlines() if not any(key in line for key in ("InitCharm", 'TriggerNode("blackfathom hazard"', "new BlackfathomHazardMultiplier")))
    code += "\n" + strategy
    code += "\n" + (ROOT / "tests/fixtures/blackfathom_scenarios.cpp").read_text()
    with tempfile.TemporaryDirectory() as temporary:
        path = Path(temporary)
        (path / "test.cpp").write_text(code)
        subprocess.run(["clang++", "-std=c++20", "-Wall", "-Wextra", "-Werror",
                        str(path / "test.cpp"), "-o", str(path / "test")], check=True)
        subprocess.run([str(path / "test")], check=True)


if __name__ == "__main__":
    main()
