#include "ws2812.h"

Color_t leds[NUM_LEDS];
uint8_t global_brightness = 70; // Default brightness for accurate color mixing

void WS2812_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    // Enable GPIOC Clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    // Configure PC2 as Output Push-Pull 50MHz
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // Default Low
    GPIO_ResetBits(GPIOC, GPIO_Pin_2);

    WS2812_Clear();
    WS2812_Show();
}

Color_t RGB(uint8_t r, uint8_t g, uint8_t b)
{
    Color_t c;
    c.r = r;
    c.g = g;
    c.b = b;
    return c;
}

void WS2812_SetLED(uint8_t index, uint8_t r, uint8_t g, uint8_t b)
{
    if (index < NUM_LEDS) {
        leds[index].r = r;
        leds[index].g = g;
        leds[index].b = b;
    }
}

void WS2812_SetLEDColor(uint8_t index, Color_t color)
{
    if (index < NUM_LEDS) {
        leds[index] = color;
    }
}

void WS2812_Clear(void)
{
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
        leds[i].r = 0;
        leds[i].g = 0;
        leds[i].b = 0;
    }
}

void WS2812_SetBrightness(uint8_t brightness)
{
    global_brightness = brightness;
}

Color_t Wheel(uint8_t WheelPos)
{
    Color_t c;
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85) {
        c.r = 255 - WheelPos * 3;
        c.g = 0;
        c.b = WheelPos * 3;
    } else if (WheelPos < 170) {
        WheelPos -= 85;
        c.r = 0;
        c.g = WheelPos * 3;
        c.b = 255 - WheelPos * 3;
    } else {
        WheelPos -= 170;
        c.r = WheelPos * 3;
        c.g = 255 - WheelPos * 3;
        c.b = 0;
    }
    return c;
}

/*
 * Bit-bang WS2812 data stream on PC2 (48MHz HCLK)
 * High pulse for bit 1: ~800ns - 900ns
 * High pulse for bit 0: ~300ns - 400ns
 * Total bit period: ~1.25us
 */
void WS2812_Show(void)
{
    // Disable interrupts to prevent timing jitter
    __disable_irq();

    for (uint8_t i = 0; i < NUM_LEDS; i++) {
        // Apply brightness scaling
        uint8_t g = (uint8_t)(((uint16_t)leds[i].g * global_brightness) >> 8);
        uint8_t r = (uint8_t)(((uint16_t)leds[i].r * global_brightness) >> 8);
        uint8_t b = (uint8_t)(((uint16_t)leds[i].b * global_brightness) >> 8);

        // WS2812 order is G, R, B
        uint32_t grb = ((uint32_t)g << 16) | ((uint32_t)r << 8) | (uint32_t)b;

        for (int8_t bit = 23; bit >= 0; bit--) {
            if ((grb >> bit) & 0x01) {
                // Bit 1: High ~850ns, Low ~400ns
                GPIOC->BSHR = GPIO_Pin_2; // Set High
                __asm__ volatile ("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
                __asm__ volatile ("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
                __asm__ volatile ("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
                __asm__ volatile ("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
                GPIOC->BSHR = (uint32_t)GPIO_Pin_2 << 16; // Set Low
                __asm__ volatile ("nop; nop; nop; nop; nop; nop; nop; nop;");
            } else {
                // Bit 0: High ~350ns, Low ~900ns
                GPIOC->BSHR = GPIO_Pin_2; // Set High
                __asm__ volatile ("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
                __asm__ volatile ("nop; nop; nop;");
                GPIOC->BSHR = (uint32_t)GPIO_Pin_2 << 16; // Set Low
                __asm__ volatile ("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
                __asm__ volatile ("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
                __asm__ volatile ("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
            }
        }
    }

    __enable_irq();

    // Reset pulse (> 50us Low)
    Delay_Us(60);
}
