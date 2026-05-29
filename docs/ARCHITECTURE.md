# Architecture

## Tile World

The prototype uses a 128 by 18 logical world. Each tile is one byte: empty, dirt, grass, stone, wood, leaves, planks, workbench, or torch. The full logical world is not materialized in WRAM; `world.c` keeps a 6-chunk active window and reloads distant columns from the procedural generator plus a small changed-tile log.

## Chunks

The logical world is divided into 16 chunks of 8 columns. `chunk.c` procedurally generates surface height, caves, trees, and basic underground layers per chunk. `chunk_tick()` now moves the active WRAM window as the camera crosses chunk boundaries; unloaded chunks are reconstructed from the generator and patched with remembered mining/placement changes. The current changed-tile log is a prototype substitute for real save-backed chunk storage.

## Collision

Collision is tile-based. The player sprite is 8 by 16 pixels, but movement uses a 6 by 8 pixel hitbox on the lower half of the sprite so decorative head pixels do not block tight gaps. Movement tests the box corners against solid world tiles before applying horizontal or vertical movement.

## Render Loop

The frame is split into logic and render commit phases. Game logic updates input, player physics, camera, chunk bookkeeping, and dirty flags before waiting for VBlank. After `wait_vbl_done()`, the render commit applies only the VRAM work that is needed: newly visible background columns, queued dirty world tiles, changed HUD/menu tiles, and sprite/background scroll positions.

## Camera

The camera follows the player horizontally and clamps to the active world width. Rendering treats the 32-column background map as a ring buffer and uses hardware scroll for smooth pixel movement.

## Hardware Constraints

The design avoids large dynamic structures, high sprite counts, and per-pixel effects. The original Game Boy is strongest when most gameplay is expressed as tile changes, simple sprites, and predictable update work per frame.
