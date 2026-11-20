# UltraCalc

A cool calculator app
## Building

Run the build script from the repository root:

```bash
./build.sh --clean
```can
## Usage

### CLI (`ultracalc-cli`)

```bash
./bin/ultracalc-cli
```

API:
```bash
./bin/ultracalc-cli --calcapi mul 8 7
```

### TUI (`ultracalc-tui`)

```bash
./bin/ultracalc-tui
```

### Desktop GUI (`ultracalc-gui`)

Requires Python 3 and PyQt6
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

./bin/ultracalc-cli ext install my-ext.uce
```

