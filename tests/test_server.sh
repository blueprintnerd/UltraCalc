#!/usr/bin/env bash
set -euo pipefail

SERVER_BIN="${1:-./bin/ultracalc-server}"
if [[ ! -x "$SERVER_BIN" ]]; then
    echo "Error: ultracalc-server binary not found at $SERVER_BIN" >&2
    exit 1
fi

PORT="$(python3 -c 'import socket; s=socket.socket(); s.bind(("127.0.0.1", 0)); print(s.getsockname()[1]); s.close()')"
BASE_URL="http://127.0.0.1:${PORT}"

"$SERVER_BIN" --host 127.0.0.1 --port "$PORT" >/tmp/ultracalc_server_test.log 2>&1 &
SERVER_PID=$!

cleanup() {
    kill "$SERVER_PID" 2>/dev/null || true
    wait "$SERVER_PID" 2>/dev/null || true
}
trap cleanup EXIT

# Wait for server readiness
for _ in $(seq 1 30); do
    if curl -fsS "${BASE_URL}/api/health" >/dev/null 2>&1; then
        break
    fi
    sleep 0.1
done

# Test GET /
ROOT_CODE="$(curl -s -o /tmp/ultracalc_root.html -w "%{http_code}" "${BASE_URL}/")"
if [[ "$ROOT_CODE" != "200" ]]; then
    echo "FAIL: GET / returned HTTP $ROOT_CODE" >&2
    exit 1
fi
if ! grep -q "UltraCalc" /tmp/ultracalc_root.html; then
    echo "FAIL: GET / did not return UltraCalc HTML" >&2
    exit 1
fi

# Test GET /api/calculate
GET_RESP="$(curl -fsS "${BASE_URL}/api/calculate?op=add&a=10&b=5")"
python3 -c "import json, sys; d=json.loads(sys.argv[1]); assert d['result'] == 15" "$GET_RESP"

# Test POST /api/calculate
POST_RESP="$(curl -fsS -X POST -H "Content-Type: application/json" -d '{"op":"mul","a":6,"b":7}' "${BASE_URL}/api/calculate")"
python3 -c "import json, sys; d=json.loads(sys.argv[1]); assert d['result'] == 42" "$POST_RESP"

# Test division by zero -> HTTP 400
DIV_ZERO_CODE="$(curl -s -o /tmp/ultracalc_divzero.json -w "%{http_code}" -X POST -H "Content-Type: application/json" -d '{"op":"div","a":10,"b":0}' "${BASE_URL}/api/calculate")"
if [[ "$DIV_ZERO_CODE" != "400" ]]; then
    echo "FAIL: division by zero returned HTTP $DIV_ZERO_CODE (expected 400)" >&2
    exit 1
fi
python3 -c "import json; d=json.load(open('/tmp/ultracalc_divzero.json')); assert 'error' in d"

# Test missing params -> HTTP 400
MISSING_CODE="$(curl -s -o /dev/null -w "%{http_code}" "${BASE_URL}/api/calculate?op=add&a=1")"
if [[ "$MISSING_CODE" != "400" ]]; then
    echo "FAIL: missing params returned HTTP $MISSING_CODE (expected 400)" >&2
    exit 1
fi

echo "All HTTP server tests passed!"
