#ifndef __ADC_AUDIO_H
#define __ADC_AUDIO_H

#include "ch32v00x.h"

// Configuration constants
#define ADC_SAMPLE_WINDOW    600   // Number of samples taken per audio frame (increased to 600 to capture vocals/mid/highs)
#define AUDIO_NOISE_GATE     26    // Noise threshold to ignore static hum (tuned to balance sensitivity and silence)
#define AUDIO_MAX_LEVEL      16    // Max LED height per channel (16 LEDs per channel for 32-LED mode)

typedef struct {
    uint8_t left_level;    // 0 to 16
    uint8_t right_level;   // 0 to 16
    uint16_t left_raw_p2p;
    uint16_t right_raw_p2p;
} AudioLevels_t;

void ADC_Audio_Init(void);
AudioLevels_t ADC_Audio_ReadLevels(void);

#endif /* __ADC_AUDIO_H */
