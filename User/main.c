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
#include "ch32v00x_flash.h"

#define SETTINGS_FLASH_ADDR   0x08003FC0
#define SETTINGS_MAGIC        0xABCD55AA

static uint8_t brightness_levels[] = {35, 70, 120, 180};
static uint8_t brightness_idx = 1; // Default to ~27% brightness for accurate color mixing

void Settings_Load(void)
{
    uint32_t magic = *(volatile uint32_t*)SETTINGS_FLASH_ADDR;
    if (magic == SETTINGS_MAGIC) {
        uint32_t data = *(volatile uint32_t*)(SETTINGS_FLASH_ADDR + 4);
        uint8_t saved_mode = (uint8_t)(data >> 8);
        uint8_t saved_brightness = (uint8_t)data;
        
        // Validate saved mode (0 to NUM_EFFECT_MODES-1)
        if (saved_mode < NUM_EFFECT_MODES) {
            VU_Effects_SetMode((VU_EffectMode_t)saved_mode);
        }
        
        // Validate saved brightness (15 to 250)
        if (saved_brightness >= 15 && saved_brightness <= 250) {
            WS2812_SetBrightness(saved_brightness);
        }
        printf("Settings Loaded from Flash: Mode %d, Brightness %d\r\n", saved_mode, global_brightness);
    } else {
        printf("No valid settings found in Flash. Using defaults.\r\n");
    }
}

void Settings_Save(void)
{
    uint8_t current_mode = (uint8_t)VU_Effects_GetMode();
    uint8_t current_brightness = global_brightness;
    uint32_t data = ((uint32_t)current_mode << 8) | current_brightness;
    
    // Check if the settings have actually changed to avoid redundant writes
    uint32_t old_magic = *(volatile uint32_t*)SETTINGS_FLASH_ADDR;
    uint32_t old_data = *(volatile uint32_t*)(SETTINGS_FLASH_ADDR + 4);
    if (old_magic == SETTINGS_MAGIC && old_data == data) {
        return; // No change, skip write
    }

    __disable_irq(); // Disable interrupts during Flash write to prevent timing disruption
    
    FLASH_Unlock();
    FLASH_ErasePage(SETTINGS_FLASH_ADDR);
    FLASH_ProgramWord(SETTINGS_FLASH_ADDR, SETTINGS_MAGIC);
    FLASH_ProgramWord(SETTINGS_FLASH_ADDR + 4, data);
    FLASH_Lock();
    
    __enable_irq();
    printf("Settings Saved to Flash: Mode %d, Brightness %d\r\n", current_mode, current_brightness);
}

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
    Settings_Load(); // Load saved mode & brightness from Flash

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
            Settings_Save(); // Save mode change instantly
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
                if (was_holding) {
                    Settings_Save(); // Save to Flash ONLY when button is released after dimming
                    was_holding = 0;
                }
            }
        }

        // Maintain ~60 FPS refresh rate (paced by 15ms ADC sampling)
        Delay_Ms(1);
    }
}
