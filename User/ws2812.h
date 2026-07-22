#ifndef __WS2812_H
#define __WS2812_H

#include "ch32v00x.h"

#define NUM_LEDS          32
#define LEDS_PER_CHANNEL  16

// Color structure (GRB order for WS2812)
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Color_t;

// Global LED buffer
extern Color_t leds[NUM_LEDS];
extern uint8_t global_brightness; // 0 to 255 (scale factor)

void WS2812_Init(void);
void WS2812_Show(void);
void WS2812_SetLED(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
void WS2812_SetLEDColor(uint8_t index, Color_t color);
void WS2812_Clear(void);
void WS2812_SetBrightness(uint8_t brightness);

// Helper function to create RGB color
Color_t RGB(uint8_t r, uint8_t g, uint8_t b);

// Helper function for Wheel color generator (0-255)
Color_t Wheel(uint8_t WheelPos);

#endif /* __WS2812_H */
