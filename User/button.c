#include "button.h"

static uint8_t last_state = 1;
static uint32_t press_start_time = 0;
static uint8_t is_pressed = 0;

void Button_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    // Enable GPIOC Clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    // Configure PC4 as Input Pull-Up
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

ButtonEvent_t Button_ReadEvent(void)
{
    uint8_t current_state = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4);
    ButtonEvent_t event = BTN_NONE;

    // Active Low Button Press
    if (last_state == 1 && current_state == 0) {
        // Button Pressed Down
        is_pressed = 1;
        press_start_time = 0;
    } else if (last_state == 0 && current_state == 0) {
        // Button Being Held Down
        press_start_time++;
        // If held for more than 30 frames (~500ms):
        if (press_start_time > 30) {
            // Every 4 frames (~66ms), trigger a BTN_HELD_TICK event
            if ((press_start_time - 30) % 4 == 0) {
                event = BTN_HELD_TICK;
            }
        }
    } else if (last_state == 0 && current_state == 1) {
        // Button Released
        if (is_pressed) {
            // Only trigger short press if it was not held down for long-press dimming
            if (press_start_time <= 30) {
                if (press_start_time > 2) { // Short press (> 30ms debounce)
                    event = BTN_SHORT_PRESS;
                }
            }
            is_pressed = 0;
            press_start_time = 0;
        }
    }

    last_state = current_state;
    return event;
}
