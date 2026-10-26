"""Execute actual RazorfenKraul code against engine doubles; no server build."""
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
DIRECTORY = ROOT / "src/Ai/Dungeon/RazorfenKraul"


def source(name):
    return "\n".join(line for line in (DIRECTORY / name).read_text().splitlines()
                     if not line.startswith(("#include", "#ifndef", "#define", "#endif")))


def main():
    files = ["RazorfenKraulShared.h", "RazorfenKraulAvoidance.h", "RazorfenKraulShared.cpp", "RazorfenKraulAvoidance.cpp",
             "RazorfenKraulActions.h", "RazorfenKraulActions.cpp", "RazorfenKraulEscapeAction.h", "RazorfenKraulEscapeAction.cpp",
             "RazorfenKraulContexts.h", "RazorfenKraulAvoidanceContexts.h", "RazorfenKraulCharm.h", "RazorfenKraulCharm.cpp",
             "RazorfenKraulStrategy.h", "RazorfenKraulStrategy.cpp"]
    code = (ROOT / "tests/fixtures/razorfen_kraul_integration_doubles.h").read_text()
    code += "\n" + "\n".join(source(name) for name in files)
    code += "\n" + (ROOT / "tests/fixtures/razorfen_kraul_integration_scenarios.cpp").read_text()
    with tempfile.TemporaryDirectory() as temporary:
        path = Path(temporary)
        (path / "test.cpp").write_text(code)
        subprocess.run(["clang++", "-std=c++20", "-Wall", "-Wextra", "-Werror",
                        str(path / "test.cpp"), "-o", str(path / "test")], check=True)
        subprocess.run([str(path / "test")], check=True)


if __name__ == "__main__":
    main()
