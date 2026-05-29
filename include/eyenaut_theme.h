#ifndef EYENAUT_THEME_H
#define EYENAUT_THEME_H

#include <stdint.h>

#define EYENAUT_THEME_BPM 120u
#define EYENAUT_THEME_STEP_FRAMES 15u
#define EYENAUT_THEME_TOTAL_STEPS 89u
#define EYENAUT_THEME_NOTE_COUNT 52u
#define EYENAUT_THEME_CHANNEL 2u
#define EYENAUT_THEME_DUTY 2u

typedef struct GBSongNote {
    uint8_t step;
    uint8_t midi;
    uint16_t gb_freq;
    uint8_t length_steps;
    uint8_t volume;
} GBSongNote;

static const GBSongNote eyenaut_theme_notes[EYENAUT_THEME_NOTE_COUNT] = {
    { 4u, 64u, 1650u, 2u, 10u },
    { 6u, 64u, 1650u, 1u, 10u },
    { 7u, 67u, 1714u, 1u, 10u },
    { 9u, 65u, 1673u, 1u, 10u },
    { 10u, 64u, 1650u, 1u, 10u },
    { 11u, 62u, 1602u, 1u, 10u },
    { 12u, 60u, 1547u, 1u, 10u },
    { 14u, 62u, 1602u, 1u, 10u },
    { 15u, 65u, 1673u, 1u, 10u },
    { 16u, 65u, 1673u, 1u, 10u },
    { 18u, 65u, 1673u, 1u, 10u },
    { 19u, 65u, 1673u, 3u, 10u },
    { 24u, 62u, 1602u, 1u, 10u },
    { 26u, 62u, 1602u, 1u, 10u },
    { 27u, 65u, 1673u, 1u, 10u },
    { 28u, 64u, 1650u, 1u, 10u },
    { 29u, 62u, 1602u, 1u, 10u },
    { 31u, 60u, 1547u, 1u, 10u },
    { 32u, 62u, 1602u, 1u, 10u },
    { 33u, 64u, 1650u, 1u, 10u },
    { 34u, 60u, 1547u, 1u, 10u },
    { 35u, 60u, 1547u, 1u, 10u },
    { 37u, 60u, 1547u, 1u, 10u },
    { 38u, 60u, 1547u, 3u, 10u },
    { 42u, 64u, 1650u, 1u, 10u },
    { 44u, 64u, 1650u, 1u, 10u },
    { 45u, 67u, 1714u, 1u, 10u },
    { 46u, 65u, 1673u, 1u, 10u },
    { 47u, 64u, 1650u, 1u, 10u },
    { 48u, 62u, 1602u, 1u, 10u },
    { 49u, 60u, 1547u, 1u, 10u },
    { 50u, 64u, 1650u, 1u, 10u },
    { 52u, 69u, 1750u, 1u, 10u },
    { 53u, 69u, 1750u, 1u, 10u },
    { 54u, 69u, 1750u, 1u, 10u },
    { 55u, 67u, 1714u, 3u, 10u },
    { 60u, 67u, 1714u, 1u, 10u },
    { 60u, 71u, 1783u, 1u, 10u },
    { 63u, 65u, 1673u, 1u, 10u },
    { 63u, 69u, 1750u, 1u, 10u },
    { 65u, 64u, 1650u, 1u, 10u },
    { 65u, 67u, 1714u, 1u, 10u },
    { 68u, 65u, 1673u, 1u, 10u },
    { 68u, 71u, 1783u, 1u, 10u },
    { 70u, 67u, 1714u, 4u, 10u },
    { 70u, 71u, 1783u, 4u, 10u },
    { 70u, 72u, 1798u, 1u, 10u },
    { 75u, 65u, 1673u, 4u, 10u },
    { 76u, 71u, 1783u, 4u, 10u },
    { 81u, 65u, 1673u, 5u, 10u },
    { 81u, 72u, 1798u, 14u, 10u },
    { 87u, 67u, 1714u, 8u, 10u }
};

#endif /* EYENAUT_THEME_H */
