# Toolchain

## GBDK-2020

GBDK-2020 is the primary SDK and compiler path. It provides `lcc`, Game Boy headers, libraries, examples, and helper tools. This project builds C directly into a `.gb` ROM with `lcc`.

## RGBDS

RGBDS is installed for assembly experiments, low-level inspection, and future optimization. It is not required for the first C prototype, but it is useful for custom routines and understanding generated code.

## png2asset

`png2asset` converts PNG artwork into GBDK-ready C data. The first prototype uses hand-written placeholder tile bytes; real art should move through `assets/` and conversion scripts.

## Emulicious and BGB

Emulicious is prepared under WSL as the default `make run` emulator, but it needs Java. It provides strong debugging tools when GUI support works.

BGB is a strong Windows-side Game Boy emulator/debugger. It is often the most convenient option with WSL: build in Linux, open the ROM from Windows.

PyBoy is useful as a lightweight WSL fallback when Java/WSLg windows do not appear correctly. This repository exposes it through `make run-wslg` and `make run-pyboy` if the sandbox venv exists at `~/gbdev-sandbox/pyboy-venv`.

Recommended emulator workflow on Windows:

- Use WSL for building and command-line tools.
- Run `make doctor` to verify GBDK, RGBDS, Java, and emulator launchers.
- Run `make doctor-wslg` when you specifically want to verify WSLg display support.
- Run `make run-wslg` first for the WSLg/PyBoy path.
- Run `make run-windows` from WSL if you want the ROM opened through the Windows `.gb` file association.
- Use a Windows-native emulator when WSLg windows are invisible or unreliable.
- Open the ROM from `\\wsl$\Ubuntu\home\arfipod\git\gb-eyenaut-adventures\build\eyenaut-adventures.gb`.
- Prefer BGB for Windows-side debugging and Emulicious for source-level/debugger experiments when its GUI works.
- If WSLg behaves strangely, run `wsl --update` and `wsl --shutdown` from PowerShell, then reopen Ubuntu.

## hUGEDriver and hUGETracker

hUGEDriver is cloned under `~/gbdev-tools/hUGEDriver`. It plays music exported by hUGETracker and can be used from GBDK or RGBDS workflows. Keep it optional until the core prototype loop is stable.
