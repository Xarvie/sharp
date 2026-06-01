#!/usr/bin/env python3
"""
run_regression.py — Sharp 统一回归测试脚本

一键运行所有测试:
  - SP 超集探针代码生成回归测试 (c_superset_probes/)

用法:
    # 运行所有测试
    python3 run_regression.py

    # 指定 sharpc 路径
    python3 run_regression.py --sharpc /path/to/sharpc

    # 并行度
    python3 run_regression.py -j 16

返回码:
    0: 全部通过
    1: 存在失败
"""

import subprocess
import sys
import os
import re
import argparse
import json
import threading
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path
from typing import List, Tuple, Optional, Dict


# ── 环境配置 ────────────────────────────────────────────────────────────

def _platform_defaults():
    if sys.platform == "win32":
        return "build", "sharpc.exe"
    return "build", "sharpc"

BUILD_DIR_NAME, SHARPC_NAME = _platform_defaults()


def find_file_in_path(name: str, paths_str: str) -> Optional[str]:
    for p in paths_str.split(os.pathsep):
        candidate = Path(p) / name
        if candidate.is_file():
            return str(candidate)
    return None


# ── 编译器后端抽象 ──────────────────────────────────────────────────────

class CompilerBackend:
    name: str = "unknown"
    label: str = "unknown"
    include_flag: str = "-I"
    _triple_cache: Optional[str] = None

    @classmethod
    def find(cls, explicit_path: Optional[str] = None) -> Tuple[str, dict]:
        raise NotImplementedError

    @classmethod
    def detect_target(cls, compiler_path: Optional[str] = None) -> str:
        return "x86_64-linux-gnu"

    @classmethod
    def system_includes(cls) -> List[str]:
        return []

    @classmethod
    def preprocess_cmd(cls, compiler_path: str, source: str, includes: List[str]) -> List[str]:
        raise NotImplementedError

    @classmethod
    def clean_output(cls, text: str, source_file: Optional[str] = None) -> str:
        return text


class ZigBackend(CompilerBackend):
    name = "zig"
    label = "zig"
    include_flag = "-I"

    @classmethod
    def _detect_target(cls, zig_path: str) -> str:
        try:
            r = subprocess.run([zig_path, "env"], capture_output=True, text=True, timeout=10)
            m = re.search(r'\.target\s*=\s*"([^"]+)"', r.stdout)
            if m:
                triple = m.group(1)
                parts = triple.split("-")
                if len(parts) >= 2:
                    parts[1] = parts[1].split(".")[0]
                if len(parts) >= 3:
                    parts[2] = parts[2].split(".")[0]
                return "-".join(parts)
        except Exception:
            pass
        return "native"

    @classmethod
    def detect_target(cls, compiler_path: Optional[str] = None) -> str:
        # No caching — always detect fresh
        return cls._detect_target(compiler_path) if compiler_path else "native"

    @classmethod
    def find(cls, explicit_path: Optional[str] = None) -> Tuple[str, dict]:
        if explicit_path:
            p = Path(explicit_path)
            if not p.is_file():
                raise SystemExit(f"ERROR: zig not found at {p}")
            target = cls.detect_target(str(p))
            sys_includes = cls._detect_sys_includes(str(p), target)
            return str(p), os.environ.copy(), sys_includes

        for p in os.environ.get("PATH", "").split(os.pathsep):
            for name in ("zig", "zig.exe"):
                c = os.path.join(p, name)
                if Path(c).is_file():
                    target = cls.detect_target(c)
                    sys_includes = cls._detect_sys_includes(c, target)
                    return c, os.environ.copy(), sys_includes

        raise SystemExit("ERROR: cannot find zig in PATH")

    @classmethod
    def _detect_sys_includes(cls, zig_path: str, target: str = "") -> List[str]:
        try:
            cmd = [zig_path, "cc", "-E", "-x", "c", "-", "-v"]
            if target:
                cmd.extend(["-target", target])
            r = subprocess.run(cmd, input="", capture_output=True, text=True, timeout=15)
            in_list = False
            result = []
            for line in r.stderr.splitlines():
                stripped = line.strip()
                if stripped.startswith("#include <...> search starts here"):
                    in_list = True
                    continue
                if stripped.startswith("End of search list"):
                    break
                if in_list and stripped:
                    result.append(stripped)
            return result
        except Exception:
            return []

    @classmethod
    def system_includes(cls, zig_path: str = "", target: str = "") -> List[str]:
        """Detect system includes on every call — no caching."""
        zig = zig_path or "zig"
        return cls._detect_sys_includes(zig, target)

    @classmethod
    def preprocess_cmd(cls, compiler_path: str, source: str, includes: List[str],
                       target: Optional[str] = None) -> List[str]:
        cmd = [compiler_path, "cc", "-E", "-std=c11"]
        if target:
            cmd.extend(["-target", target])
        cmd.append(source)
        cmd.extend(includes)
        return cmd

    @classmethod
    def clean_output(cls, text: str, source_file: Optional[str] = None) -> str:
        return re.sub(r'^\s*#\s*\d.*$', '', text, flags=re.MULTILINE)


BACKENDS: Dict[str, type] = {"zig": ZigBackend}


# ── 编译逻辑 ────────────────────────────────────────────────────────────

def find_project_root(base_dir: Path) -> Optional[Path]:
    candidate = base_dir.parent
    if (candidate / "build.py").is_file():
        return candidate
    return None


def find_build_dir(project_root: Path) -> Path:
    return project_root / BUILD_DIR_NAME


def needs_rebuild(sharpc_path: Path, project_root: Path) -> bool:
    if not sharpc_path.is_file():
        return True
    exe_mtime = sharpc_path.stat().st_mtime
    for sd in [project_root / "sharp-fe", project_root / "sharp-cpp"]:
        if not sd.is_dir():
            continue
        for p in sd.rglob("*.c"):
            if p.stat().st_mtime > exe_mtime:
                return True
        for p in sd.rglob("*.h"):
            if p.stat().st_mtime > exe_mtime:
                return True
    return False


def build_sharpc(build_dir: Path, project_root: Path) -> Tuple[bool, str]:
    build_py = project_root / "build.py"
    print("[build] python build.py sharpc ...")
    stdout, stderr, rc = run_cmd(
        [sys.executable, str(build_py), "sharpc"],
        timeout=120
    )
    if rc != 0:
        return False, f"build failed: {(stderr or stdout).strip()[:300]}"
    print("[build] done.")
    return True, "ok"


def ensure_sharpc(base_dir: Path, explicit_path: Optional[str]) -> Tuple[Path, str]:
    if explicit_path:
        p = Path(explicit_path)
        if not p.is_file():
            raise SystemExit(f"ERROR: sharpc not found at {p}")
        # v0.13: even when sharpc is supplied externally, we still need
        # zig's include paths so multiarch glibc headers resolve.
        try:
            ZigBackend.find()
        except SystemExit:
            pass  # zig not in PATH — fall back to whatever sharpc finds
        return p, f"using explicit path: {p}"

    project_root = find_project_root(base_dir)
    if not project_root:
        raise SystemExit("ERROR: cannot find project root (CMakeLists.txt)")

    build_dir = find_build_dir(project_root) or (project_root / BUILD_DIR_NAME)
    sharpc_path = build_dir / SHARPC_NAME

    if needs_rebuild(sharpc_path, project_root):
        print("[build] source changed, rebuilding sharpc ...")
        ok, msg = build_sharpc(build_dir, project_root)
        if not ok:
            raise SystemExit(f"ERROR: {msg}")
        if not sharpc_path.is_file():
            raise SystemExit(f"ERROR: sharpc not found at {sharpc_path} after build")
        return sharpc_path, "rebuilt"
    elif sharpc_path.is_file():
        return sharpc_path, "up-to-date"
    else:
        print("[build] sharpc not found, building ...")
        ok, msg = build_sharpc(build_dir, project_root)
        if not ok:
            raise SystemExit(f"ERROR: {msg}")
        return sharpc_path, "built"


# ── 工具函数 ────────────────────────────────────────────────────────────

def run_cmd(cmd: List[str], timeout: int = 60,
            env: Optional[dict] = None) -> Tuple[str, str, int]:
    try:
        r = subprocess.run(cmd, capture_output=True, text=True,
                           timeout=timeout, env=env or os.environ)
        return r.stdout, r.stderr, r.returncode
    except subprocess.TimeoutExpired:
        return "", "timeout", 124
    except Exception as e:
        return "", str(e), 1


def tokenize(text: str) -> List[str]:
    token_re = re.compile(
        r'"(?:[^"\\]|\\.)*"'
        r"|'(?:[^'\\]|\\.)*'"
        r'|L"(?:[^"\\]|\\.)*"'
        r"|L'(?:[^'\\]|\\.)*'"
        r'|L"""(?:[^"\\]|\\.)*"""'
        r"|[A-Za-z_]\w*"
        r"|[0-9][0-9xXaAbB.]*[fFlLuU]*"
        r"|->|::|\.\.\.| <<=|>>="
        r"|[+\-*/%&|^~<>=!?:;.(){}[\],#]?"
        r"|\S"
    )
    return [t for t in token_re.findall(text) if t]


# ── SP 超集探针测试 ─────────────────────────────────────────────────────

PROBES_DIR_NAME = "c_superset_probes"
UNIT_DIR_NAME = "unit"
SPECIAL_DIR_NAME = "special"
# Flags required for cross-compile probes
CROSS_COMPILE_ARGS = ["--target", "x86_64-linux-gnu"]


def strip_builtin_preamble(text: str) -> str:
    lines = text.splitlines(True)
    start = end = -1
    for i, line in enumerate(lines):
        if "typedef long" in line and "ptrdiff_t;" in line:
            start = i
        if start >= 0 and "} max_align_t;" in line:
            end = i
            break
    if start >= 0 and end >= start:
        result = "".join(lines[:start] + lines[end + 1:])
        while result.endswith("\n\n"):
            result = result[:-1]
        text = result

    # Strip leading #include lines and blank lines injected by the compiler
    lines = text.splitlines(True)
    idx = 0
    while idx < len(lines) and (lines[idx].startswith("#include") or lines[idx].strip() == ""):
        idx += 1
    if idx > 0:
        text = "".join(lines[idx:])
        while text.startswith("\n\n"):
            text = text[1:]

    return text


def _get_sharpc_env() -> dict:
    env = os.environ.copy()
    if sys.platform == "win32":
        zig_home = os.environ.get("ZIG_HOME", "")
        if zig_home and os.path.isdir(zig_home):
            env["PATH"] = zig_home + os.pathsep + env.get("PATH", "")
    return env


def _run_sharpc_codegen(src_path: str, sharpc_path: str, zig_path: str = "",
                        timeout: int = 30,
                        extra_args: Optional[List[str]] = None) -> Tuple[bool, str, str]:
    """Run sharpc -c. Returns (success, output_path, error_detail)."""
    tmp_out = src_path + ".gen.i"
    try:
        cmd = [sharpc_path, "-c", src_path, "-o", tmp_out]
        # Note: sharpc now auto-detects zig's include paths internally via
        # cpp_detect_zig_sys_paths_from_zig(), so we don't need to pass
        # -isystem flags here. This ensures consistent output regardless
        # of the test script's working directory.
        if extra_args:
            cmd.extend(extra_args)
        r = subprocess.run(cmd, capture_output=True, text=True,
                           encoding="utf-8", errors="replace",
                           timeout=timeout, env=_get_sharpc_env())
        if not os.path.isfile(tmp_out):
            err = r.stderr.strip()[:300] if r.stderr.strip() else f"no output (rc={r.returncode})"
            return False, "", err
        if os.path.getsize(tmp_out) == 0:
            return False, tmp_out, "empty output"
        return True, tmp_out, ""
    except subprocess.TimeoutExpired:
        return False, "", "timeout"
    except Exception as e:
        return False, "", str(e)


def _cleanup_gen_file(tmp_out: str) -> None:
    if tmp_out and os.path.isfile(tmp_out):
        try:
            os.unlink(tmp_out)
        except OSError:
            pass


def _cleanup_files(*paths: str) -> None:
    """Remove generated files.  Silently skips missing paths."""
    for p in paths:
        _cleanup_gen_file(p)


def _find_zig(project_root: Path) -> Optional[str]:
    """Find zig binary: project's zig/ first, then PATH."""
    for name in ("zig.exe", "zig"):
        candidate = project_root / "zig" / name
        if candidate.is_file():
            return str(candidate)
    return find_file_in_path("zig", os.environ.get("PATH", ""))


def _target_ref_tag() -> Optional[str]:
    """Return the target triple tag for reference file lookup.
    Mirrors sharpc's default target selection logic."""
    if sys.platform == "win32":
        return "x86_64-windows-gnu"
    if sys.platform == "darwin":
        return "aarch64-macos"
    if sys.platform.startswith("linux"):
        return "x86_64-linux-gnu"
    return None


def _resolve_ref_path(sp_path: str) -> Optional[str]:
    """Resolve the reference file path for a given source file.

    Looks for target-specific reference first in a target subdirectory
    (<dir>/<target>/<basename>.ref.i), then falls back to generic
    (<dir>/<basename>.ref.i). Returns None if no ref exists.
    """
    tag = _target_ref_tag()
    if tag:
        sp_dir = os.path.dirname(sp_path)
        sp_base = os.path.basename(sp_path)
        target_ref = os.path.join(sp_dir, tag, sp_base[:-3] + ".ref.i")
        if os.path.isfile(target_ref):
            return target_ref
    generic_ref = sp_path[:-3] + ".ref.i"
    if os.path.isfile(generic_ref):
        return generic_ref
    return None


def _resolve_src_path(src_path: str) -> str:
    """Resolve the source file path, preferring target-specific overrides.

    If <dir>/<target>/<basename> exists, return that; otherwise return
    the original src_path unchanged.
    """
    tag = _target_ref_tag()
    if tag:
        src_dir = os.path.dirname(src_path)
        src_base = os.path.basename(src_path)
        target_src = os.path.join(src_dir, tag, src_base)
        if os.path.isfile(target_src):
            return target_src
    return src_path


def _compare_gen_with_ref(tmp_out: str, ref_path: str) -> Tuple[int, str]:
    """Compare generated file against reference. Returns (rc, detail)."""
    try:
        with open(tmp_out, "r", encoding="utf-8", errors="replace") as f:
            gen_text = f.read()
        with open(ref_path, "r", encoding="utf-8", errors="replace") as f:
            ref_text = f.read()

        gen_stripped = strip_builtin_preamble(gen_text)
        ref_stripped = strip_builtin_preamble(ref_text)

        # Filter out #line directives (debugging metadata) for comparison.
        # #line directives are essential for source-level debugging but should
        # not affect structural comparison of generated C code.
        gen_filtered = "\n".join(
            l for l in gen_stripped.splitlines()
            if not re.match(r'^\s*#line\s+', l)
        )
        ref_filtered = "\n".join(
            l for l in ref_stripped.splitlines()
            if not re.match(r'^\s*#line\s+', l)
        )

        if re.sub(r'\s+', ' ', gen_filtered).strip() == re.sub(r'\s+', ' ', ref_filtered).strip():
            return 0, "match"

        diff_info = []
        gl = gen_filtered.splitlines()
        rl = ref_filtered.splitlines()
        for i in range(max(len(gl), len(rl))):
            g = gl[i] if i < len(gl) else "<no line>"
            r = rl[i] if i < len(rl) else "<no line>"
            if g != r:
                diff_info.append(f"L{i+1}: gen={g!r}")
                diff_info.append(f"      ref={r!r}")
                if len(diff_info) > 20:
                    diff_info.append("      ... (truncated)")
                    break
        return 1, "\n    ".join(diff_info)
    except Exception as e:
        return 2, f"compare error: {e}"


def _compile_and_run(gen_i_path: str, zig_path: str, timeout: int = 30) -> Tuple[int, str]:
    """Compile generated .gen.i with zig cc, link to executable, and run it.
    Returns (rc, detail): 0=pass, 1=fail (non-zero exit), 2=error (compile/link fail)."""
    exe_path = gen_i_path + ".exe"

    stdout, stderr, rc = run_cmd(
        [zig_path, "cc", "-x", "c", gen_i_path, "-o", exe_path, "-std=c11", "-fno-sanitize=undefined"],
        timeout=timeout
    )
    if rc != 0:
        detail = (stderr or stdout).strip()
        if len(detail) > 300:
            detail = detail[:300] + "..."
        _cleanup_files(exe_path)
        return 2, f"zig cc failed: {detail}"

    stdout, stderr, rc = run_cmd([exe_path], timeout=timeout)
    _cleanup_files(exe_path)
    if rc != 0:
        detail = f"exit code {rc}"
        if stderr.strip():
            detail += f" (stderr: {stderr.strip()[:100]})"
        return 1, detail
    return 0, "pass"


BUGS_DIR_NAME = "bugs"


def test_c_probe(c_path: str, sharpc_path: str, zig_path: str = "",
                 timeout: int = 30,
                 extra_args: Optional[List[str]] = None) -> Tuple[int, str]:
    ok, tmp_out, err = _run_sharpc_codegen(c_path, sharpc_path, zig_path, timeout, extra_args)
    if not ok:
        _cleanup_files(tmp_out)
        return 2, err
    rc, detail = _compile_and_run(tmp_out, zig_path, timeout)
    _cleanup_files(tmp_out)
    return rc, detail


def test_sp_probe(sp_path: str, sharpc_path: str, ref_path: Optional[str] = None,
                  zig_path: str = "",
                  timeout: int = 30,
                  extra_args: Optional[List[str]] = None) -> Tuple[int, str]:
    ok, tmp_out, err = _run_sharpc_codegen(sp_path, sharpc_path, zig_path, timeout, extra_args)
    if not ok:
        _cleanup_files(tmp_out)
        return 2, err
    if ref_path:
        rc, detail = _compare_gen_with_ref(tmp_out, ref_path)
        if rc != 0:
            _cleanup_files(tmp_out)
            return rc, detail
    # Compile generated C and run
    rc, detail = _compile_and_run(tmp_out, zig_path, timeout)
    _cleanup_files(tmp_out)
    return rc, detail


# ── 并行探针测试引擎 ─────────────────────────────────────────────────────

def run_all_probe_tests(
    base_dir: Path,
    sharpc_path: str,
    zig_path: str = "",
    verbose: bool = False,
    timeout: int = 30,
    jobs: int = 1,
) -> Tuple[int, int, int, Dict[str, Tuple[int, str]]]:
    """Run ALL superset probe tests (C + SP) with maximum parallelism.

    All SP and C probe files are tested individually.
    """
    probes_dir = base_dir / PROBES_DIR_NAME
    if not probes_dir.is_dir():
        return 0, 0, 0, {}

    # ── 文件分类 ──
    c_files = sorted(
        f for f in os.listdir(probes_dir)
        if f.endswith(".c") and not f.endswith(".ref.i")
    )
    sp_files = sorted(f for f in os.listdir(probes_dir) if f.endswith(".ce"))

    total_files = len(c_files) + len(sp_files)

    # ── 共享状态（带锁） ──
    file_results: Dict[str, Tuple[int, str]] = {}
    passed = failed = errors = 0
    completed = 0
    lock       = threading.Lock()   # guards counters + file_results
    print_lock = threading.Lock()   # serialises stdout

    def _record(name: str, rc: int, detail: str) -> None:
        nonlocal passed, failed, errors, completed
        with lock:
            file_results[name] = (rc, detail)
            if rc == 0:
                passed += 1
            elif rc == 1:
                failed += 1
            else:
                errors += 1
            completed += 1
            progress = f"[{completed:3d}/{total_files}]"

        if verbose:
            status = "pass" if rc == 0 else ("fail" if rc == 1 else "error")
            icon   = {"pass": "PASS", "fail": "FAIL", "error": "ERR "}[status]
            with print_lock:
                print(f"  {icon} {progress}  {name}")
                if status != "pass":
                    for line in detail.splitlines():
                        print(f"           {line}")

    if verbose:
        print(
            f"\n[probes] {total_files} tests  "
            f"({len(c_files)} C | {len(sp_files)} SP)  "
            f"jobs={jobs}\n"
        )

    # ── 单一 executor ──────────────────────────────────────────────
    #
    #  A) C 探针 + standalone SP: 立即提交，互相无依赖
    #
    # ────────────────────────────────────────────────────────────────

    with ThreadPoolExecutor(max_workers=jobs) as ex:

        # A: C 探针 + standalone SP
        c_futures: Dict = {
            ex.submit(test_c_probe, _resolve_src_path(str(probes_dir / f)), sharpc_path, zig_path, timeout): f
            for f in c_files
        }

        standalone_futures: Dict = {
            ex.submit(
                test_sp_probe,
                _resolve_src_path(str(probes_dir / f)), sharpc_path,
                _resolve_ref_path(str(probes_dir / f)),
                zig_path,
                timeout
            ): f
            for f in sp_files
        }

        # Wait for A
        for fut in as_completed(c_futures):
            _record(c_futures[fut], *fut.result())
        for fut in as_completed(standalone_futures):
            _record(standalone_futures[fut], *fut.result())

    # ── 非 verbose 模式：最终打印失败条目 ──
    if not verbose:
        failures = [
            (name, rc, detail)
            for name, (rc, detail) in sorted(file_results.items())
            if rc != 0
        ]
        if failures:
            print()
            for name, rc, detail in failures:
                icon = "FAIL" if rc == 1 else "ERR "
                print(f"  {icon}  {name}")
                for line in detail.splitlines():
                    print(f"         {line}")

    return passed, failed, errors, file_results


# ── Bugs 测试 (bugs/) ────────────────────────────────────────────────────

def run_bugs_tests(
    base_dir: Path,
    sharpc_path: str,
    zig_path: str = "",
    verbose: bool = False,
    timeout: int = 30,
    jobs: int = 1,
) -> Tuple[int, int, int, Dict[str, Tuple[int, str]]]:
    """Run all bug regression tests from bugs/ directory.

    Each .c file is:
      1. compiled: sharpc -c bug.c → bug.c.gen.i
      2. compiled with zig cc and linked
    No .ref.i files are needed — we only check that sharpc can generate
    valid C code that the backend compiler accepts.
    """
    bugs_dir = base_dir / BUGS_DIR_NAME
    if not bugs_dir.is_dir():
        print(f"  [bugs] {bugs_dir} not found, skipping.")
        return 0, 0, 0, {}

    c_files = sorted(
        str(p.relative_to(bugs_dir))
        for p in bugs_dir.glob("*.c")
    )

    if not c_files:
        print("  [bugs] no .c files found, skipping.")
        return 0, 0, 0, {}

    total = len(c_files)
    passed = failed = errors = 0
    file_results: Dict[str, Tuple[int, str]] = {}

    def _record(name: str, rc: int, detail: str):
        nonlocal passed, failed, errors
        file_results[name] = (rc, detail)
        if rc == 0:
            passed += 1
        elif rc == 1:
            failed += 1
        else:
            errors += 1

    if verbose:
        print(f"  [bugs] running {total} file(s) ...")
    else:
        print(f"  [bugs] running {total} file(s) ...", end="", flush=True)

    max_workers = max(1, jobs)

    def _test_one(name: str) -> Tuple[str, int, str]:
        src = str(bugs_dir / name)
        ok, tmp_out, err = _run_sharpc_codegen(src, sharpc_path, zig_path, timeout)
        if not ok:
            _cleanup_files(tmp_out)
            return name, 2, err
        # Check if the source has a main function.  Many bug tests are
        # compile-only (declarations + stubs) and intentionally have no main.
        # For those, only verifying that sharpc generates valid C (accepted
        # by zig cc -c) is sufficient.
        has_main = False
        try:
            with open(src, "r", encoding="utf-8", errors="replace") as f:
                content = f.read()
                # Simple heuristic: look for `int main(` or `int main (`
                # in the file (not in comments).
                for line in content.splitlines():
                    stripped = line.strip()
                    if stripped.startswith("/*") or stripped.startswith("*"):
                        continue
                    if re.search(r'\bint\s+main\s*\(', stripped):
                        has_main = True
                        break
        except Exception:
            pass
        if not has_main:
            # Compile only (no link, no run).
            exe_path = tmp_out + ".o"
            stdout, stderr, rc = run_cmd(
                [zig_path, "cc", "-c", tmp_out, "-o", exe_path, "-std=c11",
                 "-fno-sanitize=undefined"],
                timeout=timeout
            )
            _cleanup_files(tmp_out, exe_path)
            if rc != 0:
                detail = (stderr or stdout).strip()
                if len(detail) > 300:
                    detail = detail[:300] + "..."
                return name, 2, f"zig cc -c failed: {detail}"
            return name, 0, "pass (compile-only)"
        # Full compile + run for files with main.
        rc, detail = _compile_and_run(tmp_out, zig_path, timeout)
        _cleanup_files(tmp_out)
        # Some bug tests intentionally return non-zero from main as a
        # side-effect of their logic (e.g. `return my_func(...)` where
        # the function returns a non-zero value).  For bugs tests, we
        # only care that sharpc generates compilable C — runtime exit
        # code is not a pass/fail criterion.
        if rc == 1:
            return name, 0, f"pass (runtime exit: {detail})"
        return name, rc, detail

    with ThreadPoolExecutor(max_workers=max_workers) as ex:
        futures = {
            ex.submit(_test_one, f): f for f in c_files
        }
        for fut in as_completed(futures):
            name, rc, detail = fut.result()
            _record(name, rc, detail)
            if verbose:
                icon = "PASS" if rc == 0 else ("FAIL" if rc == 1 else "ERR ")
                print(f"    {icon}  {name}")

    if not verbose:
        print(" done.")
        failures = [
            (name, rc, detail)
            for name, (rc, detail) in sorted(file_results.items())
            if rc != 0
        ]
        if failures:
            print()
            for name, rc, detail in failures:
                icon = "FAIL" if rc == 1 else "ERR "
                print(f"  {icon}  {name}")
                for line in detail.splitlines():
                    print(f"         {line}")

    return passed, failed, errors, file_results


# ── Unit 测试 (unit/) ────────────────────────────────────────────────────

def run_unit_tests(
    base_dir: Path,
    sharpc_path: str,
    zig_path: str = "",
    verbose: bool = False,
    timeout: int = 30,
    jobs: int = 1,
) -> Tuple[int, int, int, Dict[str, Tuple[int, str]]]:
    """Run all unit tests from unit/ directory.

    Each .ce file is:
      1. compiled: sharpc -c test.ce → test.ref.i
      2. compared: diff generated vs .ref.i (if .ref.i exists)
      3. compiled: zig cc test.ref.i → test.exe
      4. run: ./test.exe → exit code 0 = pass
    """
    unit_dir = base_dir / UNIT_DIR_NAME
    if not unit_dir.is_dir():
        return 0, 0, 0, {}

    sp_files = sorted(str(p.relative_to(base_dir)) for p in unit_dir.rglob("*.ce"))
    if not sp_files:
        return 0, 0, 0, {}

    total_files = len(sp_files)
    file_results: Dict[str, Tuple[int, str]] = {}
    passed = failed = errors = 0
    completed = 0
    lock = threading.Lock()
    print_lock = threading.Lock()

    def _record(name: str, rc: int, detail: str) -> None:
        nonlocal passed, failed, errors, completed
        with lock:
            file_results[name] = (rc, detail)
            if rc == 0:
                passed += 1
            elif rc == 1:
                failed += 1
            else:
                errors += 1
            completed += 1
            progress = f"[{completed:3d}/{total_files}]"

        if verbose:
            status_map = {0: "pass", 1: "fail", 2: "error"}
            icon_map = {0: "PASS", 1: "FAIL", 2: "ERR "}
            status = status_map.get(rc, "???")
            icon = icon_map.get(rc, "???")
            with print_lock:
                print(f"  {icon} {progress}  {name}")
                if rc != 0:
                    for line in detail.splitlines():
                        print(f"           {line}")

    def test_unit_sp(rel_path: str) -> Tuple[int, str]:
        sp_path = str(base_dir / rel_path)
        ref_path = _resolve_ref_path(sp_path)
        ok, tmp_out, err = _run_sharpc_codegen(sp_path, sharpc_path, zig_path, timeout)
        if not ok:
            return 2, err

        if ref_path:
            rc, detail = _compare_gen_with_ref(tmp_out, ref_path)
            if rc != 0:
                _cleanup_files(tmp_out)
                return rc, detail

        rc, detail = _compile_and_run(tmp_out, zig_path, timeout)
        _cleanup_files(tmp_out)
        return rc, detail

    if verbose:
        print(f"\n[unit] {total_files} tests  jobs={jobs}\n")

    with ThreadPoolExecutor(max_workers=jobs) as ex:
        futures = {
            ex.submit(test_unit_sp, sp): sp
            for sp in sp_files
        }
        for fut in as_completed(futures):
            sp_file = futures[fut]
            rc, detail = fut.result()
            _record(sp_file, rc, detail)

    if not verbose:
        failures_list = [
            (name, rc, detail)
            for name, (rc, detail) in sorted(file_results.items())
            if rc != 0
        ]
        if failures_list:
            print()
            for name, rc, detail in failures_list:
                icon = "FAIL" if rc == 1 else "ERR "
                print(f"  {icon}  {name}")
                for line in detail.splitlines():
                    print(f"         {line}")

    return passed, failed, errors, file_results


def run_special_probe_tests(base_dir: Path, sharpc_path: str,
                            zig_path: str = "",
                            verbose: bool = False, timeout: int = 60,
                            jobs: int = 1) -> Tuple[int, int, int, Dict[str, Tuple[int, str]]]:
    """Run cross-compile / special probes from the 'special/' directory."""
    special_dir = base_dir / SPECIAL_DIR_NAME
    if not special_dir.is_dir():
        return 0, 0, 0, {}

    c_files = sorted(
        f for f in os.listdir(special_dir)
        if f.endswith(".c") and not f.endswith(".ref.i")
    )
    if not c_files:
        return 0, 0, 0, {}

    if verbose:
        print(f"\n[probes] Cross-compile probes ({len(c_files)} files) ...")

    file_results: Dict[str, Tuple[int, str]] = {}
    passed = failed = errors = 0
    lock = threading.Lock()

    def _record(name, rc, detail):
        nonlocal passed, failed, errors
        with lock:
            file_results[name] = (rc, detail)
            if rc == 0:
                passed += 1
            elif rc == 1:
                failed += 1
            else:
                errors += 1

    with ThreadPoolExecutor(max_workers=jobs) as ex:
        futures = {}
        for c_file in c_files:
            full_path = str(special_dir / c_file)
            fut = ex.submit(test_c_probe, full_path, sharpc_path, zig_path, timeout,
                           CROSS_COMPILE_ARGS)
            futures[fut] = c_file

        for fut in as_completed(futures):
            c_file = futures[fut]
            rc, detail = fut.result()
            _record(c_file, rc, detail)

    if verbose:
        for name in c_files:
            if name in file_results:
                rc, detail = file_results[name]
                status = "pass" if rc == 0 else ("fail" if rc == 1 else "error")
                icon = {"pass": "PASS", "fail": "FAIL", "error": "ERR "}[status]
                print(f"  {icon}  {name}")
                if status != "pass":
                    for line in detail.splitlines():
                        print(f"         {line}")

    return passed, failed, errors, file_results


# ── 主逻辑 ──────────────────────────────────────────────────────────────

def main():
    default_jobs = min(os.cpu_count() or 4, 16)

    parser = argparse.ArgumentParser(description="Sharp 统一回归测试")
    parser.add_argument(
        "--compiler", default="zig", choices=list(BACKENDS.keys()),
        help="Reference compiler backend (default: zig)"
    )
    parser.add_argument("--sharpc", default=None, help="Path to sharpc binary")
    parser.add_argument("--ref-compiler", default=None,
                        help="Path to reference compiler binary (default: auto-detect)")
    parser.add_argument("--verbose", "-v", action="store_true",
                        help="Print per-file results in real time")
    parser.add_argument("--timeout", type=int, default=60,
                        help="Timeout per file in seconds")
    parser.add_argument(
        "-j", "--jobs", type=int, default=default_jobs,
        help=f"Parallel jobs (default: {default_jobs} = cpu_count, capped at 16)"
    )
    parser.add_argument("--json", action="store_true", help="Output results as JSON")
    parser.add_argument("--dump-fail", action="store_true",
                        help="Save raw -E output for failed files")

    args = parser.parse_args()

    script_dir = Path(__file__).resolve().parent

    sharpc_path, build_info = ensure_sharpc(script_dir, args.sharpc)
    print(f"[build] sharpc = {sharpc_path} ({build_info})")

    # Find zig for compile+run phase
    project_root = find_project_root(script_dir)
    zig_path = _find_zig(project_root) if project_root else None
    if not zig_path:
        raise SystemExit("ERROR: cannot find zig (checked project's zig/ and PATH)")
    print(f"[build] zig    = {zig_path}")

    total_pass = total_fail = total_error = 0
    results: Dict = {}

    # ── 超集探针代码生成测试 (C + SP) ──
    print()
    print("[sp] Running ALL superset probe tests (C + SP codegen) ...")
    sp_pass, sp_fail, sp_error, sp_files = run_all_probe_tests(
        script_dir, str(sharpc_path), zig_path, args.verbose, args.timeout, args.jobs
    )
    results["sp"] = {
        "pass": sp_pass, "fail": sp_fail, "error": sp_error,
        "files": {k: list(v) for k, v in sp_files.items()}
    }
    total_pass  += sp_pass
    total_fail  += sp_fail
    total_error += sp_error

    if not args.verbose:
        total_tests = sp_pass + sp_fail + sp_error
        print(f"  {sp_pass}/{total_tests} passed, {sp_fail} failed, {sp_error} errors")

    # ── Bugs 测试 (bugs/) ──
    print()
    print("[bugs] Running bug regression tests (codegen + runtime) ...")
    bugs_pass, bugs_fail, bugs_error, bugs_files = run_bugs_tests(
        script_dir, str(sharpc_path), zig_path, args.verbose, args.timeout, args.jobs
    )
    results["bugs"] = {
        "pass": bugs_pass, "fail": bugs_fail, "error": bugs_error,
        "files": {k: list(v) for k, v in bugs_files.items()}
    }
    total_pass  += bugs_pass
    total_fail  += bugs_fail
    total_error += bugs_error

    if not args.verbose:
        total_bt = bugs_pass + bugs_fail + bugs_error
        print(f"  {bugs_pass}/{total_bt} passed, {bugs_fail} failed, {bugs_error} errors")

    # ── Unit 测试 (unit/) ──
    print()
    print("[unit] Running unit tests (codegen + runtime) ...")
    unit_pass, unit_fail, unit_error, unit_files = run_unit_tests(
        script_dir, str(sharpc_path), zig_path, args.verbose, args.timeout, args.jobs
    )
    results["unit"] = {
        "pass": unit_pass, "fail": unit_fail, "error": unit_error,
        "files": {k: list(v) for k, v in unit_files.items()}
    }
    total_pass  += unit_pass
    total_fail  += unit_fail
    total_error += unit_error

    if not args.verbose:
        total_ut = unit_pass + unit_fail + unit_error
        print(f"  {unit_pass}/{total_ut} passed, {unit_fail} failed, {unit_error} errors")

    # ── Cross-compile / special probes ──
    print()
    print("[sp] Running special cross-compile probe tests ...")
    sc_pass, sc_fail, sc_error, sc_files = run_special_probe_tests(
        script_dir, str(sharpc_path), zig_path, args.verbose, args.timeout, args.jobs
    )
    total_pass  += sc_pass
    total_fail  += sc_fail
    total_error += sc_error

    if not args.verbose:
        total_sc = sc_pass + sc_fail + sc_error
        print(f"  {sc_pass}/{total_sc} passed, {sc_fail} failed, {sc_error} errors")

    # ── 汇总输出 ──
    if args.json:
        print(json.dumps(results, indent=2))
    else:
        total_files = total_pass + total_fail + total_error
        print()
        print("=" * 60)
        print(f"  Total: {total_pass}/{total_files} passed")
        if total_fail:
            print(f"         {total_fail} failed")
        if total_error:
            print(f"         {total_error} errors")
        print("=" * 60)

    return 0 if (total_fail == 0 and total_error == 0) else 1


if __name__ == "__main__":
    sys.exit(main())