import sys
import subprocess
import platform
from pathlib import Path
import colorama
from typing import Callable

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

match platform.system():
    case "Windows": SHARED_EXT, SHARED_FLAGS = "dll",   ["-shared"]
    case "Darwin":  SHARED_EXT, SHARED_FLAGS = "dylib", ["-fPIC", "-shared"]
    case _:         SHARED_EXT, SHARED_FLAGS = "so",    ["-fPIC", "-shared"]

class ShellLink:
    def __init__(self): colorama.init(autoreset=True)

    @staticmethod
    def _msg(color: str, tag: str, msg: str | list[str]):
        msgs = msg if isinstance(msg, list) else [msg]
        [
            print(f"{color}{tag}{colorama.Style.RESET_ALL}: {line}")
            for m in msgs for line in str(m).splitlines()
        ]

    def info  (self, msg: str | list[str]): self._msg(colorama.Fore.CYAN,   "[INFO]", msg)
    def ok    (self, msg: str | list[str]): self._msg(colorama.Fore.GREEN,  "[OK]",   msg)
    def warn  (self, msg: str | list[str]): self._msg(colorama.Fore.YELLOW, "[WARN]", msg)
    def error (self, msg: str | list[str], throw_err: bool = False):
        self._msg(colorama.Fore.RED,    "[ERR]",  msg)
        if throw_err: raise ValueError(msg)

    def run(self, cmd: list[str]):
        print(f"{colorama.Fore.BLUE}$ {' '.join(cmd)}{colorama.Style.RESET_ALL}")
        try: subprocess.run(cmd, check=True)
        except subprocess.CalledProcessError as e: self.error(f"Command failed :{e.returncode}", True)
        else: self.ok("Command completed successfully.")

    def usage(self, err_code: int = 0):
        print(f"{colorama.Style.BRIGHT}USAGE:{colorama.Style.RESET_ALL}")
        for cmd, desc in HELP:
            print(f"  python build.py {colorama.Style.BRIGHT}{cmd}{colorama.Style.RESET_ALL} → {desc}")

        print(f"{colorama.Style.BRIGHT}\nFLAGS:{colorama.Style.RESET_ALL}")
        for flag, desc in FLAGS:
            print(f"  {colorama.Style.DIM}{flag}{colorama.Style.RESET_ALL} → {desc}")

        sys.exit(err_code)

SHELL = ShellLink()

class CompUtil:
    @staticmethod
    def collect_src(lib: str) -> list[Path]:
        return [
            cpp for f in Path(".").glob("warp_*") if f.is_dir() for cpp in f.glob("*.cpp")
        ] if lib == "toolkit" else list(Path(f"{lib}").glob("*.cpp"))

    @staticmethod
    def compile_sources(srcs: list[Path]) -> list[Path]:
        objs = []
        for src in srcs:
            obj = src.with_suffix(".o")
            SHELL.run([*CXXFLAGS, "-c", str(src), "-o", str(obj)])
            objs.append(obj)

        return objs

    @staticmethod
    def make_static_lib(lib_name: str, srcs: list[Path], out_path: Path = Path(".")) -> Path:
        SHELL.info(f"Building static library: lib{lib_name}.a")
        SHELL.run(["ar", "rcs", str(out_path), *map(str, CompUtil.compile_sources(srcs))])
        SHELL.ok(f"Built {lib_name}")

        return out_path

    @staticmethod
    def make_shared_lib(lib_name: str, srcs: list[Path], out_path: Path = Path(".")) -> Path:
        SHELL.info(f"Building shared library: lib{lib_name}.{SHARED_EXT}")
        SHELL.run([*CXXFLAGS, *SHARED_FLAGS, *map(str, CompUtil.compile_sources(srcs)), "-o", str(out_path)])
        SHELL.ok(f"Built {lib_name}")

        return out_path

    @staticmethod
    def clean():
        [
            path.unlink() or SHELL.info(f"Removing: {path}")
            for path in Path(".").rglob("*")
            if path.suffix in {".o", ".a", f".{SHARED_EXT}"}
        ]
        SHELL.ok("Clean complete.")

class Command:
    def __init__(self):
        self.cmd: str = sys.argv[1] if len(sys.argv) > 1 and not sys.argv[1].startswith("--") else ""
        self.arg: str = sys.argv[2] if len(sys.argv) > 2 and not sys.argv[1].startswith("--") else ""
        self.is_run    : bool = "--r" in sys.argv
        self.is_clean  : bool = "--c" in sys.argv
        self.is_static : bool = "--static" in sys.argv
        self.is_shared : bool = not self.is_static

class BuildSystem:

    @staticmethod
    def build_library(
        lib_name: str, lib_build_fn: Callable[[str, list[Path], Path], Path], out_path: Path = Path(".")
    ) -> Path: return lib_build_fn(lib_name, CompUtil.collect_src(lib_name), out_path)

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
            SHELL.warn(f"No existing libraries for '{lib_name}' found — building {"static" if is_static else "shared"} version.")

            if not BuildSystem.build_library(
                lib_name,
                CompUtil.make_static_lib if is_static else CompUtil.make_shared_lib,
                static_lib if is_static else shared_lib
            ).exists: SHELL.error(f"Failed to locate or build library for '{lib_name}'", True)

        SHELL.info(f"Linking library: {static_lib if is_static else shared_lib}")
        SHELL.run(
            [*CXXFLAGS, "-o", str(TEST_BIN), str(TEST_SRC), str(static_lib)] if is_static else
            [*CXXFLAGS, "-Ltest", f"-l{lib_name}", "-o", str(TEST_BIN), *map(str, TEST_SRC)]
        )

    @staticmethod
    def run():
        inp = Command()
        if inp.is_clean: CompUtil.clean()

        match inp.cmd:
            case 'h': SHELL.usage()
            case 't': BuildSystem.build_test(inp.arg)
            case 'f': BuildSystem.build_final(inp.arg, inp.is_static)
            case 'a': BuildSystem.build_library(
                "toolkit",
                CompUtil.make_static_lib if inp.is_static else CompUtil.make_shared_lib,
                Path("./") / f"toolkit.{'a' if inp.is_static else SHARED_EXT}"
            )
            case 'l': BuildSystem.build_library(
                inp.arg,
                CompUtil.make_static_lib if inp.is_static else CompUtil.make_shared_lib,
                Path("./") / f"{inp.arg}.{'a' if inp.is_static else SHARED_EXT}"
            )

        if inp.is_run: SHELL.run(["./" + str(TEST_BIN)])

if __name__ == "__main__":
    BuildSystem().run()