/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : Antigravity AI
 * Version            : V1.0.0
 * Date               : 2026/07/21
 * Description        : Stereo VU LED Meter (32 LEDs WS2812) for CH32V003J4M6
 *******************************************************************************/

#include "debug.h"
#include "ws2812.h"
#include "adc_audio.h"
#include "button.h"
#include "vu_effects.h"

static uint8_t brightness_levels[] = {35, 70, 120, 180};
static uint8_t brightness_idx = 1; // Default to ~27% brightness for accurate color mixing

void Startup_LED_Animation(void)
{
    // Rainbow boot animation sweep across all 32 LEDs
    for (uint16_t j = 0; j < 256; j += 4) {
        for (uint8_t i = 0; i < NUM_LEDS; i++) {
            WS2812_SetLEDColor(i, Wheel((i * 8 + j) & 0xFF));
        }
        WS2812_Show();
        Delay_Ms(5);
    }
    WS2812_Clear();
    WS2812_Show();
}

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();

#if (SDI_PRINT == SDI_PR_OPEN)
    SDI_Printf_Enable();
#else
    USART_Printf_Init(115200);
#endif

    printf("==========================================\r\n");
    printf("CH32V003J4M6 Stereo VU LED Meter 32 LEDs\r\n");
    printf("SystemClk: %d Hz\r\n", SystemCoreClock);
    printf("==========================================\r\n");

    // Initialize Hardware & Modules
    WS2812_Init();
    WS2812_SetBrightness(brightness_levels[brightness_idx]);

    ADC_Audio_Init();
    Button_Init();
    VU_Effects_Init();

    // Play startup rainbow sweep
    Startup_LED_Animation();

    while (1)
    {
        // 1. Read Stereo Audio Levels from PA2 (Left) & PD6 (Right)
        AudioLevels_t audio = ADC_Audio_ReadLevels();

        // 2. Render & Update VU Effects on 32 LEDs
        VU_Effects_Update(audio);

        // 3. Process Mode Button Event on PC4
        static int8_t brightness_dir = 1; // 1 for up, -1 for down
        static uint8_t was_holding = 0;
        ButtonEvent_t btn_evt = Button_ReadEvent();

        if (btn_evt == BTN_SHORT_PRESS) {
            VU_Effects_NextMode();
            printf("Mode Changed: %d\r\n", VU_Effects_GetMode());
        } else if (btn_evt == BTN_HELD_TICK) {
            if (!was_holding) {
                brightness_dir = -brightness_dir; // Toggle direction on new hold
                was_holding = 1;
            }
            int16_t new_brightness = (int16_t)global_brightness + (brightness_dir * 4);
            if (new_brightness >= 250) {
                new_brightness = 250;
                brightness_dir = -1; // Reverse on hitting upper limit
            } else if (new_brightness <= 15) {
                new_brightness = 15;
                brightness_dir = 1;  // Reverse on hitting lower limit
            }
            WS2812_SetBrightness((uint8_t)new_brightness);
            printf("Brightness Holding: %d\r\n", global_brightness);
        } else if (btn_evt == BTN_NONE) {
            // Reset hold state when button is physically released (IPU pin reads high)
            if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) == 1) {
                was_holding = 0;
            }
        }

        // Maintain ~60 FPS refresh rate (paced by 15ms ADC sampling)
        Delay_Ms(1);
    }
}
