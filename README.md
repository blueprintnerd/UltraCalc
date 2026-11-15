# UltraCalc

A multi-interface calculator suite with a shared C++ math engine, CLI, TUI, PyQt6 desktop GUI, HTTP server with visual UI generator, and extension manager.

## Building

Run the build script from the repository root:

```bash
./build.sh --clean
```

Flags:
- `--clean`: Remove `build/` and `bin/` before building
- `--debug`: Configure with `Debug`
- `--release`: Configure with `Release` (default)

All executables and runtime assets are staged into `bin/`:
- `bin/ultracalc-cli`
- `bin/ultracalc-tui`
- `bin/ultracalc-gui` (with `bin/guiqt.py`)
- `bin/ultracalc-server` (with `bin/uigen.html`)
- `bin/ultracalc-ext`

## Usage

### CLI (`ultracalc-cli`)

Interactive REPL:
```bash
./bin/ultracalc-cli
```

Canonical API mode:
```bash
./bin/ultracalc-cli --calcapi mul 8 7
```

### TUI (`ultracalc-tui`)

```bash
./bin/ultracalc-tui
```

### Desktop GUI (`ultracalc-gui`)

Requires Python 3 and `PyQt6` (`pip install PyQt6`):
```bash
./bin/ultracalc-gui
```

### Web Server & UI Generator (`ultracalc-server`)

```bash
./bin/ultracalc-server --host 0.0.0.0 --port 8080
```
Open `http://localhost:8080/` in a browser.

### Extension Manager (`ultracalc-ext` or `ultracalc-cli ext`)

```bash
./bin/ultracalc-ext validate my-ext.uce
./bin/ultracalc-ext install my-ext.uce
./bin/ultracalc-ext list

# Or via the CLI (directly or inside the REPL):
./bin/ultracalc-cli ext install my-ext.uce
```

## Testing

```bash
ctest --test-dir build --output-on-failure
```

## Documentation

- [ultracalcAPI.md](ultracalcAPI.md) - CLI `--calcapi` and HTTP REST API specification
- [CALCULATOR_GUIDE.md](CALCULATOR_GUIDE.md) - TUI calculator guide
