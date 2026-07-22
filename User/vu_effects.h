#ifndef __VU_EFFECTS_H
#define __VU_EFFECTS_H

#include "ws2812.h"
#include "adc_audio.h"

#define NUM_EFFECT_MODES 15

typedef enum {
    MODE_CLASSIC_VU = 0,     // Green-Yellow-Red Gradient with White Peak
    MODE_RAINBOW_VU,         // Rainbow Spectrum Gradient
    MODE_CENTER_OUT,         // Expansion from center
    MODE_FIRE_HEAT,          // Fire / Heatmap volume reaction
    MODE_OCEAN_BLUE,         // Cyan/Blue cool palette
    MODE_SOLID_PULSE,        // Minimalist Solid Color
    MODE_CLUB_STROBE,        // Flash entire channel with audio volume (Disco strobe)
    MODE_SPLIT_COLOR,        // Sharp Dual-Color Split (Cyan / Magenta)
    MODE_OUT_IN,             // Inward growth from outer ends to center
    MODE_GREEN_RED_PEAK,     // Solid Green column with a Red Peak Dot
    MODE_RAINBOW_FLOW,       // Rainbow ripple flow animation
    MODE_INTERLACED_FLOW,    // DNA double helix cross-flow
    MODE_FADING_GLOW,        // Shooting sparks with slowly fading glowing trails
    MODE_COLLISION_BEAT,     // Mirror columns colliding in the center
    MODE_LEVEL_ALERT,        // Single color column that changes color with height
    MODE_RAINBOW_ACCORDION   // Rainbow that stretches/squashes with volume
} VU_EffectMode_t;

void VU_Effects_Init(void);
void VU_Effects_SetMode(VU_EffectMode_t mode);
VU_EffectMode_t VU_Effects_GetMode(void);
void VU_Effects_NextMode(void);
void VU_Effects_Update(AudioLevels_t levels);

#endif /* __VU_EFFECTS_H */
