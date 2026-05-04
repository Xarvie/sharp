# Sharp/C Preprocessor — build, test, and benchmark
#
# Targets:
#   all              — build all test binaries + bench
#   test             — build and run all nine test suites
#   asan             — build with AddressSanitizer + UBSan and run all tests
#   strict           — build with maximum warnings and -Werror
#   bench            — build and run the standalone performance benchmark
#   profile          — perf record + perf report (top functions)        [Phase 8]
#   flamegraph       — render interactive flamegraph.svg via FlameGraph [Phase 8]
#   massif           — valgrind massif heap-peak report                 [Phase 8]
#   lua_smoke        — Lua 5.4 integration smoke test                   [Phase R1]
#   sqlite_smoke     — SQLite 3.45 integration smoke test               [Phase R2]
#   zlib_smoke       — zlib 1.3.1 integration smoke test                [Phase R3]
#   mcpp_smoke       — mcpp Validation Suite conformance test           [Phase R4]
#   real_world_tests — run all four R-series smoke tests in sequence
#   clean            — remove build artifacts

CC      ?= gcc
CSTD    ?= -std=c11
CFLAGS  ?= -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-trigraphs -O2 -g
LDLIBS  ?=



LIB_SRCS = cpp.c pptok.c macro.c expr.c directive.c cpp_targets.c
LIB_HDRS = cpp.h cpp_internal.h pptok.h macro.h expr.h directive.h

# test_phase6 and bench need POSIX clock_gettime
PHASE6_FLAGS = -D_POSIX_C_SOURCE=200809L

.PHONY: all test asan strict bench profile flamegraph massif lua_smoke sqlite_smoke zlib_smoke mcpp_smoke redis_smoke self_smoke mcpp_self_smoke mcpp_native_smoke real_world_tests clean

all: test_phase1 test_phase2 test_phase3 test_phase4 test_phase5 test_phase6 test_phase7 test_phase8 test_lua_includes test_mcpp_includes test_sharp_cpp sharp_cli bench_bin

test_phase1: test_phase1.c $(LIB_SRCS) $(LIB_HDRS)
	$(CC) $(CSTD) $(CFLAGS) -o $@ test_phase1.c $(LIB_SRCS) $(LDLIBS)

test_phase2: test_phase2.c $(LIB_SRCS) $(LIB_HDRS)
	$(CC) $(CSTD) $(CFLAGS) -o $@ test_phase2.c $(LIB_SRCS) $(LDLIBS)

test_phase3: test_phase3.c $(LIB_SRCS) $(LIB_HDRS)
	$(CC) $(CSTD) $(CFLAGS) -o $@ test_phase3.c $(LIB_SRCS) $(LDLIBS)

test_phase4: test_phase4.c $(LIB_SRCS) $(LIB_HDRS)
	$(CC) $(CSTD) $(CFLAGS) -o $@ test_phase4.c $(LIB_SRCS) $(LDLIBS)

test_phase5: test_phase5.c $(LIB_SRCS) $(LIB_HDRS)
	$(CC) $(CSTD) $(CFLAGS) -o $@ test_phase5.c $(LIB_SRCS) $(LDLIBS)

test_phase6: test_phase6.c $(LIB_SRCS) $(LIB_HDRS)
	$(CC) $(CSTD) $(PHASE6_FLAGS) $(CFLAGS) -o $@ test_phase6.c $(LIB_SRCS) $(LDLIBS)

test_phase7: test_phase7.c $(LIB_SRCS) $(LIB_HDRS)
	$(CC) $(CSTD) $(CFLAGS) -o $@ test_phase7.c $(LIB_SRCS) $(LDLIBS)

test_phase8: test_phase8.c $(LIB_SRCS) $(LIB_HDRS)
	$(CC) $(CSTD) $(CFLAGS) -o $@ test_phase8.c $(LIB_SRCS) $(LDLIBS)

# Phase R1: real-world integration regression — frozen against the
# Lua 5.4 source tree's exposure of a system-include path-search bug.
test_lua_includes: test_lua_includes.c $(LIB_SRCS) $(LIB_HDRS)
	$(CC) $(CSTD) $(CFLAGS) -o $@ test_lua_includes.c $(LIB_SRCS) $(LDLIBS)

# Phase R4: regression tests for the four PP-spec bugs found and fixed
# during the mcpp Validation Suite integration phase, plus the seven
# strictness checks added.  Each section reproduces the minimal
# trigger from a specific mcpp test (n_5, n_13_5, n_13_7, n_32, n_nularg,
# e_4_3, e_14_2, e_12_8, e_24_6, e_23_3, e_31).
test_mcpp_includes: test_mcpp_includes.c $(LIB_SRCS) $(LIB_HDRS)
	$(CC) $(CSTD) $(CFLAGS) -o $@ test_mcpp_includes.c $(LIB_SRCS) $(LDLIBS)



# Phase R1: standalone CLI driver used by lua_smoke.sh.  Not a public
# tool — production users should call cpp_run_buf() via the C API.
sharp_cli: sharp_cli.c $(LIB_SRCS) $(LIB_HDRS)
	$(CC) $(CSTD) $(CFLAGS) -o $@ sharp_cli.c $(LIB_SRCS) $(LDLIBS)

test_sharp_cpp: test_sharp_cpp.c $(LIB_SRCS) $(LIB_HDRS)
	$(CC) $(CSTD) $(CFLAGS) -o $@ test_sharp_cpp.c $(LIB_SRCS) $(LDLIBS)

probe: probe.c $(LIB_SRCS) $(LIB_HDRS)
	$(CC) $(CSTD) $(CFLAGS) -o $@ probe.c $(LIB_SRCS) $(LDLIBS)

bench_bin: bench.c $(LIB_SRCS) $(LIB_HDRS)
	$(CC) $(CSTD) $(PHASE6_FLAGS) $(CFLAGS) -o $@ bench.c $(LIB_SRCS) $(LDLIBS)

test: test_phase1 test_phase2 test_phase3 test_phase4 test_phase5 test_phase6 test_phase7 test_phase8 test_lua_includes test_mcpp_includes test_sharp_cpp
	@echo "===== Phase 1 tests ====="
	./test_phase1
	@echo
	@echo "===== Phase 2 tests ====="
	./test_phase2
	@echo
	@echo "===== Phase 3 tests ====="
	./test_phase3
	@echo
	@echo "===== Phase 4 tests ====="
	./test_phase4
	@echo
	@echo "===== Phase 5 tests ====="
	./test_phase5
	@echo
	@echo "===== Phase 6 tests ====="
	./test_phase6
	@echo
	@echo "===== Phase 7 tests ====="
	./test_phase7
	@echo
	@echo "===== Phase 8 tests ====="
	./test_phase8
	@echo
	@echo "===== Phase R1 tests (Lua-discovered regressions) ====="
	./test_lua_includes
	@echo
	@echo "===== Phase R4 tests (mcpp-discovered regressions) ====="
	./test_mcpp_includes
	@echo
	@echo "===== Existing Sharp-extension tests ====="
	./test_sharp_cpp
	@echo
	@echo

asan: CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer -O0
asan: clean test

strict: CFLAGS += -Wpedantic -Wshadow -Wstrict-prototypes -Werror -Wno-error=trigraphs
strict: clean test

bench: bench_bin
	./bench_bin 30

# Phase 8: developer-experience targets for performance investigation.
#
# `make profile` records a perf trace of `./bench_bin 200` and prints a
# top-functions report.  Requires linux-perf.
#
# `make flamegraph` does the same and renders an interactive SVG via
# Brendan Gregg's FlameGraph scripts.  Set FLAMEGRAPH_DIR to point at a
# checkout of https://github.com/brendangregg/FlameGraph (default:
# /opt/FlameGraph).
#
# `make massif` records peak heap usage with valgrind --tool=massif and
# prints a textual summary via ms_print.  Requires valgrind.
#
# All three targets force a fresh build with frame pointers preserved
# (-fno-omit-frame-pointer) so unwound stacks are accurate.  See
# PROFILING.md for the full workflow including CPU pinning and
# environment caveats (turbo boost, ASLR, etc.).

PROFILE_FLAGS  = -O2 -g -fno-omit-frame-pointer
PROFILE_ITERS ?= 200
FLAMEGRAPH_DIR ?= /opt/FlameGraph

profile_bin: bench.c $(LIB_SRCS) $(LIB_HDRS)
	$(CC) $(CSTD) $(PHASE6_FLAGS) $(PROFILE_FLAGS) -o $@ bench.c $(LIB_SRCS) $(LDLIBS)

profile: profile_bin
	@command -v perf >/dev/null 2>&1 || \
		{ echo "ERROR: 'perf' not found.  See PROFILING.md for setup." >&2; exit 1; }
	@echo "Recording perf trace ($(PROFILE_ITERS) iters)..."
	perf record -F 999 --call-graph dwarf -o perf.data ./profile_bin $(PROFILE_ITERS)
	@echo
	@echo "===== Top 20 functions by self-time ====="
	perf report -i perf.data --stdio --no-children -n | head -40

flamegraph: profile_bin
	@command -v perf >/dev/null 2>&1 || \
		{ echo "ERROR: 'perf' not found.  See PROFILING.md for setup." >&2; exit 1; }
	@test -x "$(FLAMEGRAPH_DIR)/stackcollapse-perf.pl" || \
		{ echo "ERROR: $(FLAMEGRAPH_DIR)/stackcollapse-perf.pl not found." >&2; \
		  echo "       Set FLAMEGRAPH_DIR=... or git clone Brendan Gregg's FlameGraph repo." >&2; \
		  exit 1; }
	perf record -F 999 --call-graph dwarf -o perf.data ./profile_bin $(PROFILE_ITERS)
	perf script -i perf.data | $(FLAMEGRAPH_DIR)/stackcollapse-perf.pl > perf.folded
	$(FLAMEGRAPH_DIR)/flamegraph.pl perf.folded > flamegraph.svg
	@echo "Wrote flamegraph.svg ($(shell wc -c < flamegraph.svg 2>/dev/null) bytes)"

massif: profile_bin
	@command -v valgrind >/dev/null 2>&1 || \
		{ echo "ERROR: 'valgrind' not found.  See PROFILING.md for setup." >&2; exit 1; }
	valgrind --tool=massif --massif-out-file=massif.out ./profile_bin 5
	@command -v ms_print >/dev/null 2>&1 && ms_print massif.out | head -60 || \
		echo "(install ms_print or read massif.out manually)"

# Phase R1: real-world integration smoke test against Lua 5.4 source.
#
# Usage:
#   make lua_smoke                 # auto-clones Lua to /tmp/sharp_cpp_lua/
#   make lua_smoke LUA_SRC=/path   # uses an existing checkout
#
# The script lua_smoke.sh runs sharp_cli on every .c file in the Lua
# source tree, compares output sizes against `gcc -E`, and reports the
# pass/fail summary.  Network access is required for the auto-clone path.
LUA_SRC      ?= /tmp/sharp_cpp_lua
LUA_VERSION  ?= v5.4.7

$(LUA_SRC)/lua.h:
	@if [ ! -d "$(LUA_SRC)" ]; then \
		echo "Cloning Lua $(LUA_VERSION) into $(LUA_SRC)..."; \
		git clone --depth 1 --branch $(LUA_VERSION) https://github.com/lua/lua.git $(LUA_SRC) >/dev/null 2>&1 || \
			{ echo "ERROR: git clone failed (network access required)" >&2; exit 1; }; \
	fi

lua_smoke: sharp_cli $(LUA_SRC)/lua.h
	@./lua_smoke.sh $(LUA_SRC)

# Phase R2: real-world integration smoke test against SQLite 3.45 source.
#
# Usage:
#   make sqlite_smoke                 # auto-clones SQLite to /tmp/sharp_cpp_sqlite/
#                                     # and builds amalgamation (requires tcl)
#   make sqlite_smoke SQLITE_SRC=...  # uses an existing checkout that has
#                                     # already had `make sqlite3.c` run
#
# The script sqlite_smoke.sh runs sharp_cli on every src/*.c plus the
# sqlite3.c amalgamation, comparing output sizes against `gcc -E`.
# Network access + tclsh required for the auto-clone path.
SQLITE_SRC      ?= /tmp/sharp_cpp_sqlite
SQLITE_VERSION  ?= version-3.45.0

$(SQLITE_SRC)/sqlite3.c:
	@if [ ! -d "$(SQLITE_SRC)" ]; then \
		echo "Cloning SQLite $(SQLITE_VERSION) into $(SQLITE_SRC)..."; \
		git clone --depth 1 --branch $(SQLITE_VERSION) https://github.com/sqlite/sqlite.git $(SQLITE_SRC) >/dev/null 2>&1 || \
			{ echo "ERROR: git clone failed (network access required)" >&2; exit 1; }; \
	fi
	@command -v tclsh >/dev/null 2>&1 || \
		{ echo "ERROR: tclsh required to build SQLite amalgamation" >&2; \
		  echo "       Install with: apt-get install tcl-dev tcl" >&2; exit 1; }
	@echo "Building SQLite amalgamation (this takes ~30s)..."
	@cd $(SQLITE_SRC) && ./configure --enable-amalgamation >/dev/null 2>&1 && \
		make sqlite3.c >/dev/null 2>&1 || \
		{ echo "ERROR: SQLite amalgamation build failed" >&2; exit 1; }

sqlite_smoke: sharp_cli $(SQLITE_SRC)/sqlite3.c
	@./sqlite_smoke.sh $(SQLITE_SRC)

# Phase R3: real-world integration smoke test against zlib 1.3.1.
#
# Usage:
#   make zlib_smoke                # auto-clones zlib to /tmp/sharp_cpp_zlib/
#   make zlib_smoke ZLIB_SRC=...   # uses an existing checkout
#
# Smaller and simpler than Lua/SQLite — exercises K&R-era macros and
# tight performance-oriented header structure.  Network access required.
ZLIB_SRC      ?= /tmp/sharp_cpp_zlib
ZLIB_VERSION  ?= v1.3.1

$(ZLIB_SRC)/zlib.h:
	@if [ ! -d "$(ZLIB_SRC)" ]; then \
		echo "Cloning zlib $(ZLIB_VERSION) into $(ZLIB_SRC)..."; \
		git clone --depth 1 --branch $(ZLIB_VERSION) https://github.com/madler/zlib.git $(ZLIB_SRC) >/dev/null 2>&1 || \
			{ echo "ERROR: git clone failed (network access required)" >&2; exit 1; }; \
	fi

zlib_smoke: sharp_cli $(ZLIB_SRC)/zlib.h
	@./zlib_smoke.sh $(ZLIB_SRC)

# Phase R4: real-world conformance test against mcpp's Validation Suite —
# the gold-standard PP standard-compliance test set.
#
# Usage:
#   make mcpp_smoke               # auto-clones mcpp to /tmp/sharp_cpp_mcpp/
#   make mcpp_smoke MCPP_SRC=...  # uses an existing checkout
#
# mcpp's test-t/ contains 100+ files exercising specific C99 §6.10
# subsections (n_NN.c = normal-case, e_NN.c = error-case).  The runner
# mcpp_smoke.sh classifies each file by its dg-error annotation rather
# than the n_/e_ prefix and reports per-category pass/fail.
MCPP_SRC      ?= /tmp/sharp_cpp_mcpp

$(MCPP_SRC)/cpp-test/test-t/cpp-test.exp:
	@if [ ! -d "$(MCPP_SRC)" ]; then \
		echo "Cloning mcpp into $(MCPP_SRC)..."; \
		git clone --depth 1 https://github.com/museoa/mcpp.git $(MCPP_SRC) >/dev/null 2>&1 || \
			{ echo "ERROR: git clone failed (network access required)" >&2; exit 1; }; \
	fi

mcpp_smoke: sharp_cli $(MCPP_SRC)/cpp-test/test-t/cpp-test.exp
	@./mcpp_smoke.sh $(MCPP_SRC)

# Convenience: run all R-series integration smoke tests in sequence
real_world_tests: lua_smoke sqlite_smoke zlib_smoke mcpp_smoke redis_smoke self_smoke mcpp_self_smoke mcpp_native_smoke

# Phase R11: mcpp's native .t testsuite (153 files).  Smoke test only.
mcpp_native_smoke: sharp_cli $(MCPP_SRC)/test-t/LICENSE
	@./mcpp_native_smoke.sh $(MCPP_SRC)

# Phase R8: cross-PP test — sharp-cpp processes mcpp's own source.
# Uses the same MCPP_SRC as mcpp_smoke; auto-cloned by that target.
mcpp_self_smoke: sharp_cli $(MCPP_SRC)/cpp-test/test-t/cpp-test.exp
	@./mcpp_self_smoke.sh $(MCPP_SRC)

# Phase R5: Redis is a 175 KLoC C codebase with 8 vendored deps and
# heavy use of variadic macros, atomic builtins, and feature-detection
# trampolines.  Auto-clones to /home/claude/redis-src if not present.
REDIS_SRC ?= /home/claude/redis-src
$(REDIS_SRC)/src/server.c:
	@mkdir -p "$(dir $(REDIS_SRC))"
	@cd "$(dir $(REDIS_SRC))" && git clone --depth 1 https://github.com/redis/redis.git "$(notdir $(REDIS_SRC))"

redis_smoke: sharp_cli $(REDIS_SRC)/src/server.c
	@./redis_smoke.sh $(REDIS_SRC)

# Phase R6: dogfooding — sharp-cpp processes its own sources.
# No external dependency; just runs the runner.
self_smoke: sharp_cli
	@./self_smoke.sh

clean:
	rm -f test_phase1 test_phase2 test_phase3 test_phase4 test_phase5 test_phase6 test_phase7 test_phase8 test_lua_includes test_sharp_cpp probe bench_bin profile_bin sharp_cli perf.data perf.folded flamegraph.svg massif.out *.o
