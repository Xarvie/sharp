# Sharp Language Frontend — build, test, and probe
#
# Targets:
#   probe      — Phase 0: build and run all audit probes
#   test       — build and run all test suites (one binary added per phase)
#   asan       — build with AddressSanitizer + UBSan and run all tests
#   strict     — build with maximum warnings (-Werror) and run all tests
#   dist       — package to /mnt/user-data/outputs/sharp-fe-phase$(DIST_PHASE).tar.gz
#   clean      — remove build artifacts

CC      ?= gcc
CSTD    ?= -std=c11
CFLAGS  ?= -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -O2 -g

CPP_DIR  = ../sharp-cpp/sharp-cpp
CPP_SRCS = $(CPP_DIR)/cpp.c \
           $(CPP_DIR)/pptok.c \
           $(CPP_DIR)/macro.c \
           $(CPP_DIR)/expr.c \
           $(CPP_DIR)/directive.c \
           $(CPP_DIR)/cpp_targets.c

CPP_HDRS = $(CPP_DIR)/cpp.h \
           $(CPP_DIR)/cpp_internal.h \
           $(CPP_DIR)/pptok.h \
           $(CPP_DIR)/macro.h \
           $(CPP_DIR)/expr.h \
           $(CPP_DIR)/directive.h

CPP_INCS = -I$(CPP_DIR)

# Phase 1: lex module added to front-end library.
FE_SRCS  = lex.c ast.c parse.c scope.c type.c sema.c cg.c import.c
FE_HDRS  = lex.h ast.h parse.h scope.h type.h sema.h cg.h import.h sharp.h sharp_internal.h

DIST_PHASE ?= final

.PHONY: all probe test asan strict dist clean

# ── probe ──────────────────────────────────────────────────────────────────
probe: probe.c $(CPP_SRCS) $(CPP_HDRS)
	$(CC) $(CSTD) $(CFLAGS) $(CPP_INCS) -o $@ probe.c $(CPP_SRCS)

# ── test_phase1 ─────────────────────────────────────────────────────────────
test_phase1: test_phase1.c $(FE_SRCS) $(CPP_SRCS) $(FE_HDRS) $(CPP_HDRS)
	$(CC) $(CSTD) $(CFLAGS) $(CPP_INCS) -o $@ \
	    test_phase1.c $(FE_SRCS) $(CPP_SRCS) -lm

# ── test_phase3 ─────────────────────────────────────────────────────────────
test_phase3: test_phase3.c $(FE_SRCS) $(CPP_SRCS) $(FE_HDRS) $(CPP_HDRS)
	$(CC) $(CSTD) $(CFLAGS) $(CPP_INCS) -o $@ \
	    test_phase3.c $(FE_SRCS) $(CPP_SRCS)

# ── sharpc (compiler driver) ────────────────────────────────────────────────
sharpc: sharpc.c import.c $(FE_SRCS) $(CPP_SRCS) $(FE_HDRS) $(CPP_HDRS)
	$(CC) $(CSTD) $(CFLAGS) $(CPP_INCS) -o $@ \
	    sharpc.c $(FE_SRCS) $(CPP_SRCS)
	@echo "Sharp compiler driver built: ./sharpc"

# ── test_integration ────────────────────────────────────────────────────────
test_integration: test_integration.c $(FE_SRCS) $(CPP_SRCS) $(FE_HDRS) $(CPP_HDRS)
	$(CC) $(CSTD) $(CFLAGS) $(CPP_INCS) -o $@ \
	    test_integration.c $(FE_SRCS) $(CPP_SRCS)

# ── test_import ─────────────────────────────────────────────────────────────
test_import: test_import.c $(FE_SRCS) $(CPP_SRCS) $(FE_HDRS) $(CPP_HDRS)
	$(CC) $(CSTD) $(CFLAGS) $(CPP_INCS) -o $@ \
	    test_import.c $(FE_SRCS) $(CPP_SRCS)

# ── test_decls ──────────────────────────────────────────────────────────────
# Tests for multi-variable declarations (`int x = 3, y = 7;`) and C-style
# fixed-size arrays (`int arr[3];`, `int m[3][4];`).  Closes two long-
# standing parser/sema/cg gaps.
test_decls: test_decls.c $(FE_SRCS) $(CPP_SRCS) $(FE_HDRS) $(CPP_HDRS)
	$(CC) $(CSTD) $(CFLAGS) $(CPP_INCS) -o $@ \
	    test_decls.c $(FE_SRCS) $(CPP_SRCS)

# ── test_phase11 ────────────────────────────────────────────────────────────
test_phase11: test_phase11.c $(FE_SRCS) $(CPP_SRCS) $(FE_HDRS) $(CPP_HDRS)
	$(CC) $(CSTD) $(CFLAGS) $(CPP_INCS) -o $@ \
	    test_phase11.c $(FE_SRCS) $(CPP_SRCS)

# ── test_phase9 ─────────────────────────────────────────────────────────────
test_phase9: test_phase9.c $(FE_SRCS) $(CPP_SRCS) $(FE_HDRS) $(CPP_HDRS)
	$(CC) $(CSTD) $(CFLAGS) $(CPP_INCS) -o $@ \
	    test_phase9.c $(FE_SRCS) $(CPP_SRCS)

# ── test_phase10 ────────────────────────────────────────────────────────────
test_phase10: test_phase10.c $(FE_SRCS) $(CPP_SRCS) $(FE_HDRS) $(CPP_HDRS)
	$(CC) $(CSTD) $(CFLAGS) $(CPP_INCS) -o $@ \
	    test_phase10.c $(FE_SRCS) $(CPP_SRCS)

# ── test_phase8 ─────────────────────────────────────────────────────────────
test_phase8: test_phase8.c $(FE_SRCS) $(CPP_SRCS) $(FE_HDRS) $(CPP_HDRS)
	$(CC) $(CSTD) $(CFLAGS) $(CPP_INCS) -o $@ \
	    test_phase8.c $(FE_SRCS) $(CPP_SRCS)

# ── test_phase7 ─────────────────────────────────────────────────────────────
test_phase7: test_phase7.c $(FE_SRCS) $(CPP_SRCS) $(FE_HDRS) $(CPP_HDRS)
	$(CC) $(CSTD) $(CFLAGS) $(CPP_INCS) -o $@ \
	    test_phase7.c $(FE_SRCS) $(CPP_SRCS)

# ── test_phase6 ─────────────────────────────────────────────────────────────
test_phase6: test_phase6.c $(FE_SRCS) $(CPP_SRCS) $(FE_HDRS) $(CPP_HDRS)
	$(CC) $(CSTD) $(CFLAGS) $(CPP_INCS) -o $@ \
	    test_phase6.c $(FE_SRCS) $(CPP_SRCS)

# ── test_phase5 ─────────────────────────────────────────────────────────────
test_phase5: test_phase5.c $(FE_SRCS) $(CPP_SRCS) $(FE_HDRS) $(CPP_HDRS)
	$(CC) $(CSTD) $(CFLAGS) $(CPP_INCS) -o $@ \
	    test_phase5.c $(FE_SRCS) $(CPP_SRCS)

# ── test_phase4 ─────────────────────────────────────────────────────────────
test_phase4: test_phase4.c $(FE_SRCS) $(CPP_SRCS) $(FE_HDRS) $(CPP_HDRS)
	$(CC) $(CSTD) $(CFLAGS) $(CPP_INCS) -o $@ \
	    test_phase4.c $(FE_SRCS) $(CPP_SRCS)

# ── test_phase2 ─────────────────────────────────────────────────────────────
test_phase2: test_phase2.c $(FE_SRCS) $(CPP_SRCS) $(FE_HDRS) $(CPP_HDRS)
	$(CC) $(CSTD) $(CFLAGS) $(CPP_INCS) -o $@ \
	    test_phase2.c $(FE_SRCS) $(CPP_SRCS)

# ── test ───────────────────────────────────────────────────────────────────
# Only includes test binaries actually shipped in this distribution.
# (Per-phase tests test_phase1..11 are kept by the original maintainers but
#  are not redistributed here; if you regenerate them, add them back.)
test: probe sharpc test_integration test_import test_decls
	@echo "===== Phase 0 probes =====" && ./probe
	@echo
	@echo "===== Integration tests (end-to-end) =====" && ./test_integration
	@echo
	@echo "===== Import system tests =====" && ./test_import
	@echo
	@echo "===== Multi-variable + array decl tests =====" && ./test_decls
	@echo

# ── asan ───────────────────────────────────────────────────────────────────
asan: CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer -O0
asan: clean test

# ── strict ─────────────────────────────────────────────────────────────────
strict: CFLAGS += -Wpedantic -Wshadow -Wstrict-prototypes -Werror
strict: clean test

# ── dist ───────────────────────────────────────────────────────────────────
dist:
	@echo "Packaging sharp-phase$(DIST_PHASE).tar.gz (fe + cpp) ..."
	tar -czf /mnt/user-data/outputs/sharp-phase$(DIST_PHASE).tar.gz \
	    --exclude='.git' --exclude='*.o' \
	    --exclude='sharp-fe/probe' --exclude='sharp-cpp/sharp-cpp/probe' \
	    --exclude='sharp-fe/test_phase*' \
	    --exclude='sharp-cpp/sharp-cpp/test_phase*' \
	    --exclude='sharp-cpp/sharp-cpp/test_lua*' \
	    --exclude='sharp-cpp/sharp-cpp/test_mcpp*' \
	    --exclude='sharp-cpp/sharp-cpp/test_sharp*' \
	    --exclude='sharp-cpp/sharp-cpp/sharp_prof' \
	    --exclude='sharp-cpp/sharp-cpp/bench*' \
	    --exclude='sharp-cpp/sharp-cpp/gmon.out' \
	    --exclude='sharp-cpp/sharp-cpp/*.sh' \
	    --exclude='sharp-cpp/sharp-cpp/test_lua_includes' \
	    --exclude='sharp-cpp/sharp-cpp/test_mcpp_includes' \
	    --exclude='sharp-cpp/sharp-cpp/test_lua_includes.c' \
	    --exclude='sharp-cpp/sharp-cpp/test_mcpp_includes.c' \
	    -C /home/claude sharp-fe sharp-cpp
	@echo "Done -> /mnt/user-data/outputs/sharp-phase$(DIST_PHASE).tar.gz"

# ── clean ──────────────────────────────────────────────────────────────────
clean:
	rm -f probe $(patsubst %.c,%,$(wildcard test_phase*.c)) \
	      sharp_fe_cli sharpc bench_bin *.o
