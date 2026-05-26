#!/usr/bin/env python3
"""
build.py — Sharp compiler build script (no CMake dependency).

Uses the bundled zig compiler at   zig/zig.exe (Windows) / zig/zig (POSIX)
to compile all targets: sharpc, probe_cpp, probe_sharp.

Usage:
    python3 build.py              # build all targets
    python3 build.py sharpc       # build sharpc only
"""

import subprocess
import sys
import os
from pathlib import Path

ROOT = Path(__file__).resolve().parent

# ── zig path ───────────────────────────────────────────────────────────
if sys.platform == "win32":
    ZIG = str(ROOT / "zig" / "zig.exe")
    OBJ_EXT = ".obj"
    EXE_EXT = ".exe"
    LINK_FLAGS = ["-lshlwapi"]
else:
    ZIG = str(ROOT / "zig" / "zig")
    OBJ_EXT = ".o"
    EXE_EXT = ""
    LINK_FLAGS = []

BUILD_DIR = ROOT / "build"

CPP_SOURCES = [
    "sharp-cpp/cpp.c",
    "sharp-cpp/cpp_sys_paths.c",
    "sharp-cpp/cpp_targets.c",
    "sharp-cpp/directive.c",
    "sharp-cpp/expr.c",
    "sharp-cpp/macro.c",
    "sharp-cpp/pptok.c",
]

SHARP_SOURCES = [
    "sharp-fe/ast.c",
    "sharp-fe/cg.c",
    "sharp-fe/lex.c",
    "sharp-fe/parse.c",
    "sharp-fe/scope.c",
    "sharp-fe/sema.c",
    "sharp-fe/sharpc.c",
    "sharp-fe/type.c",
]

SHARPC_SOURCES = CPP_SOURCES + SHARP_SOURCES

CFLAGS = [
    "-std=c11",
    "-O2",
    "-Wall", "-Wextra", "-Wno-deprecated-declarations",
    "-DNDEBUG",
    '-DSHARP_VERSION="1.0.0"',
    "-I", str(ROOT / "sharp-fe"),
    "-I", str(ROOT / "sharp-cpp"),
]


def run(cmd, **kw):
    r = subprocess.run(cmd, **kw)
    if r.returncode != 0:
        sys.exit(r.returncode)
    return r


def compile_obj(src_rel, cflags_extra=None):
    """Compile a single .c → .obj/.o.  Returns path to object file."""
    src = ROOT / src_rel
    obj = BUILD_DIR / (src_rel.replace("/", "_").replace("\\", "_") + OBJ_EXT)
    obj.parent.mkdir(parents=True, exist_ok=True)

    if obj.is_file() and obj.stat().st_mtime > src.stat().st_mtime:
        return obj  # up to date

    flags = CFLAGS.copy()
    if cflags_extra:
        flags.extend(cflags_extra)
    cmd = [ZIG, "cc", "-c", str(src), "-o", str(obj)] + flags
    print(f"  CC {src_rel}")
    run(cmd, capture_output=True, text=True)
    return obj


def link_exe(objs, out_rel, extra_flags=None):
    """Link object files into an executable."""
    out = BUILD_DIR / out_rel
    flags = list(LINK_FLAGS)
    if extra_flags:
        flags.extend(extra_flags)
    cmd = [ZIG, "cc"] + [str(o) for o in objs] + ["-o", str(out)] + flags
    print(f"  LD {out_rel}")
    run(cmd, capture_output=True, text=True)


def build_sharpc():
    print("[build] sharpc")
    objs = [compile_obj(s) for s in SHARPC_SOURCES]
    name = "sharpc" + EXE_EXT
    link_exe(objs, name)
    return BUILD_DIR / name


def build_probe_cpp():
    print("[build] probe_cpp")
    sources = CPP_SOURCES + ["sharp-cpp/probe.c"]
    objs = [compile_obj(s) for s in sources]
    name = "probe_cpp" + EXE_EXT
    link_exe(objs, name)
    return BUILD_DIR / name


def build_probe_sharp():
    print("[build] probe_sharp")
    sources = CPP_SOURCES + ["sharp-test/probe.c"]
    objs = [compile_obj(s) for s in sources]
    name = "probe_sharp" + EXE_EXT
    link_exe(objs, name)
    return BUILD_DIR / name


def build_all():
    build_sharpc()
    build_probe_cpp()
    build_probe_sharp()
    print("[build] all done.")


if __name__ == "__main__":
    target = sys.argv[1] if len(sys.argv) > 1 else "all"
    if target == "sharpc":
        build_sharpc()
    elif target == "all":
        build_all()
    else:
        print(f"Usage: python3 build.py [sharpc|all]")
        sys.exit(1)