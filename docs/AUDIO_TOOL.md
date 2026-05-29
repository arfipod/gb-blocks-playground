# Audio Tool

`tools/gbdk_audio_piano_recorder.html` is a standalone browser tool for sketching short Game Boy melodies, recording them from a PC keyboard piano, editing the result on a step grid, and exporting C data compatible with the current lightweight ROM audio engine.

It is meant for compact themes and sound experiments. It is not a full tracker and it does not replace hUGEDriver for richer music.

## Opening The Tool

Open the HTML file directly in a browser:

```sh
xdg-open tools/gbdk_audio_piano_recorder.html
```

Opening it from your file manager or browser with `Ctrl+O` also works. The tool has no external dependencies.

## Keyboard Piano

The PC keyboard maps to a small piano range:

- White keys: `A` `S` `D` `F` `G` `H` `J` `K` `L`
- Black keys: `W` `E` `T` `Y` `U` `O` `P`
- Octave down/up: `Z` / `X`
- Record: `R`
- Play/stop: `Space`

The on-screen piano can also be clicked or tapped. The octave control changes the visible keyboard range and the notes recorded from PC keys.

## Timing Model

The tool shows musical duration in pulses, while the exported ROM data still uses steps.

- `BPM` sets quarter-note tempo.
- `Pulses` sets the loop length in quarter-note pulses.
- `Step division` chooses how many grid steps fit in one whole note: `1/4`, `1/8`, or `1/16`.
- `TOTAL_STEPS` is derived from pulses and division.
- `STEP_FRAMES` is the number of 60 Hz game frames per grid step.

At 120 BPM with `1/8` division, one quarter note is 30 frames and one step is 15 frames. A 16-pulse loop therefore becomes 32 steps.

Because the ROM advances on whole video frames, `STEP_FRAMES` is rounded to an integer. The browser recorder and metronome use that same effective grid duration so exported loops match what the current ROM engine will play.

Old project JSON files that only store `steps` are still accepted. The tool infers a reasonable pulse count from the saved step count and division.

## Recording

Press `Record` to start a finite recording pass. Recording stops automatically after the configured pulse duration. Pressing `Stop` or `Record` again stops early.

If a keyboard note is still held when recording stops, the tool closes it by writing the matching note-off event. Raw events remain visible in the Recorded events panel and can still be quantized or converted to the grid.

The metronome is WebAudio-only. When `Metronome while recording` is checked, it plays short square-wave clicks during recording and accents every fourth pulse. It is never exported to C and does not become part of the ROM data.

## Recording To Grid

Use `Recording -> Grid` after recording. The conversion:

- Quantizes note starts to grid steps.
- Clamps starts to the loop range.
- Clamps lengths so notes cannot extend past `TOTAL_STEPS`.
- Keeps a minimum note length of 1 step.
- Preserves recorded velocity as Game Boy volume.

The grid can also be edited directly. `Transpose +12` and `Transpose -12` shift all grid notes by one octave while clamping them to the supported note range.

## Exporting GBDK C

Use `Export GBDK C` to generate a header with the same note structure used by the ROM:

```c
typedef struct GBSongNote {
    uint8_t step;
    uint8_t midi;
    uint16_t gb_freq;
    uint8_t length_steps;
    uint8_t volume;
} GBSongNote;
```

The export keeps these macros:

- `STEP_FRAMES`
- `TOTAL_STEPS`
- `NOTE_COUNT`
- `CHANNEL`
- `DUTY`

To replace the current theme, export with asset name `eyenaut_theme`, then replace `include/eyenaut_theme.h` with the generated header. Build the ROM afterward:

```sh
make clean all
```

The current game calls `audio_update()` once per frame. That function advances the theme by `EYENAUT_THEME_STEP_FRAMES` and triggers notes from `eyenaut_theme_notes`.

## Monophonic CH1 And CH2

The current ROM music path is effectively monophonic per square channel. `src/audio.c` plays the theme on CH2 by writing CH2 registers. If multiple notes share the same step, the update loop writes them one after another in the same frame, so the last write wins.

That means same-step note clusters may sound like chords in the WebAudio preview, but they do not become real chords in the ROM. For CH1 and CH2 exports, treat the grid as one note per step. When the tool detects same-step clusters on CH1 or CH2, it warns in the UI and export area.

CH4 is different: it is noise percussion, not pitched square-wave harmony. The square-channel monophonic warning does not apply to CH4 in the same way.

## Melody And Duty Guidance

For clear Game Boy lead lines on a square channel, start around C5-C6, roughly MIDI 72-84. Lower octaves can work for bass, but they are better saved for a separate channel if the music engine grows later.

Duty cycle affects brightness:

- `12.5%` and `25%` tend to work well for brighter leads.
- `50%` gives a thicker tone.
- `75%` can be useful, but may feel less lead-like depending on the phrase.

Keep volume moderate, around 8-10, before deciding that a patch is too quiet or too harsh.

## Testing Audio

Use an emulator path that actually plays sound:

- Emulicious, when Java and GUI support are working.
- BGB on Windows.
- `make run-pyboy-audio` for an audio-enabled PyBoy path.

Avoid judging music through any target or emulator option that disables sound. The WSL-safe PyBoy targets may keep SDL stability settings such as `SDL_AUDIODRIVER=dummy`; those are useful for display smoke tests, but not for evaluating soundtrack or SFX audio.
