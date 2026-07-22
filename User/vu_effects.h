#ifndef __VU_EFFECTS_H
#define __VU_EFFECTS_H

#include "ws2812.h"
#include "adc_audio.h"

#define NUM_EFFECT_MODES 6

typedef enum {
    MODE_CLASSIC_VU = 0,     // Green-Yellow-Red Gradient with White Peak
    MODE_RAINBOW_VU,         // Rainbow Spectrum Gradient
    MODE_CENTER_OUT,         // Expansion from center
    MODE_FIRE_HEAT,          // Fire / Heatmap volume reaction
    MODE_OCEAN_BLUE,         // Cyan/Blue cool palette
    MODE_SOLID_PULSE         // Minimalist Solid Color
} VU_EffectMode_t;

void VU_Effects_Init(void);
void VU_Effects_SetMode(VU_EffectMode_t mode);
VU_EffectMode_t VU_Effects_GetMode(void);
void VU_Effects_NextMode(void);
void VU_Effects_Update(AudioLevels_t levels);

#endif /* __VU_EFFECTS_H */
