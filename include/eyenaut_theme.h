#ifndef EYENAUT_THEME_H
#define EYENAUT_THEME_H

#include <stdint.h>

#define EYENAUT_THEME_BPM 120u
#define EYENAUT_THEME_STEP_FRAMES 15u
#define EYENAUT_THEME_TOTAL_STEPS 64u
#define EYENAUT_THEME_NOTE_COUNT 51u
#define EYENAUT_THEME_CHANNEL 2u
#define EYENAUT_THEME_DUTY 1u

typedef struct GBSongNote {
    uint8_t step;
    uint8_t midi;
    uint16_t gb_freq;
    uint8_t length_steps;
    uint8_t volume;
} GBSongNote;

static const GBSongNote eyenaut_theme_notes[EYENAUT_THEME_NOTE_COUNT] = {
    { 0u, 76u, 1849u, 2u, 9u },
    { 2u, 76u, 1849u, 1u, 9u },
    { 3u, 79u, 1881u, 1u, 9u },
    { 4u, 77u, 1860u, 1u, 9u },
    { 5u, 76u, 1849u, 1u, 9u },
    { 6u, 74u, 1825u, 1u, 9u },
    { 7u, 72u, 1798u, 1u, 9u },
    { 8u, 74u, 1825u, 1u, 9u },
    { 9u, 77u, 1860u, 1u, 9u },
    { 10u, 79u, 1881u, 2u, 9u },
    { 12u, 79u, 1881u, 1u, 9u },
    { 13u, 81u, 1899u, 1u, 9u },
    { 14u, 79u, 1881u, 2u, 9u },
    { 16u, 76u, 1849u, 2u, 9u },
    { 18u, 79u, 1881u, 1u, 9u },
    { 19u, 81u, 1899u, 1u, 9u },
    { 20u, 83u, 1915u, 1u, 9u },
    { 21u, 81u, 1899u, 1u, 9u },
    { 22u, 79u, 1881u, 2u, 9u },
    { 24u, 76u, 1849u, 1u, 9u },
    { 25u, 74u, 1825u, 1u, 9u },
    { 26u, 72u, 1798u, 2u, 9u },
    { 28u, 76u, 1849u, 1u, 9u },
    { 29u, 79u, 1881u, 1u, 9u },
    { 30u, 76u, 1849u, 2u, 9u },
    { 32u, 84u, 1923u, 2u, 9u },
    { 34u, 83u, 1915u, 1u, 9u },
    { 35u, 81u, 1899u, 1u, 9u },
    { 36u, 79u, 1881u, 1u, 9u },
    { 37u, 76u, 1849u, 1u, 9u },
    { 38u, 74u, 1825u, 2u, 9u },
    { 40u, 76u, 1849u, 1u, 9u },
    { 41u, 79u, 1881u, 1u, 9u },
    { 42u, 81u, 1899u, 2u, 9u },
    { 44u, 79u, 1881u, 1u, 9u },
    { 45u, 76u, 1849u, 1u, 9u },
    { 46u, 74u, 1825u, 2u, 9u },
    { 48u, 72u, 1798u, 1u, 9u },
    { 49u, 76u, 1849u, 1u, 9u },
    { 50u, 79u, 1881u, 1u, 9u },
    { 51u, 84u, 1923u, 1u, 9u },
    { 52u, 83u, 1915u, 1u, 9u },
    { 53u, 81u, 1899u, 1u, 9u },
    { 54u, 79u, 1881u, 2u, 9u },
    { 56u, 76u, 1849u, 1u, 9u },
    { 57u, 79u, 1881u, 1u, 9u },
    { 58u, 81u, 1899u, 1u, 9u },
    { 59u, 79u, 1881u, 1u, 9u },
    { 60u, 76u, 1849u, 1u, 9u },
    { 61u, 74u, 1825u, 1u, 9u },
    { 62u, 72u, 1798u, 2u, 9u }
};

#endif /* EYENAUT_THEME_H */
