# Terraria GB Lite

A deliberately small Game Boy / DMG homebrew prototype in C using GBDK-2020. The goal is to explore a Terraria-like destructible tile world while staying honest about original Game Boy limits.

## Target

- Nintendo Game Boy / DMG.
- C with GBDK-2020.
- Optional RGBDS for assembly experiments and later optimization.
- No GB Studio or large engine.

## Build

Open a new WSL shell after setup so `~/.bashrc` exports `GBDK_HOME` and updates `PATH`.

```sh
cd ~/git/gb-blocks-playground
make docto
make tools-check
make clean
make
```

The ROM is written to:

```text
build/terraria-gb-lite.gb
```

## Run

Recommended from WSL:

```sh
cd ~/git/gb-blocks-playground
make run-wslg
```

`run-wslg` checks WSLg and then starts the SDL/PyBoy path, which is the most reliable Linux GUI option on this machine.

```sh
make run
```

By default `make run` uses the `emulicious` launcher. On WSL, Java and WSLg must both be working for that Linux GUI path. BGB is a strong Windows-side debugger alternative: build in WSL, then open the ROM from Windows.

If Emulicious opens as an invisible WSLg window, try the SDL-based PyBoy fallback:

```sh
make run-pyboy
```

If WSLg still creates invisible windows, this target opens the ROM from WSL using Windows interop:

```sh
make run-windows
```

That target uses the Windows default app associated with `.gb` files. You can also build in WSL and open this ROM manually from a Windows-native emulator:

```text
\\wsl$\Ubuntu\home\arfipod\git\gb-blocks-playground\build\terraria-gb-lite.gb
```

See `docs/SETUP.md` for WSLg troubleshooting notes.

## Controls

- Left / Right: move.
- A: mine the block in front of the player.
- B: place a dirt block in front of the player.

## Roadmap

- Replace the tiny WRAM world with chunk streaming.
- Add inventory and block selection.
- Add caves and a better terrain generator.
- Add a simple lighting approximation.
- Add save data once the world format is stable.
- Integrate hUGEDriver after the core prototype loop feels good.
