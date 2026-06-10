# pomodoro64 - Makefile per cc65 + VICE
#
# Target principali:
#   make          -> compila build/pomodoro64.prg (+ .dbg, .lbl, .map)
#   make run      -> compila e lancia in VICE (x64sc) con label caricate nel monitor
#   make clean    -> rimuove la cartella build

PROG    := pomodoro64
SRCS    := src/main.c src/tod.c src/bigfont.c src/sound.c
BUILD   := build

CL65    := cl65
EMU     := x64sc

PRG     := $(BUILD)/$(PROG).prg
DBG     := $(BUILD)/$(PROG).dbg
LBL     := $(BUILD)/$(PROG).lbl
MAP     := $(BUILD)/$(PROG).map

# -t c64   : target Commodore 64
# -O       : ottimizzazioni del compilatore
# -g       : informazioni di debug nei file oggetto
# -Ln      : label file in formato VICE (caricabile nel monitor con `ll`)
# --dbgfile: file di debug completo (sorgenti/linee/simboli) per debugger esterni
# -m       : mappa del linker (layout memoria, utile per capire dove finisce il codice)
CFLAGS  := -t c64 -O -g
LDFLAGS := -Ln $(LBL) -Wl --dbgfile,$(DBG) -m $(MAP)

.PHONY: all run clean

all: $(PRG)

$(PRG): $(SRCS) | $(BUILD)
	$(CL65) $(CFLAGS) $(LDFLAGS) -o $@ $(SRCS)

$(BUILD):
	mkdir -p $(BUILD)

# -moncommands carica le label nel monitor di VICE:
# apri il monitor (Alt+H / dal menu) e i simboli C (es. .main) sono gia' definiti.
run: $(PRG)
	$(EMU) -moncommands $(LBL) $(PRG)

clean:
	rm -rf $(BUILD) src/*.o
