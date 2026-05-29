# Setup

This project targets original Game Boy / DMG homebrew from Ubuntu inside WSL.

## Environment Detected

- Ubuntu 24.04.1 LTS `noble`.
- Shell: `/bin/bash`.
- Architecture: `x86_64`.
- WSL: WSL2 detected.
- WSLg: `DISPLAY` and `WAYLAND_DISPLAY` were present.

## Recommended Base Packages

Run these once from WSL:

```sh
sudo apt update
sudo apt install -y build-essential git make cmake pkg-config bison flex libpng-dev unzip wget curl python3 python3-venv python3-pip default-jre xdg-utils ca-certificates
```

GBDK-2020, RGBDS, Emulicious, hUGEDriver, PyBoy, and a temporary user-local `make` fallback were installed under your home directory. Java is currently available through OpenJDK 21, so Emulicious can start from WSL.

## Installed User Tools

```sh
~/opt/gbdk        -> GBDK-2020 4.5.0
~/opt/rgbds       -> RGBDS v1.0.1 official Linux archive
~/opt/emulicious  -> Emulicious Java distribution
~/gbdev-tools     -> optional homebrew helper tools
~/.local/bin      -> local launchers and fallback make
```

`~/.bashrc` contains this managed block:

```sh
# >>> gbdev toolchain >>>
export GBDK_HOME="$HOME/opt/gbdk"
export RGBDS_HOME="$HOME/opt/rgbds"
export GBDEV_TOOLS="$HOME/gbdev-tools"
export EMULICIOUS_HOME="$HOME/opt/emulicious"
export PATH="$GBDK_HOME/bin:$RGBDS_HOME:$HOME/.local/bin:$PATH"
# <<< gbdev toolchain <<<
```

Open a new shell, then verify:

```sh
lcc -v
png2asset --help
gbcompress --help
rgbasm --version
rgblink --version
rgbfix --version
rgbgfx --version
make --version
java -version
```

The Makefile also falls back to `~/opt/gbdk` and `~/opt/rgbds`, so builds still work if an already-open terminal has not loaded the new shell environment yet.

## Smoke Test

An official GBDK minimal example was compiled directly with `lcc`:

```sh
~/gbdev-sandbox/gbdk-smoke-test/gbdk-smoke-test.gb
```

## WSL Notes

WSLg appears available when these variables are set:

```sh
echo $DISPLAY
echo $WAYLAND_DISPLAY
```

Expected values are similar to `:0` and `wayland-0`. Java is also required for Emulicious:

```sh
java -version
```

The quick health check is:

```sh
cd ~/git/gb-eyenaut-adventures
make doctor
```

For a WSLg-specific display check, run:

```sh
make doctor-wslg
```

WSLg can sometimes create Linux GUI windows that appear in Windows Task Manager but do not become visible. This has been observed with Emulicious/Java and can also happen with SDL depending on the Windows graphics driver. If that happens:

1. Build from WSL:

   ```sh
   cd ~/git/gb-eyenaut-adventures
   make
   ```

2. Try the WSL emulator targets:

   ```sh
   make run-wslg
   make run
   make run-pyboy
   make run-pyboy-x11
   ```

   `make run-wslg` checks WSLg and then uses the SDL/PyBoy path. `make run-pyboy` forces software rendering and disables sound to avoid common WSLg Mesa/ALSA failures.

3. If a window still does not appear, close stuck emulator processes:

   ```sh
   pkill -f Emulicious.jar || true
   pkill -f pyboy || true
   ```

4. From Windows, update WSL and restart it:

   ```powershell
   wsl --update
   wsl --shutdown
   ```

   Then reopen Ubuntu and try again.

If GUI launch still fails, use one of these workflows:

- Run `make run-windows` from WSL to open the ROM through the Windows `.gb` file association.
- Run `emulicious build/eyenaut-adventures.gb` from WSL.
- Run Emulicious on Windows and open `\\wsl$\Ubuntu\home\arfipod\git\gb-eyenaut-adventures\build\eyenaut-adventures.gb`.
- Use BGB on Windows for strong Game Boy debugging.
- Keep WSL as a headless build-only environment.

If `make run-windows` says WSL cannot execute Windows programs, restart WSL from PowerShell:

```powershell
wsl --shutdown
```

Then reopen Ubuntu. If it still fails, enable interop in `/etc/wsl.conf` and restart WSL again:

```ini
[interop]
enabled=true
appendWindowsPath=true
```

The most reliable Windows workflow is: compile in WSL with `make`, then open the generated ROM from Windows using a Windows-native emulator. The ROM path from Windows is:

```text
\\wsl$\Ubuntu\home\arfipod\git\gb-eyenaut-adventures\build\eyenaut-adventures.gb
```

## Asset Tools

GBDK's `png2asset` is available for C-friendly tile and sprite conversion. GUI tools are usually smoother on Windows:

- Aseprite for pixel art.
- Tiled for map authoring.
- hUGETracker for Game Boy music.

Generated or exported assets can live in `assets/` and be converted by scripts in `tools/`.
