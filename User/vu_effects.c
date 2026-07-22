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

// Color Wheel offset for animated modes
static uint8_t rainbow_offset = 0;

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
    rainbow_offset = 0;
}

void VU_Effects_SetMode(VU_EffectMode_t mode)
{
    current_mode = mode % NUM_EFFECT_MODES;
}

VU_EffectMode_t VU_Effects_GetMode(void)
{
    return current_mode;
}

void VU_Effects_NextMode(void)
{
    current_mode = (VU_EffectMode_t)((current_mode + 1) % NUM_EFFECT_MODES);
}

// Helper: Color lookup for Classic Gradient (Green -> Yellow -> Red) on 16 LEDs
static Color_t Get_Classic_Color(uint8_t step)
{
    if (step < 10) {
        // Green to Lime/Yellow
        return RGB((step * 25), 220, 0);
    } else if (step < 14) {
        // Yellow to Orange
        return RGB(255, (220 - (step - 10) * 55), 0);
    } else {
        // Red (Overdrive)
        return RGB(255, 0, 0);
    }
}

void VU_Effects_Update(AudioLevels_t levels)
{
    WS2812_Clear();
    rainbow_offset += 2; // Cycle animation speed

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
        peak_hold_L = 48; // Hold peak for ~48 frames (~800ms)
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
        peak_hold_R = 48; // Hold peak for ~48 frames (~800ms)
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

    // Draw Left Channel (LEDs 0 to 15) & Right Channel (LEDs 16 to 31)
    switch (current_mode) {

    case MODE_CLASSIC_VU:
        // Left Channel
        for (uint8_t i = 0; i < draw_L; i++) {
            WS2812_SetLEDColor(i, Get_Classic_Color(i));
        }
        if (peak_L > 0 && peak_L <= 16) {
            WS2812_SetLEDColor(peak_L - 1, RGB(255, 255, 255)); // White Peak Dot
        }

        // Right Channel
        for (uint8_t i = 0; i < draw_R; i++) {
            WS2812_SetLEDColor(16 + i, Get_Classic_Color(i));
        }
        if (peak_R > 0 && peak_R <= 16) {
            WS2812_SetLEDColor(16 + peak_R - 1, RGB(255, 255, 255)); // White Peak Dot
        }
        break;

    case MODE_RAINBOW_VU:
        // Left Channel
        for (uint8_t i = 0; i < draw_L; i++) {
            uint8_t hue = (i * 15 + rainbow_offset) & 0xFF;
            WS2812_SetLEDColor(i, Wheel(hue));
        }
        if (peak_L > 0 && peak_L <= 16) {
            WS2812_SetLEDColor(peak_L - 1, RGB(255, 255, 255));
        }

        // Right Channel
        for (uint8_t i = 0; i < draw_R; i++) {
            uint8_t hue = (i * 15 + rainbow_offset) & 0xFF;
            WS2812_SetLEDColor(16 + i, Wheel(hue));
        }
        if (peak_R > 0 && peak_R <= 16) {
            WS2812_SetLEDColor(16 + peak_R - 1, RGB(255, 255, 255));
        }
        break;

    case MODE_CENTER_OUT:
        // Left Channel (Starts from LED 7/8 expanding outwards)
        {
            uint8_t fill_L = draw_L / 2; // Max 8 steps of expansion
            for (uint8_t i = 0; i < fill_L; i++) {
                Color_t col = Wheel((i * 30 + rainbow_offset) & 0xFF);
                if (7 >= i) WS2812_SetLEDColor(7 - i, col);
                if (8 + i < 16) WS2812_SetLEDColor(8 + i, col);
            }
        }
        // Right Channel (Starts from LED 23/24 expanding outwards)
        {
            uint8_t fill_R = draw_R / 2;
            for (uint8_t i = 0; i < fill_R; i++) {
                Color_t col = Wheel((i * 30 + rainbow_offset) & 0xFF);
                if (23 >= (16 + i)) WS2812_SetLEDColor(23 - i, col);
                if (24 + i < 32) WS2812_SetLEDColor(24 + i, col);
            }
        }
        break;

    case MODE_FIRE_HEAT:
        // Left Channel
        for (uint8_t i = 0; i < draw_L; i++) {
            uint8_t heat = (i * 16);
            WS2812_SetLEDColor(i, RGB(255, heat, (heat > 128 ? heat - 128 : 0)));
        }
        if (peak_L > 0 && peak_L <= 16) {
            WS2812_SetLEDColor(peak_L - 1, RGB(255, 255, 200));
        }

        // Right Channel
        for (uint8_t i = 0; i < draw_R; i++) {
            uint8_t heat = (i * 16);
            WS2812_SetLEDColor(16 + i, RGB(255, heat, (heat > 128 ? heat - 128 : 0)));
        }
        if (peak_R > 0 && peak_R <= 16) {
            WS2812_SetLEDColor(16 + peak_R - 1, RGB(255, 255, 200));
        }
        break;

    case MODE_OCEAN_BLUE:
        // Left Channel (Deep Blue -> Cyan -> White)
        for (uint8_t i = 0; i < draw_L; i++) {
            WS2812_SetLEDColor(i, RGB(0, i * 15, 100 + i * 9));
        }
        if (peak_L > 0 && peak_L <= 16) {
            WS2812_SetLEDColor(peak_L - 1, RGB(0, 255, 255));
        }

        // Right Channel
        for (uint8_t i = 0; i < draw_R; i++) {
            WS2812_SetLEDColor(16 + i, RGB(0, i * 15, 100 + i * 9));
        }
        if (peak_R > 0 && peak_R <= 16) {
            WS2812_SetLEDColor(16 + peak_R - 1, RGB(0, 255, 255));
        }
        break;

    case MODE_SOLID_PULSE:
        // Left Channel: Purple / Magenta
        for (uint8_t i = 0; i < draw_L; i++) {
            WS2812_SetLEDColor(i, RGB(180, 0, 255));
        }
        // Right Channel: Cyan
        for (uint8_t i = 0; i < draw_R; i++) {
            WS2812_SetLEDColor(16 + i, RGB(0, 220, 255));
        }
        break;
    }

    WS2812_Show();
}
