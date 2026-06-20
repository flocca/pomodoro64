# pomodoro64

Pomodoro timer per Commodore 64, scritto in C con [cc65](https://cc65.github.io/).

## Requisiti

Servono il compilatore **cc65** e l'emulatore **VICE** (binario `x64sc`).

**macOS** (Homebrew):

```sh
brew install cc65 vice
```

**Linux**:

```sh
# Debian / Ubuntu
sudo apt install cc65 vice

# Fedora
sudo dnf install cc65 vice

# Arch
sudo pacman -S cc65 vice
```

Su alcune distribuzioni il pacchetto VICE non include le ROM della
Commodore per motivi di licenza: se `x64sc` si lamenta di ROM mancanti,
installale con [`vice-roms`](https://vice-emu.sourceforge.io/) o copia
`kernal`/`basic`/`chargen` in `~/.local/share/vice/C64/`.

## Build e run

```sh
./build.sh run      # compila e lancia in VICE (x64sc)
./build.sh build    # solo compilazione
./build.sh clean    # pulizia
```

Da VSCode: F5 ("Build & Run in VICE") oppure Cmd+Shift+B per la sola build.

La build produce in `build/`:

- `pomodoro64.prg` — eseguibile C64
- `pomodoro64.lbl` — label per il monitor di VICE (caricate automaticamente da `make run`)
- `pomodoro64.dbg` — debug info cc65
- `pomodoro64.map` — mappa del linker

## Immagini disco e drive reale

Due strumenti distinti, con scopi diversi:

- **`c1541`** (incluso in VICE) lavora **lato host** sui file immagine `.d64`.
- **OpenCBM** (`brew install opencbm`) scrive su un **floppy 1541 reale** tramite
  uno **ZoomFloppy** (interfaccia XUM1541 via USB). I target `disk*` richiedono
  che OpenCBM sia installato e lo ZoomFloppy collegato; `UNIT=8` di default.

```sh
# Immagine .d64 (host)
make d64                       # immagine dedicata col solo .prg -> build/pomodoro64.d64
make d64-add D64=raccolta.d64  # aggiunge il .prg a un'immagine esistente (altri file intatti)

# Floppy reale via ZoomFloppy + OpenCBM
make disk-detect               # verifica che ZoomFloppy e drive rispondano (lancialo per primo)
make disk                      # scrive il .prg sul floppy, affiancandolo agli altri file
make disk-format               # formatta il floppy e poi scrive il .prg (disco dedicato)
make disk-image D64=build/pomodoro64.d64  # copia un'intera immagine sul floppy
```

Il nome del file su disco è `pomodoro64` (variabile `CBMNAME`); etichetta e id
del disco sono `pomodoro64,p1` (`DISKNAME`). Cambia `UNIT` se il drive non è l'8.

## Uso

Nel menu: **F1/F3/F5** cambiano le durate di lavoro / pausa breve / pausa
lunga, **SPAZIO** avvia. Durante una fase: **SPAZIO** pausa/riprendi,
**S** salta alla fase successiva, **R** torna al menu. Ogni 4 pomodori
completati la pausa è lunga.

## Architettura

| Modulo | Ruolo |
|---|---|
| `src/main.c` | macchina a stati (menu → lavoro → pausa), UI conio |
| `src/tod.c` | cronometro sul Time-of-Day clock del CIA #1: hardware, non deriva; pausa = stop del clock scrivendo il registro ore |
| `src/bigfont.c` | cifre giganti 5×10 in PETSCII, scrittura diretta in memoria video ($0400/$D800) |
| `src/sound.c` | beep e jingle sulla voce 1 del SID ($D400) |

Il rilevamento PAL/NTSC (per il flag 50/60Hz del TOD) legge il contatore
raster a 9 bit del VIC-II: solo il PAL supera la linea 271.
