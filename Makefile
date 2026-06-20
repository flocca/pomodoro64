# pomodoro64 - Makefile per cc65 + VICE
#
# Target principali:
#   make          -> compila build/pomodoro64.prg (+ .dbg, .lbl, .map)
#   make run      -> compila e lancia in VICE (x64sc) con label caricate nel monitor
#   make clean    -> rimuove la cartella build
#
# Immagini disco (.d64, lato host con c1541 di VICE):
#   make d64                 -> crea build/pomodoro64.d64 dedicata col solo .prg
#   make d64-add D64=x.d64   -> aggiunge il .prg a un'immagine esistente (altri file inclusi)
#
# Drive reale via ZoomFloppy + OpenCBM (richiede `brew install opencbm`, UNIT=8 default):
#   make disk                -> scrive il .prg sul floppy nel drive (non formatta)
#   make disk-format         -> formatta il floppy e poi scrive il .prg (disco dedicato)
#   make disk-image D64=x.d64-> copia un'intera immagine .d64 sul floppy (d64copy)

PROG     := pomodoro64
SRCS     := src/main.c src/tod.c src/bigfont.c src/sound.c
BUILD    := build

CL65     := cl65
EMU      := x64sc
C1541    := c1541

# Immagine/disco: nome CBM del file (max 16 char), etichetta+id del disco, unit OpenCBM
CBMNAME  := pomodoro64
DISKNAME := pomodoro64,p1
UNIT     := 8
D64      := $(BUILD)/$(PROG).d64

PRG      := $(BUILD)/$(PROG).prg
DBG      := $(BUILD)/$(PROG).dbg
LBL      := $(BUILD)/$(PROG).lbl
MAP      := $(BUILD)/$(PROG).map

# -t c64   : target Commodore 64
# -O       : ottimizzazioni del compilatore
# -g       : informazioni di debug nei file oggetto
# -Ln      : label file in formato VICE (caricabile nel monitor con `ll`)
# --dbgfile: file di debug completo (sorgenti/linee/simboli) per debugger esterni
# -m       : mappa del linker (layout memoria, utile per capire dove finisce il codice)
CFLAGS  := -t c64 -O -g
LDFLAGS := -Ln $(LBL) -Wl --dbgfile,$(DBG) -m $(MAP)

.PHONY: all run clean d64 d64-add disk disk-format disk-image

all: $(PRG)

$(PRG): $(SRCS) | $(BUILD)
	$(CL65) $(CFLAGS) $(LDFLAGS) -o $@ $(SRCS)

$(BUILD):
	mkdir -p $(BUILD)

# -moncommands carica le label nel monitor di VICE:
# apri il monitor (Alt+H / dal menu) e i simboli C (es. .main) sono gia' definiti.
run: $(PRG)
	$(EMU) -moncommands $(LBL) $(PRG)

# --- Immagini disco .d64 (host, c1541) -----------------------------------

# Immagine dedicata: formatta da zero e scrive il solo .prg.
d64: $(PRG)
	$(C1541) -format "$(DISKNAME)" d64 $(D64) -write $(PRG) $(CBMNAME)
	@echo "creata $(D64)"

# Aggiunge il .prg a un'immagine esistente senza riformattarla.
# Cancella prima un'eventuale copia omonima cosi' il target e' ripetibile.
# Uso: make d64-add D64=percorso/immagine.d64
d64-add: $(PRG)
	$(C1541) -attach $(D64) -delete $(CBMNAME) -write $(PRG) $(CBMNAME)
	@echo "aggiunto $(CBMNAME) a $(D64)"

# --- Drive reale via ZoomFloppy + OpenCBM --------------------------------

# Guardia: i target disk* richiedono cbmcopy/cbmforng/d64copy sul PATH.
HAVE_OPENCBM = command -v cbmcopy >/dev/null 2>&1 || \
	{ echo "OpenCBM non trovato: brew install opencbm (e collega lo ZoomFloppy)"; exit 1; }

# Scrive il .prg sul floppy nel drive UNIT, senza formattare (lo affianca agli altri file).
disk: $(PRG)
	@$(HAVE_OPENCBM)
	cbmcopy --transfer=auto -w $(UNIT) -o $(CBMNAME) $(PRG)

# Formatta il floppy (cbmforng) e poi scrive il .prg: disco dedicato.
disk-format: $(PRG)
	@$(HAVE_OPENCBM)
	cbmforng $(UNIT) "$(DISKNAME)"
	cbmcopy --transfer=auto -w $(UNIT) -o $(CBMNAME) $(PRG)

# Copia un'intera immagine .d64 sul floppy (sovrascrive il disco).
# Uso: make disk-image D64=build/pomodoro64.d64
disk-image:
	@command -v d64copy >/dev/null 2>&1 || \
		{ echo "OpenCBM non trovato: brew install opencbm"; exit 1; }
	d64copy $(D64) $(UNIT)

clean:
	rm -rf $(BUILD) src/*.o
