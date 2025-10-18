import subprocess, platform, sys
from pathlib import Path
import colorama

# --- Init Colorama ---
colorama.init(autoreset=True)

# --- Logging helpers ---
def info(msg: str): print(f"{colorama.Fore.CYAN}[INFO]{colorama.Style.RESET_ALL} {msg}")
def ok(msg: str): print(f"{colorama.Fore.GREEN}[OK]{colorama.Style.RESET_ALL} {msg}")
def warn(msg: str): print(f"{colorama.Fore.YELLOW}[WARN]{colorama.Style.RESET_ALL} {msg}")
def error(msg: str): print(f"{colorama.Fore.RED}[ERR]{colorama.Style.RESET_ALL} {msg}")
def cmd_echo(cmd: list[str]): print(f"{colorama.Fore.BLUE}$ {' '.join(cmd)}{colorama.Style.RESET_ALL}")

# --- Config ---
CXX = "g++"
CXXFLAGS = ["-std=c++20", "-O3", "-Wall", "-Wextra", "-I."]
AR = "ar"
ARFLAGS = ["rcs"]

# --- Platform ---
if platform.system() == "Windows":
    shared_ext, shared_flags = "dll", ["-shared"]
else:
    shared_ext, shared_flags = "so", ["-fPIC", "-shared"]

LIB_TYPE = {"static", "shared"}

# --- Helpers ---
def run(cmd: list[str]):
    cmd_echo(cmd)
    subprocess.run(cmd, check=True)

def compile_sources(srcs: list[Path]) -> list[Path]:
    objs: list[Path] = []
    for src in srcs:
        if not src.exists():
            warn(f"Skipping missing file: {src}")
            continue
        obj = src.with_suffix(".o")
        run([CXX, *CXXFLAGS, "-c", str(src), "-o", str(obj)])
        objs.append(obj)
    return objs

def make_static(name: str, objs: list[Path]):
    run([AR, *ARFLAGS, f"libwarp_{name}.a", *map(str, objs)])
    ok(f"Built libwarp_{name}.a")

def make_shared(name: str, objs: list[Path]):
    run([CXX, *CXXFLAGS, *shared_flags, "-o", f"libwarp_{name}.{shared_ext}", *map(str, objs)])
    ok(f"Built libwarp_{name}.{shared_ext}")

def ensure_sources_exist(lib: str):
    folder = Path(f"warp_{lib}")
    if lib != "toolkit" and not folder.exists():
        error(f"Missing source folder: {folder}")
        sys.exit(1)

def log_usage():
    print(f"{colorama.Style.BRIGHT}Usage:{colorama.Style.RESET_ALL}")
    print("  python build.py [lib name after 'warp_'] [static|shared]  → build specific library")
    print("  python build.py toolkit [static|shared]                   → build combined library")
    print("  python build.py clean                                     → remove build artifacts")

def clean():
    exts = [".o", ".a", f".{shared_ext}"]
    for path in Path(".").rglob("*"):
        if path.suffix in exts and path.is_file():
            info(f"Removing: {path}")
            path.unlink()
    ok("Clean complete.")
    sys.exit(0)

# --- Entry ---
if __name__ == "__main__":
    if len(sys.argv) < 2:
        log_usage()
        sys.exit(1)

    if sys.argv[1] == "clean":
        clean()

    if len(sys.argv) != 3 or sys.argv[2] not in LIB_TYPE:
        log_usage()
        sys.exit(1)

    lib, kind = sys.argv[1], sys.argv[2]
    ensure_sources_exist(lib)

    objs = compile_sources(
        [
            cpp for folder in Path(".").glob("warp_*") if folder.is_dir()
            for cpp in folder.glob("*.cpp")
        ]
    ) if lib == "toolkit" else compile_sources(list(Path(f"warp_{lib}").glob("*.cpp")))
    
    if not objs:
        error(f"No source files found for '{lib}'")
        sys.exit(1)

    (make_static if kind == "static" else make_shared)(lib, objs)