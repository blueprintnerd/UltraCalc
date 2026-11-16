# UltraCalc v1.0 Shipping Checklist

This checklist tracks all remaining engineering tasks, bug fixes, architecture alignments, and validation steps required to ship UltraCalc v1.0 as a production-ready multi-interface suite.

---

## Architecture Overview & Deliverables

UltraCalc v1.0 will ship as five standalone executables and shared runtime assets:

1. `ultracalc-cli`: Command-line calculator engine supporting interactive REPL, natural language expressions, and the canonical `--calcapi` interface.
3. `ultracalc-gui`: Native C++ launcher wrapper executing the PyQt6 desktop frontend (`guiqt.py`).
4. `ultracalc-server`: Lightweight HTTP server using `cpp-httplib` serving `uigen.html` and a JSON REST API.
5. `ultracalc-ext`: Extension management binary for installing, listing, and verifying extension archives.

---

## Phase 1: Core Engine and CLI (`cli.cpp`, `math.cpp`, `math.hpp`)

- [ ] **Fix syntax errors in [cli.cpp](file:///root/Orbit/Projects/UltraCalc/cli.cpp)**
  - [ ] Remove stray braces, empty statements (`}{;`), and unmatched closing braces.
  - [ ] Add missing standard library namespace qualifications (`std::string`, `std::cout`, `std::size_t`).
  - [ ] Add missing semicolon on print statements.
- [ ] **Refactor [cli.cpp](file:///root/Orbit/Projects/UltraCalc/cli.cpp) dispatch logic**
  - [ ] Replace fragile fixed character offset indexing with a robust tokenizer.
  - [ ] Support natural language syntax: `add <num1>, <num2>`, `sub <num1> from <num2>`, `mul <num1>, <num2>`, `div <num1> by <num2>`, `mod <num1> by <num2>`.
  - [ ] Support operator syntax: `<num1> + <num2>`, `<num1> - <num2>`, `<num1> * <num2>`, `<num1> / <num2>`, `<num1> % <num2>`.
  - [ ] Implement canonical API flag `--calcapi <operator> <num1> <num2>` printing raw output for script integration.
- [ ] **Defensive error handling in [math.cpp](file:///root/Orbit/Projects/UltraCalc/math.cpp) and [cli.cpp](file:///root/Orbit/Projects/UltraCalc/cli.cpp)**
  - [ ] Catch numeric parsing exceptions (`std::invalid_argument`, `std::out_of_range`).
  - [ ] Handle division by zero gracefully in `divider()` instead of crashing.
  - [ ] Return descriptive error messages to standard error with appropriate non-zero exit codes.
- [ ] **CMake Integration**
  - [ ] Add `ultracalc-cli` target to [CMakeLists.txt](file:///root/Orbit/Projects/UltraCalc/CMakeLists.txt).

---

## Phase 2: Extension Engine (`extensionparser.cpp`, `extensionparser.hpp`)

- [ ] **Vendor missing JSON dependency**
  - [ ] Add single-header `json.hpp` (nlohmann/json) to include path to resolve missing include compilation failure.
- [ ] **Build and binary packaging**
  - [ ] Update [CMakeLists.txt](file:///root/Orbit/Projects/UltraCalc/CMakeLists.txt) executable target name to `ultracalc-ext`.
  - [ ] Implement CLI subcommands for `ultracalc-ext`:
    - [ ] `install <path-to-archive>`: Extracts `.zip` archive to extensions directory.
    - [ ] `list`: Lists installed extensions and parsed metadata.
    - [ ] `validate <path-to-archive>`: Verifies archive structure and manifest schema without installing.
- [ ] **Manifest schema enforcement**
  - [ ] Validate required fields in `manifest.json` (`name`, `version`, `description`, `main`).
  - [ ] Normalize paths to prevent directory traversal outside the extensions directory.
- [ ] **Defer experimental layout engine**
  - [ ] Keep [frontendlang.cpp](file:///root/Orbit/Projects/UltraCalc/frontendlang.cpp) isolated from the v1.0 build path.

---

## Phase 4: Desktop GUI & Native Launcher (`guiqt.py`, `gui_launcher.cpp`)

- [ ] **Decouple environment paths in [guiqt.py](file:///root/Orbit/Projects/UltraCalc/guiqt.py)**
  - [ ] Remove hardcoded `/vercel/sandbox/UltraCalc` working directory in subprocess calls.
  - [ ] Resolve `ultracalc-cli` binary dynamically (check application directory, sibling `bin/` directory, and system `PATH`).
- [ ] **Subprocess communication & error reporting**
  - [ ] Capture subprocess standard error and display failure details on the status label instead of failing silently.
  - [ ] Ensure floating-point and integer results format consistently on the LCD display.
- [ ] **C++ GUI launcher (`ultracalc-gui`)**
  - [ ] Implement native C++ wrapper (`gui_launcher.cpp`) that locates the system Python 3 interpreter and runs `guiqt.py`.
  - [ ] Add user-friendly diagnostic message if PyQt6 is not installed.
  - [ ] Add `ultracalc-gui` executable target to [CMakeLists.txt](file:///root/Orbit/Projects/UltraCalc/CMakeLists.txt).

---

## Phase 5: Web Server & Web Frontend (`serve.cpp`, `uigen.html`)

- [ ] **Vendor `cpp-httplib` header**
  - [ ] Add `httplib.h` single-header library into third-party include directory.
- [ ] **Implement [serve.cpp](file:///root/Orbit/Projects/UltraCalc/serve.cpp)**
  - [ ] Configure HTTP server listening on configurable host and port (default: `0.0.0.0:8080`).
  - [ ] Add static file serving for [uigen.html](file:///root/Orbit/Projects/UltraCalc/uigen.html) at route `/`.
  - [ ] Add REST calculation endpoint `GET /api/calculate` with parameters `op`, `a`, `b`.
  - [ ] Add REST calculation endpoint `POST /api/calculate` with JSON payload `{"op": "...", "a": ..., "b": ...}`.
  - [ ] Return JSON payload with structure `{"result": <value>}` or `{"error": "<message>"}` with matching HTTP status codes.
  - [ ] Add `ultracalc-server` executable target to [CMakeLists.txt](file:///root/Orbit/Projects/UltraCalc/CMakeLists.txt).
- [ ] **Connect Web UI ([uigen.html](file:///root/Orbit/Projects/UltraCalc/uigen.html))**
  - [ ] Update frontend button event listeners to trigger asynchronous `fetch()` requests against `/api/calculate`.
  - [ ] Display returned calculation or error state in the calculator display element.

---

## Phase 6: Automated Build Script & Local Packaging

- [ ] **Create build and packaging script (`build.sh`)**
  - [ ] Automate CMake configuration, compilation of all five targets, and asset staging.
  - [ ] Stage all compiled executables into a central `bin/` directory:
    - `bin/ultracalc-cli`
    - `bin/ultracalc-tui`
    - `bin/ultracalc-ext`
    - `bin/ultracalc-server`
    - `bin/ultracalc-gui`
  - [ ] Copy required runtime assets alongside binaries:
    - `bin/guiqt.py`
    - `bin/uigen.html`
  - [ ] Support `--clean`, `--debug`, and `--release` flags.
- [ ] **Repository hygiene**
  - [ ] Remove committed intermediate binary [ultracalcbase.o](file:///root/Orbit/Projects/UltraCalc/ultracalcbase.o) from git tracking.
  - [ ] Update [.gitignore](file:///root/Orbit/Projects/UltraCalc/.gitignore) to ignore `build/`, `bin/`, `*.o`, `*.a`, `.cache/`, and python `__pycache__/`.

---

## Phase 7: Automated Test Suite

- [ ] **Core math unit tests**
  - [ ] Implement `tests/test_math.cpp` verifying `add`, `subtract`, `multiplier`, `divider`, and modulo.
  - [ ] Verify division-by-zero behavior.
- [ ] **CLI integration tests**
  - [ ] Implement `tests/test_cli.sh` testing `--calcapi` flags, operator forms, natural language forms, and invalid inputs.
- [ ] **Extension parser unit tests**
  - [ ] Fix and run [tests/test_extensionparser.cpp](file:///root/Orbit/Projects/UltraCalc/tests/test_extensionparser.cpp).
  - [ ] Test zip extraction, manifest parsing, missing manifest rejection, and malformed archive handling.
- [ ] **HTTP server endpoint tests**
  - [ ] Implement automated smoke test sending requests to `/api/calculate` and verifying response codes and payloads.
- [ ] **CTest registration**
  - [ ] Register all tests under `add_test()` in [CMakeLists.txt](file:///root/Orbit/Projects/UltraCalc/CMakeLists.txt).

---

## Phase 8: Documentation & Canonical API Reconciliation

- [ ] **Reconcile documentation**
  - [ ] Update [README.md](file:///root/Orbit/Projects/UltraCalc/README.md) with clear build instructions using `build.sh` and usage examples for all five executables.
  - [ ] Update [ultracalcAPI.md](file:///root/Orbit/Projects/UltraCalc/ultracalcAPI.md) to define both the CLI `--calcapi` interface and the HTTP REST API specifications.
  - [ ] Update [CALCULATOR_GUIDE.md](file:///root/Orbit/Projects/UltraCalc/CALCULATOR_GUIDE.md) to reflect current keybindings, features, and roadmapped operations (`pow`, `sqrt`).
- [ ] **Final release sign-off**
  - [ ] Run `build.sh` cleanly from scratch.
  - [ ] Run `ctest --output-on-failure` with all tests passing.
  - [ ] Manual verification of each interface in `bin/`.
