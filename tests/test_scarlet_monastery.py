"""Execute actual Scarlet Monastery code against engine doubles; no server build."""
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
DIRECTORY = ROOT / "src/Ai/Dungeon/SM"


def source(name):
    return "\n".join(line for line in (DIRECTORY / name).read_text().splitlines()
                     if not line.startswith(("#include", "#ifndef", "#define", "#endif")))


def main():
    files = ["SMShared.h", "SMShared.cpp", "SMValueContext.h", "SMActions.h",
             "SMTriggers.h", "SMMultipliers.h", "SMStrategy.h", "SMActions.cpp",
             "SMTriggers.cpp", "SMMultipliers.cpp", "SMStrategy.cpp",
             "SMActionContext.h", "SMTriggerContext.h"]
    code = (ROOT / "tests/fixtures/scarlet_engine_doubles.h").read_text()
    code += "\n" + "\n".join(source(name) for name in files)
    code += "\n" + (ROOT / "tests/fixtures/scarlet_scenarios.cpp").read_text()
    with tempfile.TemporaryDirectory() as temporary:
        path = Path(temporary)
        (path / "test.cpp").write_text(code)
        subprocess.run(["clang++", "-std=c++20", "-Wall", "-Wextra", "-Werror",
                        str(path / "test.cpp"), "-o", str(path / "test")], check=True)
        subprocess.run([str(path / "test")], check=True)


if __name__ == "__main__":
    main()
