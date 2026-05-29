# Architecture

## Tile World

The prototype uses a 128 by 32 logical world. Each tile is one byte: empty, dirt, grass, stone, wood, leaves, planks, workbench, torch, coal, copper, iron, platform, or door. The full logical world is not materialized in WRAM; `world.c` keeps a 5-chunk active window and reloads distant columns from the fixed-seed procedural generator plus a small changed-tile log.

## Chunks

The logical world is divided into 16 chunks of 8 columns. `chunk.c` procedurally generates surface height, caves, trees, ore pockets, and basic underground layers per chunk from `WORLD_DEFAULT_SEED`. `chunk_tick()` centers the active WRAM window on the chunk under the camera center, keeping that chunk plus two chunks on either side when the world edges allow it. Unloaded chunks are reconstructed from the generator and patched with remembered mining/placement changes. The current changed-tile log is the WRAM version of the eventual battery SRAM save payload.

## Biomes

Biomes are currently procedural/generative only, with no extra tile IDs. `world_biome_at_x()` divides the 128-column world into four 32-column regions: Meadow, Rootwood Grove, Stone Belt, and Celestial Ruins. `chunk_generate_tile()` uses the biome to vary surface height, tree frequency, dirt depth, cave density, surface material, and ore frequency.

## Collision

Collision is tile-based. The player sprite is 8 by 16 pixels, but movement uses a 6 by 8 pixel hitbox on the lower half of the sprite so decorative head pixels do not block tight gaps. Movement tests the box corners against solid world tiles before applying horizontal or vertical movement.

Platforms are handled as a player-specific collision case instead of a global solid tile rule. They do not block horizontal movement or upward movement. During downward movement they block only when the lower hitbox points cross the platform's top edge from above, and holding Down disables that platform collision so the player can drop through.

## Placement

`world_can_place_tile()` owns block placement rules. Normal blocks require an empty target with at least one solid neighbor. Torches require empty space plus solid support on the left, right, or below, and other torches do not count as support. Workbenches remain one tile wide for now and require an empty target with solid floor underneath. Player overlap is checked from `player.c` so world placement helpers stay independent of actors.

Doors are 1x2 multi-tile blocks. The placement target is the bottom tile: the target and tile above must be empty, with solid floor below. Closed doors use top/bottom tiles and are solid; open doors use top/bottom open tiles and are non-solid. Pressing B while aiming at any door tile toggles both halves, and mining either half removes the whole door.

## Health

The player keeps a small HP counter, an invulnerability timer, and accumulated fall distance. Enemy contact and long falls call `player_damage()`. At zero HP the player respawns at the fixed start position with inventory intact and a longer invulnerability window.

## Enemies

`enemy.c` keeps up to three active surface enemies. They spawn near the camera, walk left or right, reverse at walls or ledges, despawn outside a camera margin, and damage the player on contact.

## Crafting

Crafting recipes are fixed data in `inventory.c`. A recipe can output either an inventory item or a tool-level upgrade. `inventory_craft_status()` separates missing materials, missing workbench, full output stack, and already-owned tool upgrades so the menu can show locks only for workbench-gated recipes and blink missing inputs after a failed craft.

## Tools

Tool progression is a single byte on `Inventory`. There is no equipment UI or durability yet; the best crafted pickaxe level is always active. `inventory_tile_required_tool_level()` defines the mining gate per tile: hand breaks soft blocks, while stone and ores require the basic stone pickaxe. Failed mining attempts play the error SFX and blink the target cursor without changing the world.

## Mining

`mining.c` owns held-button mining state. Holding A on the same target increments progress until the tile's mining time is reached, then the block is removed and converted into a pickup drop. Moving the target, releasing A, hitting an empty tile, lacking the required tool level, or having no free drop slot resets mining. The target cursor uses extra sprite patterns to show early and late progress instead of drawing crack overlays into the background.

## Item Drops

`item_drop.c` keeps a small fixed pool of pickup entities. Drops use one 8x16 sprite each with the lower tile transparent, fall vertically until they touch solid ground, and try to auto-pick up when overlapping the player. If the inventory is full, the drop stays in the world until picked up later or its lifetime expires. The drop pool is deliberately small so mining cannot flood OAM or WRAM.

## Render Loop

The frame is split into logic and render commit phases. Game logic updates input, player physics, camera, chunk bookkeeping, and dirty flags before waiting for VBlank. After `wait_vbl_done()`, the render commit applies only the VRAM work that is needed: newly visible background columns, queued dirty world tiles, changed HUD/menu tiles, and sprite/background scroll positions. Because the world is currently 32 tiles tall, each streamed column fills the full BG map height and vertical camera movement is handled by hardware scroll.

## Lighting

Lighting is a DMG-friendly visual approximation. The renderer generates dark and very-dark tile variants from the normal world tiles by adding denser pixel patterns. Torch distance chooses the visual variant for each world tile; world data, collision, and saves still store the normal tile IDs. Lighting is invalidated when a block changes or the active chunk window reloads, then visible columns are redrawn.

## Audio

`audio.c` provides minimal register-based SFX for jump, mine, place, craft, and failed craft, plus the lightweight `eyenaut_theme` music loop generated by `tools/gbdk_audio_piano_recorder.html`. The theme runs on CH2 and is advanced once per frame from the game loop; most SFX use CH1 or CH4 so the melody keeps playing. CH2 playback is monophonic in this engine: if multiple notes share a step, the last register write wins. The recorder documents and warns about that constraint; see [AUDIO_TOOL.md](AUDIO_TOOL.md). This layer is intentionally small so hUGEDriver can take over richer music later without unwinding a large custom driver.

## Camera

The camera follows the player horizontally and vertically, clamping to the active world bounds. Rendering treats the 32 by 32 background map as a ring buffer horizontally and uses hardware scroll for smooth pixel movement on both axes.

## Hardware Constraints

The design avoids large dynamic structures, high sprite counts, and per-pixel effects. The original Game Boy is strongest when most gameplay is expressed as tile changes, simple sprites, and predictable update work per frame.
