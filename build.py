import sys
import subprocess
import platform
from pathlib import Path
import colorama


# --- Shell and Logging ---
class ShellLink:
    def __init__(self):
        colorama.init(autoreset=True)

    @staticmethod
    def _msg(color: str, tag: str, msg: str):
        print(f"{color}{tag}{colorama.Style.RESET_ALL}: {msg}")

    @staticmethod
    def _usage(cmd: str, desc: str):
        print(f"  python build.py {colorama.Style.BRIGHT}{cmd}{colorama.Style.RESET_ALL} → {desc}")

    # Log levels
    def info(self, msg: str): self._msg(colorama.Fore.CYAN, "[INFO]", msg)
    def ok(self, msg: str): self._msg(colorama.Fore.GREEN, "[OK]", msg)
    def warn(self, msg: str): self._msg(colorama.Fore.YELLOW, "[WARN]", msg)
    def error(self, msg: str): self._msg(colorama.Fore.RED, "[ERR]", msg)

    def run(self, cmd: list[str]):
        print(f"{colorama.Fore.BLUE}$ {' '.join(cmd)}{colorama.Style.RESET_ALL}")
        try:
            subprocess.run(cmd, check=True)
        except subprocess.CalledProcessError as e:
            self.error(f"Command failed with exit code {e.returncode}")
            sys.exit(e.returncode)

    def usage(self):
        print(f"{colorama.Style.BRIGHT}Building Tools:{colorama.Style.RESET_ALL}")
        self._usage("[lib name after 'warp_'] [static|shared]", "build specific library")
        self._usage("toolkit [static|shared]", "build combined library")
        self._usage("clean", "remove build artifacts")
        print(f"{colorama.Style.BRIGHT}Testing Tools:{colorama.Style.RESET_ALL}")
        self._usage("test [warp_libname]", "compile & run test with library source")
        self._usage("final [warp_libname]", "compile & run test linked against built library")
        sys.exit(0)


# --- Compiler / Linker Config ---
class CommandConfig:
    def __init__(self, shell: ShellLink):
        self.shell = shell
        self.CXX = "g++"
        self.CXXFLAGS = ["-std=c++20", "-O3", "-Wall", "-Wextra", "-I."]
        self.AR = "ar"
        self.ARFLAGS = ["rcs"]
        self.LIB_TYPE = {"static", "shared"}

        match platform.system():
            case "Windows": self.shared_ext, self.shared_flags = "dll", ["-shared"]
            case "Darwin": self.shared_ext, self.shared_flags = "dylib", ["-fPIC", "-shared"]
            case _: self.shared_ext, self.shared_flags = "so", ["-fPIC", "-shared"]

    # --- Core commands ---
    def compile(self, src: Path, out: Path):
        self.shell.info(f"Compiling {src} → {out}")
        self.shell.run([self.CXX, *self.CXXFLAGS, "-c", str(src), "-o", str(out)])

    def link_static(self, name: str, objs: list[Path]) -> Path:
        out = Path(f"libwarp_{name}.a")
        self.shell.run([self.AR, *self.ARFLAGS, str(out), *map(str, objs)])
        self.shell.ok(f"Built {out}")
        return out

    def link_shared(self, name: str, objs: list[Path]) -> Path:
        out = Path(f"libwarp_{name}.{self.shared_ext}")
        self.shell.run([self.CXX, *self.CXXFLAGS, *self.shared_flags, "-o", str(out), *map(str, objs)])
        self.shell.ok(f"Built {out}")
        return out

    def clean(self):
        exts = [".o", ".a", f".{self.shared_ext}"]
        for path in Path(".").rglob("*"):
            if path.suffix in exts:
                self.shell.info(f"Removing: {path}")
                path.unlink()
        self.shell.ok("Clean complete.")
        sys.exit(0)


# --- Build System ---
class BuildSystem:
    def __init__(self):
        self.shell = ShellLink()
        self.cfg = CommandConfig(self.shell)
        self.test_src = Path("test/main.cpp")
        self.test_bin = Path("test/prog")

    def parse_args(self):
        args = sys.argv
        if len(args) < 2 or args[1] == "help":
            self.shell.usage()
        return args[1], args[2] if len(args) > 2 else None

    # --- Helpers ---
    def collect_sources(self, lib: str) -> list[Path]:
        folder = Path(f"warp_{lib}")
        return [
            cpp for f in Path(".").glob("warp_*") if f.is_dir() for cpp in f.glob("*.cpp")
        ] if lib == "toolkit" else list(folder.glob("*.cpp"))

    def compile_sources(self, lib: str) -> list[Path]:
        srcs = self.collect_sources(lib)
        objs = []
        for src in srcs:
            obj = src.with_suffix(".o")
            self.cfg.compile(src, obj)
            objs.append(obj)

        return objs

    # --- Build Modes ---
    def build_library(self, lib: str, kind: str) -> Path:
        objs = self.compile_sources(lib)
        return self.cfg.link_static(lib, objs) if kind == "static" else self.cfg.link_shared(lib, objs)

    def build_test(self, lib_name: str):
        lib_src = Path(lib_name) / f"{lib_name}.cpp"
        if not lib_src.exists():
            self.shell.error(f"Missing library source: {lib_src}")
            sys.exit(1)
        cmd = [self.cfg.CXX, *self.cfg.CXXFLAGS, "-o", str(self.test_bin), str(self.test_src), str(lib_src)]
        self.shell.run(cmd)
        self.shell.run(["./" + str(self.test_bin)])

    def build_final(self, lib_name: str):
        lib_base = lib_name.removeprefix("warp_")
        test_dir = Path("test")
        test_dir.mkdir(exist_ok=True)

        static_lib = test_dir / f"libwarp_{lib_base}.a"
        shared_lib = test_dir / f"libwarp_{lib_base}.{self.cfg.shared_ext}"

        # --- auto build missing library ---
        if not static_lib.exists() and not shared_lib.exists():
            self.shell.warn(f"No existing libraries for '{lib_name}' found — building shared version.")
            built = self.build_library(lib_base, "shared")
            built.replace(shared_lib)

        # --- prefer static, fallback to shared ---
        if static_lib.exists():
            self.shell.info(f"Linking static library: {static_lib}")
            cmd = [self.cfg.CXX, *self.cfg.CXXFLAGS, "-o", str(self.test_bin), str(self.test_src), str(static_lib)]
        elif shared_lib.exists():
            self.shell.info(f"Linking shared library: {shared_lib}")
            cmd = [
                self.cfg.CXX,
                *self.cfg.CXXFLAGS,
                "-Ltest",
                f"-lwarp_{lib_base}",
                "-o",
                str(self.test_bin),
                str(self.test_src),
                "-Wl,-rpath,./test",
            ]
        else:
            self.shell.error(f"Failed to locate or build library for '{lib_name}'")
            sys.exit(1)

        self.shell.run(cmd)
        self.shell.run(["./" + str(self.test_bin)])

    # --- Entrypoint ---
    def run(self):
        cmd, arg = self.parse_args()

        if cmd == "clean":
            self.cfg.clean()

        if cmd in {"test", "final"}:
            if not arg or not arg.startswith("warp_"):
                self.shell.usage()
            (self.build_test if cmd == "test" else self.build_final)(arg)
            return

        if not arg or arg not in self.cfg.LIB_TYPE:
            self.shell.usage()

        self.build_library(cmd, arg)


# --- Main Entry ---
if __name__ == "__main__":
    BuildSystem().run()