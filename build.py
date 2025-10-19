import sys
import subprocess
import platform
from pathlib import Path
import colorama
from typing import Callable


# Command usage information
USAGE = [
    ["[warp_libname]", "build specific library"],
    ["a", "build combined library (all)"],
    ["t [warp_libname]", "test with library source (DEV)"],
    ["f [warp_libname]", "test linked against built library (DEV)"],
    ["h", "display this usage information"],
]

FLAGS = [
    ["--r", "Runs the output application"],
    ["--c", "Cleans build artifacts"],
    ["--static", "Builds static library"],
    ["--shared", "Builds shared library"],
]

# Compiler configuration
CXX: str = "g++"
CXXFLAGS: list[str] = ["-std=c++20", "-O3", "-Wall", "-Wextra", "-I."]

AR: str = "ar"
ARFLAGS: list[str] = ["rcs"]

match platform.system():
    case "Windows": SHARED_EXT, SHARED_FLAGS = "dll",   ["-shared"]
    case "Darwin":  SHARED_EXT, SHARED_FLAGS = "dylib", ["-fPIC", "-shared"]
    case _:         SHARED_EXT, SHARED_FLAGS = "so",    ["-fPIC", "-shared"]

class ShellLink:
    def __init__(self):
        colorama.init(autoreset=True)

    @staticmethod
    def _msg(color: str, tag: str, msg: str):
        print(f"{color}{tag}{colorama.Style.RESET_ALL}: {msg}")

    @staticmethod
    def _usage(cmd: str, desc: str):
        print(f"  python build.py {colorama.Style.BRIGHT}{cmd}{colorama.Style.RESET_ALL} → {desc}")

    @staticmethod
    def _flags(flag: str, desc: str):
        print(f"  {colorama.Style.DIM}{flag}{colorama.Style.RESET_ALL} → {desc}")

    def info(self, msg: str):
        self._msg(colorama.Fore.CYAN, "[INFO]", msg)

    def ok(self, msg: str):
        self._msg(colorama.Fore.GREEN, "[OK]", msg)

    def warn(self, msg: str):
        self._msg(colorama.Fore.YELLOW, "[WARN]", msg)

    def error(self, msg: str):
        self._msg(colorama.Fore.RED, "[ERR]", msg)

    def run(self, cmd: list[str]):
        print(f"{colorama.Fore.BLUE}$ {' '.join(cmd)}{colorama.Style.RESET_ALL}")
        try:
            subprocess.run(cmd, check=True)
        except subprocess.CalledProcessError as e:
            self.error(f"Command failed with exit code {e.returncode}")
            raise ValueError(f"Invalid Command {' '.join(cmd)}")
        else:
            self.ok("Command completed successfully.")

    def usage(self, err_code: int = 0):
        print(f"{colorama.Style.BRIGHT}USAGE:{colorama.Style.RESET_ALL}")

        for cmd, desc in USAGE:
            self._usage(cmd, desc)

        print(f"{colorama.Style.BRIGHT}\nFLAGS:{colorama.Style.RESET_ALL}")

        for flag, desc in FLAGS:
            self._flags(flag, desc)

        sys.exit(err_code)

SHELL = ShellLink()

class CompileTools:
    @staticmethod
    def collect_sources(lib: str) -> list[Path]:
        return [
            cpp
            for f in Path(".").glob("warp_*") if f.is_dir()
            for cpp in f.glob("*.cpp")
        ] if lib == "toolkit" else list(Path(f"{lib}").glob("*.cpp"))

    @staticmethod
    def compile_sources(srcs: list[Path]) -> list[Path]:
        objs = []

        for src in srcs:
            obj = src.with_suffix(".o")
            SHELL.info(f"Compiling {src} → {obj}")
            SHELL.run([CXX, *CXXFLAGS, "-c", str(src), "-o", str(obj)])
            objs.append(obj)

        return objs

    @staticmethod
    def make_static_lib(lib_name: str, srcs: list[Path], out_dir: Path = Path(".")) -> Path:
        SHELL.info(f"Building static library: lib{lib_name}.a")

        lib_path = out_dir / f"lib{lib_name}.a"
        SHELL.run([AR, *ARFLAGS, *map(str, CompileTools.compile_sources(srcs)), "-o", str(lib_path)])
        SHELL.ok(f"Built {lib_path}")

        return lib_path

    @staticmethod
    def make_shared_lib(lib_name: str, srcs: list[Path], out_path: Path = Path(".")) -> Path:
        SHELL.info(f"Building shared library: lib{lib_name}.{SHARED_EXT}")

        # print(f"lib_name = {lib_name}")
        # print(f"srcs = {srcs}")
        # print(f"out_path = {out_path}")

        SHELL.run([CXX, *CXXFLAGS, *SHARED_FLAGS, *map(str, CompileTools.compile_sources(srcs)), "-o", str(out_path)])
        SHELL.ok(f"Built {lib_name}")

        return out_path

    # Clean build artifacts
    @staticmethod
    def clean():
        for path in Path(".").rglob("*"):
            if path.suffix in {".o", ".a", f".{SHARED_EXT}"}:
                SHELL.info(f"Removing: {path}")
                path.unlink()

        SHELL.ok("Clean complete.")

class Command:
    def __init__(
        self, cmd: str, arg: str, clean: bool, run: bool, static: bool = False
    ):
        self.cmd = cmd
        self.arg = arg
        self.is_clean: bool = clean
        self.is_run: bool = run
        self.is_static: bool = static
        self.is_shared: bool = not static # on if --static is not active

class BuildSystem:
    def __init__(self):
        self.test_src = list(Path("test").glob("*.cpp"))
        self.test_bin = Path("test/prog")

    # Build a library given its name and build function -> library file
    def build_library(
        self,
        lib_name: str,
        lib_build_fn: Callable[[str, list[Path], Path], Path],
        out_dir: Path = Path(".")
    ) -> Path:
        return lib_build_fn(
            lib_name,
            CompileTools.collect_sources(lib_name),
            out_dir
        )

    def build_test(self, lib_name: str):
        lib_folder = Path(lib_name)
        lib_src = lib_folder / f"{lib_name}.cpp"

        if not lib_src.exists():
            SHELL.error(f"Missing library source: {lib_src}")
            raise ValueError(f"{lib_src} Not found")

        SHELL.run([CXX, *CXXFLAGS, "-o", str(self.test_bin), *map(str, self.test_src), str(lib_src)])

    def build_final(self, lib_name: str, is_static: bool = False):
        test_dir = Path("test")

        static_lib = test_dir / f"lib{lib_name}.a"
        shared_lib = test_dir / f"lib{lib_name}.{SHARED_EXT}"

        # Build library if missing
        if not static_lib.exists() and not shared_lib.exists():
            SHELL.warn(f"No existing libraries for '{lib_name}' found — building shared version.")
            built_lib = self.build_library(
                lib_name,
                CompileTools.make_static_lib if is_static else CompileTools.make_shared_lib,
                static_lib if is_static else shared_lib
            )

        if not built_lib.exists():
            SHELL.error(f"Failed to locate or build library for '{lib_name}'")
            sys.exit(1)

        SHELL.info(f"Linking library: {built_lib}")

        if is_static:
            SHELL.run([CXX, *CXXFLAGS, "-o", str(self.test_bin), str(self.test_src), str(static_lib)])
        else:
            SHELL.run([CXX, *CXXFLAGS, "-Ltest", f"-l{lib_name}", "-o", str(self.test_bin), *map(str, self.test_src)])

    def parse_args(self) -> Command:
        return Command(
            cmd=sys.argv[1] if len(sys.argv) > 1 and not sys.argv[1].startswith("--") else "",
            arg=sys.argv[2] if len(sys.argv) > 2 and not sys.argv[1].startswith("--") else "",
            run='--r' in sys.argv,
            clean='--c' in sys.argv,
            static='--static' in sys.argv,
        )

    def run(self):
        command: Command = self.parse_args()

        if command.is_clean: CompileTools.clean()

        match command.cmd:
            case 'h': SHELL.usage() # help
            case 't': self.build_test(command.arg)
            case 'f': self.build_final(command.arg, command.is_static)
            case 'a': pass
            case _: pass

        if command.is_run: SHELL.run(["./" + str(self.test_bin)])

if __name__ == "__main__":
    BuildSystem().run()