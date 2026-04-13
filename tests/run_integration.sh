#!/bin/bash
# Integration test runner for dsl-codegen
# Usage: ./run_integration.sh [build_dir]

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${1:-$SCRIPT_DIR/../build}"
DSL_CODEGEN="$BUILD_DIR/dsl-codegen"
FIXTURES="$SCRIPT_DIR/fixtures"
EXIT_CODE=0

echo "=== DSL Codegen Integration Tests ==="

# Test 1: PI Controller
echo ""
echo "Test 1: PI Controller generation"
if "$DSL_CODEGEN" "$FIXTURES/pi_controller.xml" > /tmp/pi_output.c 2>/tmp/pi_stderr.txt; then
    if diff -q /tmp/pi_output.c "$FIXTURES/pi_controller_expected.c" > /dev/null 2>&1; then
        echo "  PASS: Output matches expected"
    else
        echo "  FAIL: Output differs from expected"
        diff /tmp/pi_output.c "$FIXTURES/pi_controller_expected.c" || true
        EXIT_CODE=1
    fi
else
    echo "  FAIL: dsl-codegen exited with error"
    cat /tmp/pi_stderr.txt
    EXIT_CODE=1
fi

# Test 2: Algebraic loop detection
echo ""
echo "Test 2: Algebraic loop detection"
if "$DSL_CODEGEN" "$FIXTURES/algebraic_loop.xml" > /dev/null 2>/tmp/loop_stderr.txt; then
    echo "  FAIL: Expected error exit code, but succeeded"
    EXIT_CODE=1
else
    if grep -q "algebraic loop detected" /tmp/loop_stderr.txt; then
        echo "  PASS: Algebraic loop detected"
    else
        echo "  FAIL: Wrong error message"
        cat /tmp/loop_stderr.txt
        EXIT_CODE=1
    fi
fi

# Test 3: Determinism check
echo ""
echo "Test 3: Determinism check"
"$DSL_CODEGEN" "$FIXTURES/pi_controller.xml" > /tmp/pi_run1.c
"$DSL_CODEGEN" "$FIXTURES/pi_controller.xml" > /tmp/pi_run2.c
if cmp -s /tmp/pi_run1.c /tmp/pi_run2.c; then
    echo "  PASS: Outputs are byte-identical"
else
    echo "  FAIL: Outputs differ between runs"
    EXIT_CODE=1
fi

echo ""
if [ $EXIT_CODE -eq 0 ]; then
    echo "All integration tests passed."
else
    echo "Some tests failed."
fi

exit $EXIT_CODE
