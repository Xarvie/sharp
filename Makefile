# Makefile for sharpc
CC     ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-switch

SRC = util.c types.c lexer.c parser.c sema.c lower.c hir.c cgen.c main.c
OBJ = $(SRC:.c=.o)
BIN = sharpc

# Preprocessor sub-module
CPP_SRC = cpp/pptok.c cpp/macro.c cpp/expr.c cpp/directive.c cpp/cpp.c
CPP_OBJ = $(CPP_SRC:.c=.o)
CPP_BIN = sharp-cpp
CPP_FLAGS = $(CFLAGS) -Icpp -Wno-implicit-fallthrough

all: $(BIN) $(CPP_BIN)

$(BIN): $(OBJ) $(CPP_OBJ)
	$(CC) $(CFLAGS) -Icpp -o $@ $(OBJ) $(CPP_OBJ) $(LDFLAGS)

%.o: %.c sharp.h types.h ast.h hir.h
	$(CC) $(CFLAGS) -Icpp -c $< -o $@

# Preprocessor library object files
cpp/%.o: cpp/%.c cpp/cpp.h cpp/cpp_internal.h
	$(CC) $(CPP_FLAGS) -c $< -o $@

# Preprocessor standalone driver
$(CPP_BIN): $(CPP_OBJ) cpp/cpp_main.c
	$(CC) $(CPP_FLAGS) -o $@ $(CPP_OBJ) cpp/cpp_main.c

# Compile a Sharp source, then a C compile and run.
# Usage:  make run FILE=examples/hello.sp
run: $(BIN)
	./$(BIN) $(FILE) -o $(FILE:.sp=.c)
	$(CC) -std=c11 -O2 -Wall $(FILE:.sp=.c) -o $(FILE:.sp=.out)
	@echo "--- running $(FILE:.sp=.out) ---"
	@./$(FILE:.sp=.out); echo "exit: $$?"

clean:
	rm -f $(OBJ) $(BIN) $(CPP_OBJ) $(CPP_BIN) examples/*.c examples/*.out

.PHONY: all run test test-positive test-negative test-unit test-cpp clean

# ----------------------------------------------------------------------
# Regression suite. Split into positive (should-compile) and negative
# (should-fail-with-specific-diagnostic) halves. Both are required for
# `make test`.
#
# Negative tests prove that diagnostics actually fire when users write
# broken code — without them, an accidentally-permissive type checker
# would silently regress to the phase-7 "accept anything" behaviour.
# ----------------------------------------------------------------------

test: test-positive test-negative test-unit test-cpp
	@echo "all tests passed"

# Unit tests for the types interning module.
test-unit: $(BIN)
	@echo "--- unit tests (types module) ---"
	@$(CC) -std=c11 -O2 -Wall -I. -o /tmp/sharp_test_types \
	    tests/test_types.c types.c util.c
	@/tmp/sharp_test_types

# Preprocessor smoke test
test-cpp: $(CPP_OBJ)
	@echo "--- cpp smoke tests ---"
	@$(CC) -std=c11 -O2 -Wall -Icpp -o /tmp/sharp_test_cpp \
	    tests/test_cpp_smoke.c $(CPP_OBJ)
	@/tmp/sharp_test_cpp

test-positive: $(BIN)
	@echo "--- positive tests (should compile + produce expected exit) ---"
	@fail=0; \
	for t in hello:39 vec:98 raii:154 raii_edge:42 raii_stress:223 generic:42 param_dtor:42 io:99; do \
	  name=$${t%:*}; want=$${t##*:}; \
	  ./$(BIN) examples/$$name.sp -o examples/$$name.c || { fail=1; continue; }; \
	  if [ "$$name" = "generic" ]; then \
	    $(CC) -std=c11 -pedantic -O2 -Wall \
	        examples/$$name.c examples/generic_shim.c -o examples/$$name.out || { fail=1; continue; }; \
	  else \
	    $(CC) -std=c11 -pedantic -O2 -Wall \
	        examples/$$name.c -o examples/$$name.out || { fail=1; continue; }; \
	  fi; \
	  ./examples/$$name.out > /dev/null 2>&1; got=$$?; \
	  if [ "$$got" = "$$want" ]; then \
	    printf "  PASS  %-16s  exit=%s\n" "$$name" "$$got"; \
	  else \
	    printf "  FAIL  %-16s  exit=%s  want=%s\n" "$$name" "$$got" "$$want"; \
	    fail=1; \
	  fi; \
	done; \
	if [ $$fail -ne 0 ]; then echo "positive FAILED"; exit 1; fi

test-negative: $(BIN)
	@echo "--- negative tests (should fail with specific diagnostic) ---"
	@./tests/run_negative.sh ./$(BIN) tests/negative || exit 1
