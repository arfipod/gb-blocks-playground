# Docker Development Environment

This image contains the Game Boy development tools needed by the project:

- GBDK-2020 4.5.0.
- RGBDS v1.0.1.
- Python 3 with PyBoy for emulator smoke tests.
- Java, SDL2 runtime libraries, build tools, `make`, `git`, `ripgrep`, and asset conversion dependencies.

The image is intended to be a portable build environment. It does not copy the repository into the image; mount the working tree so generated files stay on the host.

The pinned binary toolchain image currently targets `linux/amd64`, which matches typical Windows/WSL and Linux x86_64 machines.

## Build The Image

From WSL or Linux:

```sh
docker build -t gb-eyenaut-adventures-dev .
```

From Windows PowerShell:

```powershell
docker build -t gb-eyenaut-adventures-dev .
```

## Build The ROM

From WSL or Linux:

```sh
docker run --rm -it \
  -v "$PWD:/workspace/gb-eyenaut-adventures" \
  -w /workspace/gb-eyenaut-adventures \
  gb-eyenaut-adventures-dev \
  make clean all
```

From Windows PowerShell:

```powershell
docker run --rm -it `
  -v "${PWD}:/workspace/gb-eyenaut-adventures" `
  -w /workspace/gb-eyenaut-adventures `
  gb-eyenaut-adventures-dev `
  make clean all
```

The ROM is produced on the host at:

```text
build/eyenaut-adventures.gb
```

## Interactive Shell

From WSL or Linux:

```sh
docker run --rm -it \
  -v "$PWD:/workspace/gb-eyenaut-adventures" \
  -w /workspace/gb-eyenaut-adventures \
  gb-eyenaut-adventures-dev
```

Inside the container:

```sh
make doctor
make
make run
make run-pyboy
```

`make run` falls back to PyBoy in the container. Emulator windows may need GUI forwarding. For a no-GUI build workflow, use `make` in Docker and open the generated ROM with a native emulator on Windows or Linux.

## Make Shortcuts

The repository also includes convenience targets:

```sh
make docker-build
make docker-doctor
make docker-make
make docker-shell
```

These targets assume the local Docker CLI can run Linux containers.
