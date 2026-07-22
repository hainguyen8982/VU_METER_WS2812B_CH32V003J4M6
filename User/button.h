#ifndef __BUTTON_H
#define __BUTTON_H

#include "ch32v00x.h"

typedef enum {
    BTN_NONE = 0,
    BTN_SHORT_PRESS,
    BTN_LONG_PRESS,
    BTN_HELD_TICK       // Periodic event triggered during active holding
} ButtonEvent_t;

void Button_Init(void);
ButtonEvent_t Button_ReadEvent(void);

#endif /* __BUTTON_H */
