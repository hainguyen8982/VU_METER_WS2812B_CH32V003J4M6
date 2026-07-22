#include "vu_effects.h"

static VU_EffectMode_t current_mode = MODE_CLASSIC_VU;

// Peak Hold State Variables
static uint8_t peak_L = 0;
static uint8_t peak_R = 0;
static uint8_t peak_hold_L = 0;
static uint8_t peak_hold_R = 0;
static uint8_t peak_decay_counter_L = 0;
static uint8_t peak_decay_counter_R = 0;

// Smoothed Display Levels (Fast attack, slow decay, scaled 0 to 160)
static uint8_t display_L = 0;
static uint8_t display_R = 0;

void VU_Effects_Init(void)
{
    current_mode = MODE_CLASSIC_VU;
    peak_L = 0;
    peak_R = 0;
    peak_hold_L = 0;
    peak_hold_R = 0;
    peak_decay_counter_L = 0;
    peak_decay_counter_R = 0;
    display_L = 0;
    display_R = 0;
}

void VU_Effects_SetMode(VU_EffectMode_t mode)
{
    current_mode = mode;
}

VU_EffectMode_t VU_Effects_GetMode(void)
{
    return current_mode;
}

void VU_Effects_NextMode(void)
{
    // Kept as no-op since only 1 effect is used
}

void VU_Effects_Update(AudioLevels_t levels)
{
    WS2812_Clear();

    uint8_t target_L = levels.left_level * 10;
    uint8_t target_R = levels.right_level * 10;

    // Smooth Left Channel Level (Fast attack, high-res smooth decay)
    if (target_L > display_L) {
        display_L = target_L; // Instant rise
    } else if (display_L > 0) {
        if (display_L >= 3) display_L -= 3; // Decay 3 units per frame (~0.8s total fall)
        else display_L = 0;
    }

    // Smooth Right Channel Level (Fast attack, high-res smooth decay)
    if (target_R > display_R) {
        display_R = target_R; // Instant rise
    } else if (display_R > 0) {
        if (display_R >= 3) display_R -= 3;
        else display_R = 0;
    }

    uint8_t draw_L = display_L / 10;
    uint8_t draw_R = display_R / 10;

    // Update Peak Hold & Decay Physics (Left) based on physical height (0 to 16)
    if (draw_L >= peak_L) {
        peak_L = draw_L;
        peak_hold_L = 16; // Hold peak for ~16 frames (~260ms)
        peak_decay_counter_L = 0;
    } else {
        if (peak_hold_L > 0) {
            peak_hold_L--;
        } else if (peak_L > 0) {
            peak_decay_counter_L++;
            if (peak_decay_counter_L >= 2) { // Fall 1 step every 2 frames (~33ms per step)
                peak_L--;
                peak_decay_counter_L = 0;
            }
        }
    }

    // Update Peak Hold & Decay Physics (Right) based on physical height (0 to 16)
    if (draw_R >= peak_R) {
        peak_R = draw_R;
        peak_hold_R = 16; // Hold peak for ~16 frames (~260ms)
        peak_decay_counter_R = 0;
    } else {
        if (peak_hold_R > 0) {
            peak_hold_R--;
        } else if (peak_R > 0) {
            peak_decay_counter_R++;
            if (peak_decay_counter_R >= 2) { // Fall 1 step every 2 frames (~33ms per step)
                peak_R--;
                peak_decay_counter_R = 0;
            }
        }
    }

    // Draw Left Channel (LEDs 0 to 15) - Solid Green (GRB)
    for (uint8_t i = 0; i < draw_L; i++) {
        WS2812_SetLEDColor(i, RGB(0, 255, 0));
    }
    if (peak_L > 0 && peak_L <= 16) {
        WS2812_SetLEDColor(peak_L - 1, RGB(0, 255, 0));
    }

    // Draw Right Channel (LEDs 16 to 31) - Solid Green
    for (uint8_t i = 0; i < draw_R; i++) {
        WS2812_SetLEDColor(16 + i, RGB(0, 255, 0));
    }
    if (peak_R > 0 && peak_R <= 16) {
        WS2812_SetLEDColor(16 + peak_R - 1, RGB(0, 255, 0));
    }

    WS2812_Show();
}
