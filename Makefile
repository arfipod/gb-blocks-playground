PROJECT := eyenaut-adventures
ROM := build/$(PROJECT).gb
SOURCES := $(wildcard src/*.c)
INCLUDES := -Iinclude

GBDK_HOME ?= $(HOME)/opt/gbdk
RGBDS_HOME ?= $(HOME)/opt/rgbds

ifneq ($(wildcard $(GBDK_HOME)/bin/lcc),)
LCC ?= $(GBDK_HOME)/bin/lcc
PNG2ASSET ?= $(GBDK_HOME)/bin/png2asset
else
LCC ?= lcc
PNG2ASSET ?= png2asset
endif

ifneq ($(wildcard $(RGBDS_HOME)/rgbasm),)
RGBASM ?= $(RGBDS_HOME)/rgbasm
RGBLINK ?= $(RGBDS_HOME)/rgblink
RGBFIX ?= $(RGBDS_HOME)/rgbfix
RGBGFX ?= $(RGBDS_HOME)/rgbgfx
else
RGBASM ?= rgbasm
RGBLINK ?= rgblink
RGBFIX ?= rgbfix
RGBGFX ?= rgbgfx
endif

ifneq ($(wildcard $(HOME)/.local/bin/emulicious),)
EMULATOR ?= $(HOME)/.local/bin/emulicious
EMULATOR_FLAGS ?= -scale 4
else ifneq ($(shell command -v pyboy 2>/dev/null),)
EMULATOR ?= pyboy
EMULATOR_FLAGS ?= --window SDL2 --scale 4 --dmg --no-sound-emulation
else
EMULATOR ?= emulicious
EMULATOR_FLAGS ?= -scale 4
endif

ifneq ($(wildcard $(HOME)/gbdev-sandbox/pyboy-venv/bin/pyboy),)
PYBOY ?= $(HOME)/gbdev-sandbox/pyboy-venv/bin/pyboy
else
PYBOY ?= pyboy
endif

DOCKER_IMAGE ?= gb-eyenaut-adventures-dev
DOCKER_WORKDIR ?= /workspace/gb-eyenaut-adventures
DOCKER_RUN ?= docker run --rm -it -v "$(CURDIR):$(DOCKER_WORKDIR)" -w "$(DOCKER_WORKDIR)" $(DOCKER_IMAGE)

LCCFLAGS := $(INCLUDES) -Wm-yn"EYENAUT ADV"
PYBOY_ENV ?= SDL_AUDIODRIVER=dummy LIBGL_ALWAYS_SOFTWARE=1 MESA_LOADER_DRIVER_OVERRIDE=llvmpipe

.PHONY: all clean run run-pyboy run-pyboy-x11 run-wslg run-windows debug wslg-check doctor doctor-wslg tools-check docker-build docker-shell docker-make docker-doctor

all: $(ROM)

$(ROM): $(SOURCES) $(wildcard include/*.h) | build
	$(LCC) $(LCCFLAGS) -o $@ $(SOURCES)

build:
	mkdir -p build

clean:
	rm -f build/*.gb build/*.ihx build/*.noi build/*.map build/*.sym build/*.cdb build/*.adb
	rm -f src/*.asm src/*.lst src/*.o src/*.rel src/*.sym src/*.noi src/*.rst

run: $(ROM)
	@if ! command -v $(EMULATOR) >/dev/null 2>&1; then \
		echo "No emulator command found. Set EMULATOR=/path/to/emulator or install the emulicious launcher."; \
		exit 1; \
	fi
	@if [ "$$(basename "$(EMULATOR)")" = "emulicious" ] && ! command -v java >/dev/null 2>&1; then \
		echo "Emulicious is installed, but Java is missing. Install it with: sudo apt install -y default-jre"; \
		exit 1; \
	fi
	@if [ "$$(basename "$(EMULATOR)")" = "pyboy" ]; then \
		$(PYBOY_ENV) $(EMULATOR) $(EMULATOR_FLAGS) $(ROM); \
	else \
		$(EMULATOR) $(EMULATOR_FLAGS) $(ROM); \
	fi

run-pyboy: $(ROM)
	@if [ ! -x "$(PYBOY)" ] && ! command -v $(PYBOY) >/dev/null 2>&1; then \
		echo "PyBoy was not found at $(PYBOY)."; \
		echo "Create it with: python3 -m venv ~/gbdev-sandbox/pyboy-venv && ~/gbdev-sandbox/pyboy-venv/bin/python -m pip install pyboy"; \
		exit 1; \
	fi
	$(PYBOY_ENV) $(PYBOY) --window SDL2 --scale 4 --dmg --no-sound-emulation $(ROM)

run-pyboy-x11: $(ROM)
	@if [ ! -x "$(PYBOY)" ] && ! command -v $(PYBOY) >/dev/null 2>&1; then \
		echo "PyBoy was not found at $(PYBOY)."; \
		echo "Create it with: python3 -m venv ~/gbdev-sandbox/pyboy-venv && ~/gbdev-sandbox/pyboy-venv/bin/python -m pip install pyboy"; \
		exit 1; \
	fi
	SDL_VIDEODRIVER=x11 $(PYBOY_ENV) $(PYBOY) --window SDL2 --scale 4 --dmg --no-sound-emulation $(ROM)

run-wslg: wslg-check run-pyboy

run-windows: $(ROM)
	@if ! command -v cmd.exe >/dev/null 2>&1; then \
		echo "cmd.exe was not found. Run this target from WSL with Windows interop enabled."; \
		exit 1; \
	fi
	@if ! cmd.exe /C ver >/dev/null 2>&1; then \
		echo "cmd.exe was found, but WSL cannot execute Windows programs in this session."; \
		echo "From PowerShell, run: wsl --shutdown"; \
		echo "Then reopen Ubuntu and try: make run-windows"; \
		exit 1; \
	fi
	@ROM_WIN="$$(wslpath -aw "$(ROM)")"; \
	printf 'Opening %s with the Windows default ROM handler...\n' "$$ROM_WIN"; \
	( cd /mnt/c/Windows && cmd.exe /C start "" "$$ROM_WIN" )

debug: run

wslg-check:
	@echo "WSL distro:      $${WSL_DISTRO_NAME:-not WSL}"
	@echo "DISPLAY:         $${DISPLAY:-missing}"
	@echo "WAYLAND_DISPLAY: $${WAYLAND_DISPLAY:-missing}"
	@if [ -z "$${WSL_DISTRO_NAME:-}" ]; then \
		echo "This target is intended to run inside WSL."; \
		exit 1; \
	fi
	@if [ -z "$${DISPLAY:-}" ] && [ -z "$${WAYLAND_DISPLAY:-}" ]; then \
		echo "WSLg display variables are missing. From PowerShell, run: wsl --update; wsl --shutdown"; \
		exit 1; \
	fi
	@if [ -n "$${WAYLAND_DISPLAY:-}" ] && [ -S "/mnt/wslg/runtime-dir/$${WAYLAND_DISPLAY}" ]; then \
		echo "WSLg Wayland socket: ok"; \
	elif [ -S /tmp/.X11-unix/X0 ]; then \
		echo "WSLg X11 socket: ok"; \
	else \
		echo "WSLg display variables exist, but no display socket was found."; \
		exit 1; \
	fi

doctor: tools-check

doctor-wslg: tools-check wslg-check

docker-build:
	docker build -t $(DOCKER_IMAGE) .

docker-shell:
	$(DOCKER_RUN)

docker-make:
	$(DOCKER_RUN) make clean all

docker-doctor:
	$(DOCKER_RUN) make doctor

tools-check:
	@echo "GBDK lcc:      $$(command -v $(LCC) || true)"
	@$(LCC) -v 2>&1 | head -n 1
	@echo "png2asset:     $$(command -v $(PNG2ASSET) || true)"
	@$(PNG2ASSET) --help >/dev/null 2>&1 && echo "png2asset:     ok" || echo "png2asset:     present, help check skipped"
	@echo "RGBDS rgbasm:  $$(command -v $(RGBASM) || true)"
	@$(RGBASM) --version
	@echo "RGBDS rgblink: $$(command -v $(RGBLINK) || true)"
	@$(RGBLINK) --version
	@echo "RGBDS rgbfix:  $$(command -v $(RGBFIX) || true)"
	@$(RGBFIX) --version
	@echo "RGBDS rgbgfx:  $$(command -v $(RGBGFX) || true)"
	@$(RGBGFX) --version
	@echo "Emulator:      $$(command -v $(EMULATOR) || true)"
	@echo "PyBoy:         $(PYBOY)"
	@if command -v java >/dev/null 2>&1; then java -version 2>&1 | head -n 1; else echo "Java:          missing"; fi
