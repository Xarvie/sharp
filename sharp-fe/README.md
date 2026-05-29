# Sharp Frontend

C11 superset language frontend — parses `.ce` (Sharp) and `.c` (C11) files,
produces C11 output for downstream compilation.

## Repository layout

```
sharp-fe/          — compiler source (this directory)
sharp-cpp/         — C preprocessor module (sibling, required)
sharp-test/        — test suite (sibling)
  ├── tokcmp.c            token-comparison tool source
  ├── probe.c             phase-0 unit test driver
  ├── c_superset_probe.sh end-to-end compile+run script
  ├── c_superset_probes/  99 .c probe files
  └── sqlite3.c/h         SQLite3 amalgamation (regression target)
```

## Build (run from sharp-fe/)

```bash
make sharpc        # compiler binary → ./sharpc
make tokcmp        # token diff tool → ./tokcmp  (source in ../sharp-test/)
make               # build everything
make test          # full test suite
make asan          # rebuild with AddressSanitizer + UBSan
make strict        # rebuild with -Wpedantic -Wshadow -Werror
```

## Validation — three gates

### Gate 1 — tokcmp batch (token identity)

```bash
./tokcmp --batch ../sharp-test/c_superset_probes
# Expected: identical=98  compiled=1  differ=0  errors=0  total=99
```

Each `.c` probe is preprocessed twice — once direct `gcc -E`, once through
`sharpc → gcc -E` — and the token streams are diffed.

### Gate 2 — SQLite3 (632 K tokens)

```bash
./tokcmp \
  --isystem=/usr/lib/gcc/x86_64-linux-gnu/13/include \
  --isystem=/usr/local/include \
  --isystem=/usr/include/x86_64-linux-gnu \
  --isystem=/usr/include \
  ../sharp-test/sqlite3.c
# Expected: ✅  sqlite3.c  632613 tokens
```

### Gate 3 — compile + run

```bash
make test
# Expected: C-superset probes: 99 / 99 pass
```

## Development workflow

### Debug a tokcmp diff

```bash
# See which tokens differ:
./tokcmp -v path/to/file.c

# With system includes:
ISYS="--isystem=/usr/lib/gcc/x86_64-linux-gnu/13/include \
      --isystem=/usr/local/include \
      --isystem=/usr/include/x86_64-linux-gnu \
      --isystem=/usr/include"
./tokcmp -v $ISYS path/to/file.c

# Inspect sharpc output:
./sharpc path/to/file.c $ISYS
```

The diff shows `- token` (gcc-E original) vs `+ token` (sharpc output).
Trace the first divergence in `parse.c` (AST) or `cg.c` (emission), fix, re-run.

### Add a probe

1. Create `../sharp-test/c_superset_probes/pNN_description.c` with `main()` returning 0 on success.
2. `./tokcmp ../sharp-test/c_superset_probes/pNN_description.c` → should show ✅.
3. `make test` → 99/99.

### Test a library

```bash
git clone --depth=1 https://github.com/libevent/libevent /tmp/libevent
cmake -S /tmp/libevent -B /tmp/libevent/build -DEVENT__DISABLE_OPENSSL=ON
ISYS="... --isystem=/tmp/libevent --isystem=/tmp/libevent/include --isystem=/tmp/libevent/build/include"
for f in /tmp/libevent/*.c; do ./tokcmp $ISYS "$f"; done
```

## Current real-world targets

| Library | Score | Notes |
|---|---|---|
| SQLite3 3.47.2 | ✅ 1/1 (632K tokens) | |
| curl 8.11.1 | ✅ 132/132 | |
| libevent 2.1.12 | ✅ 33/42 | 9 platform-structural† |
| mbedTLS 3.6.2 | ✅ 106/108 | 2 structural |
| zstd 1.5.6 | ✅ 39/39 | |
| brotli 1.1.0 | ✅ 32/32 | |
| libpng 1.6.44 | ✅ 17/18 | |
| zlib 1.3.1 | ✅ 15/15 | |
| Lua 5.4.7 | ✅ 34/34 | |
| oniguruma 6.9.9 | ✅ 53/53 | |
| jansson 2.14 | ✅ 12/12 | |
| lz4 1.9.3 | ✅ 4/4 | |

† libevent: Windows IOCP/winsock (4), SSL/mbedTLS (2), BSD sys/tree.h (1),
  SHA1 endianness (1), wepoll (1).
