import subprocess
import platform
import sys
from pathlib import Path
import colorama


# Colorized Shell Utilities
class ShellLink:
    def __init__(self):
        colorama.init(autoreset=True)

    @staticmethod
    def __msg_helper__(color: str, pre: str, msg: str):
        print(f"{color}{pre}{colorama.Style.RESET_ALL}: {msg}")

    @staticmethod
    def __usage_helper__(cmd: str, desc: str):
        print(f"  python build.py {colorama.Style.BRIGHT}{cmd}{colorama.Style.RESET_ALL} → {desc}")

    # Logging methods
    def info(self, msg: str):
        self.__msg_helper__(colorama.Fore.CYAN, "[INFO]", msg)

    def ok(self, msg: str):
        self.__msg_helper__(colorama.Fore.GREEN, "[OK]", msg)

    def warn(self, msg: str):
        self.__msg_helper__(colorama.Fore.YELLOW, "[WARN]", msg)

    def error(self, msg: str):
        self.__msg_helper__(colorama.Fore.RED, "[ERR]", msg)

    def cmd_echo(self, cmd: list[str]):
        self.__msg_helper__(colorama.Fore.BLUE, "$", " ".join(cmd))

    def run(self, cmd: list[str]):
        self.cmd_echo(cmd)
        try:
            subprocess.run(cmd, check=True)
        except subprocess.CalledProcessError as e:
            self.error(f"Command failed: {e}")
            sys.exit(1)

    def log_usage(self):
        print(f"{colorama.Style.BRIGHT}Usage:{colorama.Style.RESET_ALL}")
        self.__usage_helper__("[lib name after 'warp_'] [static|shared]", "build specific library")
        self.__usage_helper__("toolkit [static|shared]", "build combined library")
        self.__usage_helper__("clean", "remove build artifacts")
        sys.exit(0)


# Compiler / Linker Configuration
class CommandConfig:
    CXX = "g++"
    CXXFLAGS = ["-std=c++20", "-O3", "-Wall", "-Wextra", "-I."]
    AR = "ar"
    ARFLAGS = ["rcs"]
    LIB_TYPE = {"static", "shared"}

    def __init__(self, shell: ShellLink):
        self.shell = shell

        if platform.system() == "Windows":
            self.shared_ext = "dll"
            self.shared_flags = ["-shared"]
        else:
            self.shared_ext = "so"
            self.shared_flags = ["-fPIC", "-shared"]

    # Compile source files
    def compile_sources(self, srcs: list[Path]) -> list[Path]:
        objs: list[Path] = []

        for src in srcs:
            if not src.exists():
                self.shell.warn(f"Skipping missing file: {src}")
                continue

            obj = src.with_suffix(".o")
            self.shell.info(f"Compiling {src} → {obj}")
            self.shell.run([self.CXX, *self.CXXFLAGS, "-c", str(src), "-o", str(obj)])
            objs.append(obj)

        return objs

    # Build static library
    def make_static(self, name: str, objs: list[Path]):
        out = f"libwarp_{name}.a"
        self.shell.run([self.AR, *self.ARFLAGS, out, *map(str, objs)])
        self.shell.ok(f"Built {out}")

    # Build shared library
    def make_shared(self, name: str, objs: list[Path]):
        out = f"libwarp_{name}.{self.shared_ext}"
        self.shell.run([self.CXX, *self.CXXFLAGS, *self.shared_flags, "-o", out, *map(str, objs)])
        self.shell.ok(f"Built {out}")

    # Validate presence of source folder
    def ensure_sources_exist(self, lib: str):
        folder = Path(f"warp_{lib}")
        if lib != "toolkit" and not folder.exists():
            self.shell.error(f"Missing source folder: {folder}")
            sys.exit(1)

    # Remove artifacts
    def clean(self):
        exts = [".o", ".a", f".{self.shared_ext}"]
        for path in Path(".").rglob("*"):
            if path.suffix in exts and path.is_file():
                self.shell.info(f"Removing: {path}")
                path.unlink()
        self.shell.ok("Clean complete.")
        sys.exit(0)


# Parses and executes build commands
class BuildSystem:
    def __init__(self):
        self.shell = ShellLink()
        self.cfg = CommandConfig(self.shell)

    def parse_args(self):
        args = sys.argv

        # No args or help → show usage
        if len(args) < 2 or args[1] == "help":
            self.shell.log_usage()

        cmd = args[1]

        # Clean command → immediate action
        if cmd == "clean":
            self.cfg.clean()

        # Validate argument count
        if len(args) != 3 or args[2] not in self.cfg.LIB_TYPE:
            self.shell.log_usage()

        return args[1], args[2]

    def collect_sources(self, lib: str) -> list[Path]:
        if lib == "toolkit":
            # Collect all warp_* source files
            return [
                cpp
                for folder in Path(".").glob("warp_*")
                if folder.is_dir()
                for cpp in folder.glob("*.cpp")
            ]
        else:
            folder = Path(f"warp_{lib}")
            return list(folder.glob("*.cpp"))

    def build(self):
        lib, kind = self.parse_args()
        self.cfg.ensure_sources_exist(lib)

        srcs = self.collect_sources(lib)
        if not srcs:
            self.shell.error(f"No source files found for '{lib}'")
            sys.exit(1)

        objs = self.cfg.compile_sources(srcs)
        if not objs:
            self.shell.error(f"Failed to produce any object files for '{lib}'")
            sys.exit(1)

        if kind == "static":
            self.cfg.make_static(lib, objs)
        else:
            self.cfg.make_shared(lib, objs)


if __name__ == "__main__":
    BuildSystem().build()
