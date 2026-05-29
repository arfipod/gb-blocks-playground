# Memory Budget

## DMG Facts

- Screen: 160 by 144 pixels.
- Background tiles: 8 by 8 pixels.
- Visible background: 20 by 18 tiles.
- Background map: 32 by 32 tile entries in VRAM.
- VRAM: 8 KiB.
- WRAM: 8 KiB.
- OAM: 40 sprites, with a practical limit of 10 sprites per scanline.

## What This Means

A destructible tile adventure cannot keep a large world in RAM. A simple byte-per-tile world uses:

```text
32 x 18 = 576 bytes
64 x 32 = 2048 bytes
128 x 18 = 2304 bytes
128 x 64 = 8192 bytes
```

The last example would consume essentially all WRAM before player state, stacks, buffers, inventory, enemies, or save helpers.

## Current Prototype

The current logical world is wider than one background map:

```text
WORLD_CHUNK_COUNT x CHUNK_WIDTH_TILES = 16 x 8 = 128 columns
WORLD_HEIGHT_TILES = 18 rows
full logical world, if materialized = 128 x 18 = 2304 bytes
```

The prototype now streams a smaller active chunk window in WRAM:

```text
active world tiles = 6 chunks x 8 columns x 18 rows = 864 bytes
changed tile log   = 128 entries x about 4 bytes = about 512 bytes
dirty tile queue   = 32 entries x about 3 bytes = about 96 bytes
```

Rendering also keeps CPU-side tile buffers. The background renderer updates one column at a time:

```text
bg_buffer  = 18 bytes
win_buffer = 20 x 18 = 360 bytes
```

Those main arrays are roughly:

```text
864 + 512 + 96 + 18 + 360 = 1850 bytes
```

That is about 23% of DMG WRAM before stack, player state, camera state, inventory, library globals, enemies, dropped items, or a real save system.

## First Approximation

For early development:

- Keep only the active world area or changed chunk data in WRAM.
- Store block IDs as bytes first, then pack later if needed.
- Keep changed tile coordinates in a small queue or save-backed chunk delta table.
- Use ROM banks for generated or authored terrain templates.
- Avoid large recursive generators or dynamic allocation.
