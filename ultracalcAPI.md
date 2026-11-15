# UltraCalc API

UltraCalc exposes two canonical calculation APIs: the CLI `--calcapi` interface and the HTTP REST API served by `ultracalc-server`.

## 1. CLI `--calcapi` Interface

```bash
ultracalc-cli --calcapi <operator> <num1> <num2>
```

### Operators

- `add`: Addition (`num1 + num2`)
- `sub`: Subtraction (`num1 - num2`)
- `mul`: Multiplication (`num1 * num2`)
- `div`: Division (`num1 / num2`)
- `mod`: Modulo (`num1 % num2`)

On success, `ultracalc-cli` prints the plain-text numeric result to `stdout` and exits with code `0`. On invalid input or division/modulo by zero, it prints an error message to `stderr` and exits with a non-zero code.

## 2. HTTP REST API (`ultracalc-server`)

Start the server:
```bash
ultracalc-server --host 0.0.0.0 --port 8080
```

### Endpoints

- `GET /`: Serves `uigen.html` (UltraCalc UI Generator & interactive calculator).
- `GET /api/health`: Returns `{"status": "ok"}`.
- `GET /api/calculate?op=<op>&a=<num1>&b=<num2>`: Evaluates the operation and returns JSON.
- `POST /api/calculate`: Accepts JSON body `{"op": "<op>", "a": <num1>, "b": <num2>}`.

### Responses

- `200 OK`: `{"result": <value>}`
- `400 Bad Request`: `{"error": "<message>"}`
