# UltraCalc — Issues

Audit of the C++ sources (`cli.cpp`, `tui.cpp`, `math.cpp`, `extensionparser.cpp`) and the Python/Qt frontends.

---

## 1. `cli.cpp` — does not compile

The CLI entry point is unparsable as written.

| Line | Issue |
|------|-------|
| 29 (`}{;`) | Stray stray-brace group after the `sub` branch: `}{;` — dangling block and empty statement. |
| 33 | Extra `} }` at end of `lineparse` — one `}` too many everywhere; the nesting never balances. |
| 60 (`cout << left`) | Missing semicolon after the truncated statement `cout << left`. |
| 102 | The final `else if (line.find("mod") == 0)` branch computes nothing and never prints a result — right-hand side is missing. |
| 5 | `lineparse` takes `std::string&` but is called with temporaries; also no `using namespace std` / `std::` prefixes at all for `cout`, `string`, `size_t`. |
| 118–123 | The `-`, `*`, `/` handlers use hardcoded offsets (`minusposition - 4`, plus `+4`) that assume the operator string is at index 4 — wrong for any real input once tokens vary. |

**Fix:** the file needs restructuring around tokenize-then-dispatch (strip prefix, find split token, `stoi` both halves, call the `add/sub/mul/div` helpers from `math.cpp`). No incremental patch will survive — rewrite `lineparse`.

> Note — the whole file was rewritten by the audit into a working tokenize-then-dispatch version; drop that in rather than hand-patching offsets.

## 2. Duplicate / dead frontends

- `guiqt.py` (16 lines) is an unfinished stub — an empty `QWidget` shell with a bare `def` and an unused `pyqtSignal` import. It duplicates `qtgui.py` (209 lines, the real one). Delete `guiqt.py` unless it's meant as a scratch sketch.
- `serve.cpp` (0 lines) is committed but empty; either remove it from the repo or leave it out of `git` until there is content.

## 3. Python GUI and docs use wrong flags

- `qtgui.py` calls the engine with `-v`-style flags that don't exist in `math.hpp`'s API — `divider()` takes a numeric `flag_nodec`, not a named flag. Verify every subprocess call against `math.hpp`.
- `qtgui.py` line-1 shebang uses `#!/usr/bin/env python3` but never reports failures from the subprocess; stderr should be surfaced in the GUI, not swallowed.

## 4. Missing / weak test coverage

- `tests/test_extensionparser.cpp` is good — extend the same pattern to `math.cpp` (`add/sub/mul/div`, the `%`-vs-`divider()` flag behaviour) and, once rewritten, to `lineparse`.
- `minics.md` and `ultracalcBase.md` describe behaviour that isn't asserted anywhere; add at least one test per documented example.

## 5. Minor hygiene

- `ultracalcbase.o` and other build intermediates are committed — add a `.gitignore` (`build/`, `*.o`).
- `minics.md`, `ultracalcBase.md`, `README`, and `about.md` disagree on the engine's name and flags; pick one canonical command surface and document it once (e.g. `ultracalcAPI.md`), then reference it from everywhere else.
- `docs/CALCULATOR_GUIDE.md` mentions `pow` and `sqrt`, which the engine doesn't expose — either mark it as roadmap in the guide or implement it.

---

### Suggested order

1. Rewrite `lineparse` (section 1) — everything else in the CLI blocks on this.
2. Consolidate the frontends: keep `qtgui.py`, drop `guiqt.py`.
3. Grow the test suite, porting the `test_extensionparser` harness pattern.
4. Reconcile the docs against the real CLI surface (sections 3, 5).

---

## What you need to finish the app

**Must-have before calling it 1.0**

1. Working `lineparse` in the CLI (see section 1) — the app has no usable entry point until this compiles and dispatches correctly.
2. Wire the extension system end to end: `extensionparser.cpp` can extract `.zip`/`.xz` archives, but nothing in the CLI or TUI calls `dearchive()`. Add an `ext load <file>` command and a manifest check (`manifest.toml`) after extraction.
3. Decide on one canonical binary + command surface (`ultracalc` with the documented verbs) and make the docs, `README.md`, and the Qt GUI target exactly that.
4. Error handling contract: every `std::stoi`/`std::stof` call in `cli.cpp` and `qtgui.py` can throw on bad input. Wrap them, print a friendly message, and keep the REPL/GUI alive.
5. CI + tests: a `cmake && ctest` workflow (GitHub Actions) running the existing `test_extensionparser` plus new tests for `math.cpp` and `lineparse`.

**Nice-to-have later**

6. `pow`, `sqrt`, and parentheses in the expression engine (already listed as roadmap in `docs/CALCULATOR_GUIDE.md`).
7. Load-extension sandboxing review: currently extracted entries are only path-traversal-checked at extract time; verify extension code can't escape its directory at load time.
8. Identifier checks on shipped extensions so an extension can't shadow builtin commands (`add`, `sub`, `mul`, `div`, `mod`).
