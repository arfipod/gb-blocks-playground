#include <gb/gb.h>
#include "audio.h"
#include "eyenaut_theme.h"

static uint8_t music_playing;
static uint8_t music_frame_counter;
static uint8_t music_step;
static uint8_t music_note_cursor;
static uint8_t music_gate_counter;

void audio_init(void)
{
    NR52_REG = AUDENA_ON;
    NR50_REG = AUDVOL_VOL_LEFT(7u) | AUDVOL_VOL_RIGHT(7u);
    NR51_REG = AUDTERM_1_LEFT | AUDTERM_1_RIGHT |
               AUDTERM_2_LEFT | AUDTERM_2_RIGHT |
               AUDTERM_4_LEFT | AUDTERM_4_RIGHT;
}

void audio_start_music(void)
{
    music_playing = 1u;
    music_frame_counter = 0u;
    music_step = 0u;
    music_note_cursor = 0u;
    music_gate_counter = 0u;
}

void audio_stop_music(void)
{
    music_playing = 0u;
    NR22_REG = 0x00u;
}

static void play_square_ch1(uint8_t sweep, uint8_t length, uint8_t envelope,
                            uint8_t freq_lo, uint8_t freq_hi)
{
    NR10_REG = sweep;
    NR11_REG = length;
    NR12_REG = envelope;
    NR13_REG = freq_lo;
    NR14_REG = (uint8_t)(0x80u | freq_hi);
}

static void play_square_ch2(uint8_t length, uint8_t envelope,
                            uint8_t freq_lo, uint8_t freq_hi)
{
    NR21_REG = length;
    NR22_REG = envelope;
    NR23_REG = freq_lo;
    NR24_REG = (uint8_t)(0x80u | freq_hi);
}

static void play_music_note(const GBSongNote *note)
{
    uint8_t duty_bits = (uint8_t)((EYENAUT_THEME_DUTY & 0x03u) << 6);
    uint8_t envelope = (uint8_t)((note->volume & 0x0Fu) << 4);

    NR21_REG = duty_bits;
    NR22_REG = envelope;
    NR23_REG = (uint8_t)(note->gb_freq & 0xFFu);
    NR24_REG = (uint8_t)(0x80u | ((note->gb_freq >> 8) & 0x07u));
    music_gate_counter = (uint8_t)(note->length_steps * EYENAUT_THEME_STEP_FRAMES);
}

static void play_noise(uint8_t length, uint8_t envelope, uint8_t poly)
{
    NR41_REG = length;
    NR42_REG = envelope;
    NR43_REG = poly;
    NR44_REG = 0x80u;
}

void audio_play_jump(void)
{
    play_square_ch1(0x16u, 0x80u, 0xF2u, 0xC0u, 0x06u);
}

void audio_play_mine(void)
{
    play_noise(0x18u, 0xF2u, 0x43u);
}

void audio_play_place(void)
{
    play_noise(0x08u, 0xB1u, 0x22u);
}

void audio_play_craft(void)
{
    play_square_ch1(0x00u, 0x40u, 0xD2u, 0x80u, 0x07u);
}

void audio_play_error(void)
{
    play_square_ch1(0x00u, 0x80u, 0xA2u, 0x80u, 0x04u);
}

void audio_update(void)
{
    if (!music_playing) {
        return;
    }

    if (music_gate_counter != 0u) {
        --music_gate_counter;
        if (music_gate_counter == 0u) {
            NR22_REG = 0x00u;
        }
    }

    if (music_frame_counter == 0u) {
        while (music_note_cursor < EYENAUT_THEME_NOTE_COUNT &&
               eyenaut_theme_notes[music_note_cursor].step == music_step) {
            play_music_note(&eyenaut_theme_notes[music_note_cursor]);
            ++music_note_cursor;
        }
    }

    ++music_frame_counter;
    if (music_frame_counter >= EYENAUT_THEME_STEP_FRAMES) {
        music_frame_counter = 0u;
        ++music_step;

        if (music_step >= EYENAUT_THEME_TOTAL_STEPS) {
            music_step = 0u;
            music_note_cursor = 0u;
        }
    }
}
