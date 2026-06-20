# pomodoro64

Pomodoro timer for the Commodore 64, written in C with cc65, run in VICE. macOS dev host.

## Build & run
- `make` / `./build.sh build` — compile to `build/pomodoro64.prg` (+ `.lbl`, `.dbg`, `.map`)
- `make run` / `./build.sh run` — build and launch in VICE (`x64sc`), label file loaded into the monitor
- `make clean` — remove `build/` and `src/*.o`
- Sources are modular: `main.c` (state machine + conio UI), `tod.c` (CIA TOD clock), `bigfont.c` (PETSCII big digits), `sound.c` (SID). Add new `.c` files to `SRCS` in the Makefile.

## Environment
- Toolchain: `brew install cc65 vice`. VICE is a **formula, not a cask** (`brew install vice`); the `vice` cask no longer exists.
- cc65 headers: `/opt/homebrew/share/cc65/include` (stable symlink; prefer over the versioned Cellar path).
- cc65 2.19 (current Homebrew) has **no `get_tv()`/`TV_NTSC` for the c64 target** — detect PAL/NTSC manually via the VIC-II raster counter (only PAL exceeds line 271). See `tod.c`.

## Headless testing in VICE
- Run non-interactively: `x64sc -default -warp +confirmonexit -sounddev dummy -keybuf "<keys>" -limitcycles <N> -exitscreenshot /tmp/x.png build/pomodoro64.prg`, then Read the PNG to verify the screen.
- `-keybuf` injects keystrokes (e.g. `" s"` = space then s); `-limitcycles` exits after N cycles (the run always logs "cycle limit reached" — expected, not an error). Bump cycles to let a phase run to completion in warp mode.

## C64 / cc65 gotchas
- Source strings must be **lowercase**; cc65 maps them to PETSCII and the default uppercase charset renders them as caps. ASCII uppercase becomes graphics symbols.
- Set `VIC.addr = 0x15` to select the uppercase/graphics charset — PETSCII circle codes (`$51`/`$57`, used for the pomodoro markers) only exist there.
- Big digits and color are written straight to screen/color RAM (`$0400`/`$D800`), bypassing conio.
- SID notes in `sound.c` busy-wait on the raster — only call them on state transitions, never inside the timer loop.
- Comments and UI strings in this project are in Italian — match that style.

## VSCode IntelliSense
- `.vscode/c_cpp_properties.json` points IntelliSense at the cc65 includes and neutralizes `__fastcall__`/`__cdecl__`. The C/C++(1696) "update includePath" error means that config isn't selected — reload window or pick the "C64 (cc65)" configuration.
