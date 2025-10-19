import sys
import subprocess
import platform
from pathlib import Path
import colorama

HELP, FLAGS = [
    ["l [warp_libname]", "build specific library"],
    ["a",                "build combined library warp_toolkit"],
    ["t [warp_libname]", "test with library source (DEV)"],
    ["f [warp_libname]", "test linked against built library (DEV)"],
    ["h",                "display this usage information"],
], [
    ["--r",      "Runs the output application (DEV)"],
    ["--c",      "Cleans build artifacts"],
    ["--static", "Builds static library"],
    ["--shared", "Builds shared library"],
]

CXXFLAGS: list[str] = ["g++", "-std=c++20", "-O3", "-Wall", "-Wextra", "-I."]

TEST_SRC: list[Path] = list(Path("test").glob("*.cpp"))
TEST_BIN: Path       = Path("test/prog")

BUILD_ALL_CMD: str = "warp_toolkit"

match platform.system():
    case "Windows": SHARED_EXT, SHARED_FLAGS = "dll",   ["-shared"]
    case "Darwin":  SHARED_EXT, SHARED_FLAGS = "dylib", ["-fPIC", "-shared"]
    case _:         SHARED_EXT, SHARED_FLAGS = "so",    ["-fPIC", "-shared"]

class TerminalLink:
    def __init__(self): colorama.init(autoreset=True)

    @staticmethod
    def _msg(color: str, tag: str, msg: str): print(f"{color}{tag}{colorama.Style.RESET_ALL} {msg}")

    def info  (self, msg: str): self._msg(colorama.Fore.CYAN,   "[INFO]", msg)
    def ok    (self, msg: str): self._msg(colorama.Fore.GREEN,  "[OK]",   msg)
    def warn  (self, msg: str): self._msg(colorama.Fore.YELLOW, "[WARN]", msg)
    def error (self, msg: str, throw_err: bool = False):
        self._msg(colorama.Fore.RED,    "[ERR]",  msg)
        if throw_err: raise ValueError(msg)

    def run(self, cmd: list[str]):
        print(f"{colorama.Fore.BLUE}$ {' '.join(cmd)}{colorama.Style.RESET_ALL}")
        try: subprocess.run(cmd, check=True)
        except subprocess.CalledProcessError as e: self.error(f"Command failed :{e.returncode}", True)

    def usage(self, err_code: int = 0):
        print(f"{colorama.Style.BRIGHT}USAGE:{colorama.Style.RESET_ALL}")
        for cmd, desc in HELP:
            print(f"  python build.py {colorama.Style.BRIGHT}{cmd}{colorama.Style.RESET_ALL} → {desc}")

        print(f"{colorama.Style.BRIGHT}\nFLAGS:{colorama.Style.RESET_ALL}")
        for flag, desc in FLAGS:
            print(f"  {colorama.Style.DIM}{flag}{colorama.Style.RESET_ALL} → {desc}")

        sys.exit(err_code)

class Command:
    def __init__(self):
        self.cmd: str = sys.argv[1] if len(sys.argv) > 1 and not sys.argv[1].startswith("--") else ""
        self.arg: str = sys.argv[2] if len(sys.argv) > 2 and not sys.argv[1].startswith("--") else ""
        self.is_run    : bool = "--r" in sys.argv
        self.is_clean  : bool = "--c" in sys.argv
        self.is_static : bool = "--static" in sys.argv
        self.is_shared : bool = not self.is_static

    def is_valid_run(self) -> bool: return self.is_run and self.cmd in {'t', 'f'}

SHELL = TerminalLink()

class Utils:
    @staticmethod
    def collect_src(lib: str) -> list[Path]:
        if lib == BUILD_ALL_CMD:
            return [cpp for f in Path(".").glob("warp_*") if f.is_dir() for cpp in f.glob("*.cpp")]
        return list(Path(f"{lib}").glob("*.cpp"))

    @staticmethod
    def compile_srcs(srcs: list[Path]) -> list[Path]:
        objs = [src.with_suffix(".o") for src in srcs]
        for src, obj in zip(srcs, objs):
            SHELL.info(f"Compiling {src} → {obj}")
            SHELL.run([*CXXFLAGS, "-c", str(src), "-o", str(obj)])
        return objs

    @staticmethod
    def clean():
        for path in Path(".").rglob("*"):
            if path.suffix in {".o", ".a", f".{SHARED_EXT}"}:
                SHELL.info(f"Removing: {path}")
                path.unlink(missing_ok=True)
        SHELL.ok("Clean complete.")

class Build:
    @staticmethod
    def build_lib(lib_name: str, is_static: bool, out_dir: Path = Path(".\\")) -> Path:
        if not lib_name.startswith("warp_"): SHELL.error(f"{lib_name} is invalid", True)

        objs: list[Path] = Utils.compile_srcs(Utils.collect_src(lib_name))
        target: Path = out_dir / f"{lib_name}.{'a' if is_static else SHARED_EXT}"
        SHELL.info(f"Building {"static" if is_static else "shared"} library: {target}")

        SHELL.run(
            ["ar", "rcs", str(target), *map(str, objs)] if is_static else
            [*CXXFLAGS, *SHARED_FLAGS, *map(str, objs), "-o", str(target)]
        )

        SHELL.ok(f"Built {target}")
        return target

    @staticmethod
    def build_test(lib_name: str):
        lib_src = Path(lib_name) / f"{lib_name}.cpp"
        if not lib_src.exists(): SHELL.error(f"Missing library source: {lib_src}", True)
        else: SHELL.run([*CXXFLAGS, "-o", str(TEST_BIN), *map(str, TEST_SRC), str(lib_src)])

    @staticmethod
    def build_final(lib_name: str, is_static: bool = False):
        static_lib = Path("test") / f"lib{lib_name}.a"
        shared_lib = Path("test") / f"lib{lib_name}.{SHARED_EXT}"

        if not static_lib.exists() and not shared_lib.exists():
            if not Build.build_lib(lib_name, is_static, Path("test")).exists:
                SHELL.error(f"Failed to locate or build library for '{lib_name}'", True)

        SHELL.info(f"Linking library: {static_lib if is_static else shared_lib}")
        SHELL.run(
            [*CXXFLAGS, "-o", str(TEST_BIN), *map(str, TEST_SRC), str(static_lib)] if is_static else
            [*CXXFLAGS, "-Ltest", f"-l{lib_name}", "-o", str(TEST_BIN), *map(str, TEST_SRC)]
        )

if __name__ == "__main__":
    inp = Command()
    if inp.is_clean: Utils.clean()

    match inp.cmd:
        case 'h' | "help"  : SHELL.usage()
        case 't' | "test"  : Build.build_test(inp.arg)
        case 'f' | "final" : Build.build_final(inp.arg, inp.is_static)
        case 'a' | "all"   : Build.build_lib(BUILD_ALL_CMD, inp.is_static)
        case 'l' | "lib"   : Build.build_lib(inp.arg, inp.is_static)

    if inp.is_valid_run(): SHELL.run([".\\" + str(TEST_BIN)])