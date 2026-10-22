"""Execute actual Deadmines code against engine doubles; no server build."""
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
DIRECTORY = ROOT / "src/Ai/Dungeon/Deadmines"


def source(name):
    return "\n".join(line for line in (DIRECTORY / name).read_text().splitlines()
                     if not line.startswith(("#include", "#ifndef", "#define", "#endif")))


def main():
    files = ["DeadminesShared.h", "DeadminesShared.cpp", "DeadminesActions.h",
             "DeadminesActions.cpp", "DeadminesContexts.h", "DeadminesStrategy.h", "DeadminesStrategy.cpp"]
    code = (ROOT / "tests/fixtures/deadmines_engine_doubles.h").read_text()
    code += "\n" + "\n".join(source(name) for name in files)
    code += "\n" + (ROOT / "tests/fixtures/deadmines_scenarios.cpp").read_text()
    with tempfile.TemporaryDirectory() as temporary:
        path = Path(temporary)
        (path / "test.cpp").write_text(code)
        subprocess.run(["clang++", "-std=c++20", "-Wall", "-Wextra", "-Werror",
                        str(path / "test.cpp"), "-o", str(path / "test")], check=True)
        subprocess.run([str(path / "test")], check=True)


if __name__ == "__main__":
    main()
