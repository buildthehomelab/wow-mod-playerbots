"""Execute actual RazorfenKraul code against engine doubles; no server build."""
from pathlib import Path
import subprocess
import tempfile
import re

ROOT = Path(__file__).resolve().parents[1]
DIRECTORY = ROOT / "src/Ai/Dungeon/RazorfenKraul"


def source(name):
    return "\n".join(line for line in (DIRECTORY / name).read_text().splitlines()
                     if not line.startswith(("#include", "#ifndef", "#define", "#endif")))


def main():
    config = (ROOT / "src/PlayerbotAIConfig.cpp").read_text()
    assert 'GetOption<bool>("Playerbots.EnableRazorfenKraulMechanics", false)' in config
    assert "Playerbots.EnableRazorfenKraulMechanics = 0" in (ROOT / "conf/playerbots.conf.dist").read_text()
    instance = (ROOT / "src/Bot/PlayerbotAI.cpp").read_text().split("void PlayerbotAI::ApplyInstanceStrategies", 1)[1]
    assert instance.count('"vanilla-rfk"') == 2
    assert 'case 47:' in instance
    for kind in ("Action", "Trigger", "Value"):
        shared = (ROOT / f"src/Bot/Engine/BuildShared{kind}Contexts.cpp").read_text()
        assert f"new RazorfenKraul{kind}Context()" in shared
    assert 'creators["vanilla-rfk"]' in (ROOT / "src/Ai/Dungeon/DungeonStrategyContext.h").read_text()

    files = ["RazorfenKraulShared.h", "RazorfenKraulShared.cpp", "RazorfenKraulActions.h",
             "RazorfenKraulActions.cpp", "RazorfenKraulContexts.h", "RazorfenKraulStrategy.h", "RazorfenKraulStrategy.cpp"]
    files.remove("RazorfenKraulStrategy.cpp")
    code = (ROOT / "tests/fixtures/razorfen_kraul_engine_doubles.h").read_text()
    code += "\n" + "\n".join(source(name).replace("    RazorfenKraul::InitCharmTriggers(triggers);", "").replace("    RazorfenKraul::InitCharmMultipliers(botAI, multipliers);", "").replace('    triggers.push_back(new TriggerNode("razorfen kraul hazard", {NextAction("razorfen kraul escape", ACTION_INTERRUPT - 1)}));', "").replace("    multipliers.push_back(new RazorfenKraulHazardMultiplier(botAI));", "") for name in files)
    strategy = source("RazorfenKraulStrategy.cpp")
    strategy = strategy[:strategy.index("class RazorfenKraulHazardMultiplier")] + strategy[strategy.index("class RazorfenKraulFocusMultiplier"):]
    strategy = re.sub(r'    triggers.push_back\(\s*new TriggerNode\("razorfen kraul hazard".*?;\n', '', strategy, flags=re.S)
    strategy = "\n".join(line for line in strategy.splitlines() if not any(key in line for key in ("InitCharm", 'TriggerNode("razorfen kraul hazard"', "new RazorfenKraulHazardMultiplier")))
    code += "\n" + strategy
    code += "\n" + (ROOT / "tests/fixtures/razorfen_kraul_scenarios.cpp").read_text()
    with tempfile.TemporaryDirectory() as temporary:
        path = Path(temporary)
        (path / "test.cpp").write_text(code)
        subprocess.run(["clang++", "-std=c++20", "-Wall", "-Wextra", "-Werror",
                        str(path / "test.cpp"), "-o", str(path / "test")], check=True)
        subprocess.run([str(path / "test")], check=True)


if __name__ == "__main__":
    main()
