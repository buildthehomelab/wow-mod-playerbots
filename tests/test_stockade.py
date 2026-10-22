"""Execute actual Stockade code against engine doubles; no server build."""
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
DIRECTORY = ROOT / "src/Ai/Dungeon/Stockade"


def source(name):
    return "\n".join(line for line in (DIRECTORY / name).read_text().splitlines()
                     if not line.startswith(("#include", "#ifndef", "#define", "#endif")))


def main():
    config = (ROOT / "src/PlayerbotAIConfig.cpp").read_text()
    assert 'GetOption<bool>("Playerbots.EnableStockadeMechanics", false)' in config
    assert "Playerbots.EnableStockadeMechanics = 0" in (ROOT / "conf/playerbots.conf.dist").read_text()
    instance = (ROOT / "src/Bot/PlayerbotAI.cpp").read_text().split("void PlayerbotAI::ApplyInstanceStrategies", 1)[1]
    assert instance.count('"vanilla-stockade"') == 2
    assert 'case 34:' in instance
    for kind in ("Action", "Trigger", "Value"):
        shared = (ROOT / f"src/Bot/Engine/BuildShared{kind}Contexts.cpp").read_text()
        assert f"new Stockade{kind}Context()" in shared
    assert 'creators["vanilla-stockade"]' in (ROOT / "src/Ai/Dungeon/DungeonStrategyContext.h").read_text()

    files = ["StockadeShared.h", "StockadeShared.cpp", "StockadeActions.h",
             "StockadeActions.cpp", "StockadeContexts.h", "StockadeStrategy.h", "StockadeStrategy.cpp"]
    code = (ROOT / "tests/fixtures/stockade_engine_doubles.h").read_text()
    code += "\n" + "\n".join(source(name) for name in files)
    code += "\n" + (ROOT / "tests/fixtures/stockade_scenarios.cpp").read_text()
    with tempfile.TemporaryDirectory() as temporary:
        path = Path(temporary)
        (path / "test.cpp").write_text(code)
        subprocess.run(["clang++", "-std=c++20", "-Wall", "-Wextra", "-Werror",
                        str(path / "test.cpp"), "-o", str(path / "test")], check=True)
        subprocess.run([str(path / "test")], check=True)


if __name__ == "__main__":
    main()
