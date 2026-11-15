#!/usr/bin/env bash
set -euo pipefail

CLI_BIN="${1:-./bin/ultracalc-cli}"
if [[ ! -x "$CLI_BIN" ]]; then
    echo "Error: ultracalc-cli binary not found at $CLI_BIN" >&2
    exit 1
fi

assert_eq() {
    local actual="$1"
    local expected="$2"
    local label="$3"
    if [[ "$actual" != "$expected" ]]; then
        echo "FAIL [$label]: expected '$expected', got '$actual'" >&2
        exit 1
    fi
}

# --calcapi tests
assert_eq "$("$CLI_BIN" --calcapi add 10 5)" "15" "calcapi add"
assert_eq "$("$CLI_BIN" --calcapi sub 10 3)" "7" "calcapi sub"
assert_eq "$("$CLI_BIN" --calcapi mul 8 7)" "56" "calcapi mul"
assert_eq "$("$CLI_BIN" --calcapi div 20 4)" "5" "calcapi div"
assert_eq "$("$CLI_BIN" --calcapi mod 17 5)" "2" "calcapi mod"
assert_eq "$("$CLI_BIN" --calcapi add -4 10)" "6" "calcapi negative"

# Error cases (must exit non-zero)
if "$CLI_BIN" --calcapi div 10 0 >/dev/null 2>&1; then
    echo "FAIL: division by zero should exit non-zero" >&2
    exit 1
fi

if "$CLI_BIN" --calcapi badop 1 2 >/dev/null 2>&1; then
    echo "FAIL: invalid operator should exit non-zero" >&2
    exit 1
fi

if "$CLI_BIN" --calcapi add abc 2 >/dev/null 2>&1; then
    echo "FAIL: non-numeric operand should exit non-zero" >&2
    exit 1
fi

# CLI ext subcommand tests (.uce)
EXT_TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$EXT_TMP_DIR"' EXIT
python3 -c "
import zipfile
zf = zipfile.ZipFile('${EXT_TMP_DIR}/sample.uce', 'w')
zf.writestr('manifest.json', '{\"name\":\"SampleExt\",\"version\":\"1.0.0\",\"description\":\"Sample\",\"main\":\"index.html\"}')
zf.writestr('index.html', '<h1>Sample</h1>')
zf.close()
"
"$CLI_BIN" ext validate "${EXT_TMP_DIR}/sample.uce" | grep -q "SampleExt"
"$CLI_BIN" ext install "${EXT_TMP_DIR}/sample.uce" -o "${EXT_TMP_DIR}/installed" | grep -q "SampleExt"
"$CLI_BIN" ext list -d "${EXT_TMP_DIR}/installed" | grep -q "SampleExt"

# Interactive REPL ext install + calculation tests
REPL_EXT_DIR="${EXT_TMP_DIR}/repl_installed"
REPL_OUT="$(printf "ext install ${EXT_TMP_DIR}/sample.uce -o ${REPL_EXT_DIR}\next list -d ${REPL_EXT_DIR}\nadd 43, 54\nsub 12 from 32\nmul 6, 7\ndiv 8 by 2\nmod 17 by 5\n3 + 4\n10 - 3\n6 * 7\n8 / 2\n10 %% 3\nbad input\nexit\n" | "$CLI_BIN" 2>/dev/null)"

for expected in "Installed extension: SampleExt" "97" "20" "42" "4" "2" "7"; do
    if ! echo "$REPL_OUT" | grep -q "$expected"; then
        echo "FAIL [REPL]: expected '$expected' in output" >&2
        echo "$REPL_OUT" >&2
        exit 1
    fi
done

echo "All CLI integration tests passed!"
