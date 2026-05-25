#!/usr/bin/env python3
"""
compare_tokens.py — 比较 gcc -E 与 sharpc -E 的预处理输出是否在 token 级别一致。

用法:
    # 单个文件
    python3 compare_tokens.py <source.c> -I/path1 -I/path2

    # 多个文件（共享 -I 参数）
    python3 compare_tokens.py a.c b.c c.c -I/path1 -I/path2

    # 指定 sharpc 路径
    python3 compare_tokens.py a.c -I/path1 --sharpc /path/to/sharpc

    # 静默模式（只打印汇总）
    python3 compare_tokens.py a.c b.c -I/path1 --summary

输出:
    exit 0: 全部 token 匹配
    exit 1: 存在不匹配或错误
"""

import subprocess
import sys
import os
import re
import argparse


def run_cmd(cmd, timeout=60):
    """运行命令，返回 (stdout, stderr, returncode)。"""
    try:
        result = subprocess.run(
            cmd, capture_output=True, text=True, timeout=timeout
        )
        return result.stdout, result.stderr, result.returncode
    except subprocess.TimeoutExpired:
        return "", "timeout", 124
    except Exception as e:
        return "", str(e), 1


def tokenize(text):
    """
    将预处理输出分割为 C token 列表。
    - 移除 #line / # 行标记指令
    - 用 C 词法规则分割 token（保留 `[` `]` `(` `)` 等标点附着）
    - 忽略纯空白差异
    """
    # 移除 #line 指令（gcc linemarkers）
    text = re.sub(r'^\s*#\s*\d.*$', '', text, flags=re.MULTILINE)

    # C token 正则：字符串、字符、标识符/数字、标点、其他
    token_re = re.compile(
        r'"(?:[^"\\]|\\.)*"'      # 字符串字面量
        r"|'(?:[^'\\]|\\.)*'"    # 字符字面量
        r"|L\"(?:[^\"\\]|\\.)*\""  # 宽字符串
        r"|L'(?:[^'\\]|\\.)*'"   # 宽字符
        r"|[A-Za-z_]\w*"         # 标识符
        r"|[0-9][0-9xXaAbB.]*[fFlLuU]*"  # 数字（含 hex、科学计数法）
        r"|->|::|\.\.\.|<<=|>>="  # 三字符操作符
        r"|[+\-*/%&|^~<>=!?:;.(){}[\],#]?"  # 单/双字符标点
        r"|\S"                    # 其他非空白字符
    )

    tokens = token_re.findall(text)
    # 过滤空字符串
    return [t for t in tokens if t]


def compare_tokens(source, includes, sharpc_path=None):
    """
    比较 gcc -E 和 sharpc -E 的输出 token。
    返回: 0=匹配, 1=不匹配, 2=编译失败
    """
    if sharpc_path is None:
        sharpc_path = os.path.join(
            os.path.dirname(os.path.abspath(__file__)),
            "..", "sharp-fe", "sharpc"
        )
        if not os.path.isfile(sharpc_path):
            sharpc_path = "sharpc"

    gcc_cmd = ["gcc", "-E", source] + includes
    sharp_cmd = [sharpc_path, source, "-E"] + includes

    gcc_out, gcc_err, gcc_rc = run_cmd(gcc_cmd)
    if gcc_rc != 0:
        print(f"ERROR: gcc failed for {os.path.basename(source)}", file=sys.stderr)
        return 2

    sharp_out, sharp_err, sharp_rc = run_cmd(sharp_cmd)
    if sharp_rc != 0:
        print(f"ERROR: sharpc failed for {os.path.basename(source)}", file=sys.stderr)
        return 2

    gcc_tokens = tokenize(gcc_out)
    sharp_tokens = tokenize(sharp_out)

    if gcc_tokens == sharp_tokens:
        return 0

    # 不匹配，打印差异
    bname = os.path.basename(source)
    print(f"FAIL: {bname}")
    print(f"  gcc tokens:    {len(gcc_tokens)}")
    print(f"  sharpc tokens: {len(sharp_tokens)}")

    # 找第一个不同的 token 位置
    min_len = min(len(gcc_tokens), len(sharp_tokens))
    for i in range(min_len):
        if gcc_tokens[i] != sharp_tokens[i]:
            ctx = 3
            start = max(0, i - ctx)
            end = min(min_len, i + ctx + 1)
            g_ctx = ' '.join(gcc_tokens[start:end])
            s_ctx = ' '.join(sharp_tokens[start:end])
            print(f"  First diff at token [{i}]:")
            print(f"    gcc:    ... {g_ctx}")
            print(f"    sharpc: ... {s_ctx}")
            break
    else:
        if len(gcc_tokens) != len(sharp_tokens):
            longer = "gcc" if len(gcc_tokens) > len(sharp_tokens) else "sharpc"
            diff = abs(len(gcc_tokens) - len(sharp_tokens))
            print(f"  All {min_len} tokens match, but {longer} has {diff} extra at end")

    return 1


def main():
    # 使用 parse_known_args 让 -I 可以出现在任意位置
    parser = argparse.ArgumentParser(
        description="Compare gcc -E and sharpc -E output at token level"
    )
    parser.add_argument(
        "sources", nargs="+",
        help="Source .c files to compare"
    )
    parser.add_argument(
        "-I", dest="includes", action="append", default=[],
        help="Include directories (can be repeated)"
    )
    parser.add_argument(
        "--sharpc", default=None,
        help="Path to sharpc binary (default: auto-detect)"
    )
    parser.add_argument(
        "--summary", action="store_true",
        help="Only print pass/fail summary"
    )
    parser.add_argument(
        "--timeout", type=int, default=60,
        help="Timeout per file in seconds (default: 60)"
    )

    args, _ = parser.parse_known_args()

    # 将 -I 参数转为 gcc/sharpc 能识别的格式
    includes = [f"-I{d}" for d in args.includes]

    pass_count = 0
    fail_count = 0
    error_count = 0

    for source in args.sources:
        if not os.path.isfile(source):
            print(f"ERROR: {source}: file not found", file=sys.stderr)
            error_count += 1
            continue

        rc = compare_tokens(source, includes, args.sharpc)
        if rc == 0:
            if not args.summary:
                print(f"PASS: {os.path.basename(source)}")
            pass_count += 1
        elif rc == 1:
            fail_count += 1
        else:
            error_count += 1

    total = pass_count + fail_count + error_count
    if total > 0 or args.summary:
        print()
        print("=" * 50)
        label = f"{pass_count} passed"
        if fail_count:
            label += f", {fail_count} failed"
        if error_count:
            label += f", {error_count} errors"
        label += f" out of {total}"
        print(f"  {label}")
        print("=" * 50)

    return 0 if (fail_count == 0 and error_count == 0) else 1


if __name__ == "__main__":
    sys.exit(main())
