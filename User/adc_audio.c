#include "adc_audio.h"

// Direct register fast ADC reading to bypass slow library function inside loop
static uint16_t Read_ADC_Channel_Fast(uint8_t channel)
{
    // Clear and set Channel in RSQR3 rank 1 (lowest 5 bits)
    ADC1->RSQR3 = (ADC1->RSQR3 & ~0x1F) | channel;
    
    // Start conversion (Set ADON bit)
    ADC1->CTLR2 |= ADC_ADON;
    
    // Poll EOC flag in status register
    while (!(ADC1->STATR & ADC_EOC));
    
    // Return conversion value (Reading RDATAR automatically clears EOC flag)
    return ADC1->RDATAR;
}

// Integer square root algorithm for logarithmic scaling
static uint32_t isqrt(uint32_t n) {
    uint32_t root = 0;
    uint32_t bit = 1UL << 30;
    while (bit > n) bit >>= 2;
    while (bit != 0) {
        if (n >= root + bit) {
            n -= root + bit;
            root = (root >> 1) + bit;
        } else {
            root >>= 1;
        }
        bit >>= 2;
    }
    return root;
}

void ADC_Audio_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    ADC_InitTypeDef ADC_InitStructure = {0};

    // Enable Clocks (GPIOA for PA2, GPIOD for PD6, AFIO, ADC1)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO | RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8); // 48MHz / 8 = 6MHz ADC Clock

    // Disable OSC function on PA1/PA2 to use as normal GPIO/ADC (Set PA12_RM bit to 1)
    AFIO->PCFR1 |= (1 << 15);

    // Configure PA2 (ADC Channel 0) as Analog Input (Left Channel)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Configure PD6 (ADC Channel 6) as Analog Input (Right Channel)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // ADC1 Configuration
    ADC_DeInit(ADC1);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_Cmd(ADC1, ENABLE);

    // ADC Calibration
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));

    // Pre-configure sample times once for fast switching inside read loop
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_43Cycles);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 1, ADC_SampleTime_43Cycles);
}

AudioLevels_t ADC_Audio_ReadLevels(void)
{
    AudioLevels_t result;
    uint16_t min_L = 1024, max_L = 0;
    uint16_t min_R = 1024, max_R = 0;

    // Rapid sampling over a window (~15ms per frame at 600 cycles)
    for (uint16_t i = 0; i < ADC_SAMPLE_WINDOW; i++) {
        uint16_t val_L = Read_ADC_Channel_Fast(ADC_Channel_0);
        uint16_t val_R = Read_ADC_Channel_Fast(ADC_Channel_6);

        if (val_L < min_L) min_L = val_L;
        if (val_L > max_L) max_L = val_L;

        if (val_R < min_R) min_R = val_R;
        if (val_R > max_R) max_R = val_R;
    }

    uint16_t p2p_L = (max_L > min_L) ? (max_L - min_L) : 0;
    uint16_t p2p_R = (max_R > min_R) ? (max_R - min_R) : 0;

    // Apply EMA smoothing to filter out random single-frame spikes
    static uint16_t ema_p2p_L = 0;
    static uint16_t ema_p2p_R = 0;
    ema_p2p_L = (ema_p2p_L * 2 + p2p_L) / 3;
    ema_p2p_R = (ema_p2p_R * 2 + p2p_R) / 3;

    result.left_raw_p2p = ema_p2p_L;
    result.right_raw_p2p = ema_p2p_R;

    // Sliding window history to detect signal variance (dynamic music vs static hum)
    static uint16_t hist_L[16] = {0};
    static uint16_t hist_R[16] = {0};
    static uint8_t hist_idx = 0;

    hist_L[hist_idx] = ema_p2p_L;
    hist_R[hist_idx] = ema_p2p_R;
    hist_idx = (hist_idx + 1) % 16;

    // Calculate max and min in the sliding window of 250ms
    uint16_t max_hist_L = 0, min_hist_L = 1024;
    uint16_t max_hist_R = 0, min_hist_R = 1024;
    for (uint8_t i = 0; i < 16; i++) {
        if (hist_L[i] > max_hist_L) max_hist_L = hist_L[i];
        if (hist_L[i] < min_hist_L) min_hist_L = hist_L[i];

        if (hist_R[i] > max_hist_R) max_hist_R = hist_R[i];
        if (hist_R[i] < min_hist_R) min_hist_R = hist_R[i];
    }
    uint16_t diff_L = max_hist_L - min_hist_L;
    uint16_t diff_R = max_hist_R - min_hist_R;

    // Noise subtraction & cutoff (Tối ưu hóa cân bằng giữa nhạc dạo cực nhỏ và lọc nhiễu tĩnh)
    int an_izm_l = 0;
    int an_izm_r = 0;

    // Left Channel: Mute instantly if signal is extremely flat (hum) and low
    if (ema_p2p_L > 22) {
        if (diff_L < 5 && ema_p2p_L < 65) {
            an_izm_l = 0; // Classify as static hum -> Mute
        } else {
            an_izm_l = ema_p2p_L - 15; // Active music -> High sensitivity subtraction
        }
    }

    // Right Channel: Mute instantly if signal is extremely flat (hum) and low
    if (ema_p2p_R > 22) {
        if (diff_R < 5 && ema_p2p_R < 65) {
            an_izm_r = 0; // Classify as static hum -> Mute
        } else {
            an_izm_r = ema_p2p_R - 15; // Active music -> High sensitivity subtraction
        }
    }

    // Shared AGC Max Peak & Min Floor Tracking
    static int dynamic_max = 64;
    static int dynamic_min = 0;
    int max_val = (an_izm_l > an_izm_r) ? an_izm_l : an_izm_r;
    int min_val = (an_izm_l < an_izm_r) ? an_izm_l : an_izm_r;

    // Peak tracking (Đỉnh âm lượng - Đặt mặc định 64 khi im lặng)
    if (max_val > dynamic_max) {
        dynamic_max = max_val;
    } else if (max_val == 0) {
        // Reset dynamic_max and dynamic_min instantly on pause/silence to prevent delays
        dynamic_max = 64;
        dynamic_min = 0;
    } else if (dynamic_max > 64) {
        dynamic_max -= (dynamic_max >> 5) + 1; // Smooth decay
    }

    // Floor tracking (Đáy âm lượng thực tế)
    if (max_val > 0) {
        if (min_val < dynamic_min) {
            dynamic_min = min_val;
        } else {
            dynamic_min += (min_val - dynamic_min) >> 5; // Co giãn sát theo đáy nhạc thực tế
        }
    } else {
        dynamic_min = 0;
    }

    // Active dynamic range window
    uint32_t active_range = (dynamic_max > dynamic_min) ? (dynamic_max - dynamic_min) : 48;
    if (active_range < 48) active_range = 48;

    // Square-Root Scaling for high dynamic range and responsiveness
    uint8_t lvl_L = 0;
    uint8_t lvl_R = 0;

    if (an_izm_l > dynamic_min) {
        uint32_t ratio_l = ((uint32_t)(an_izm_l - dynamic_min) * 10000UL) / active_range;
        if (ratio_l > 10000UL) ratio_l = 10000UL;
        uint32_t sqrt_l = isqrt(ratio_l * 10000UL);
        lvl_L = (uint8_t)((sqrt_l * AUDIO_MAX_LEVEL) / 10000UL);
    }

    if (an_izm_r > dynamic_min) {
        uint32_t ratio_r = ((uint32_t)(an_izm_r - dynamic_min) * 10000UL) / active_range;
        if (ratio_r > 10000UL) ratio_r = 10000UL;
        uint32_t sqrt_r = isqrt(ratio_r * 10000UL);
        lvl_R = (uint8_t)((sqrt_r * AUDIO_MAX_LEVEL) / 10000UL);
    }

    if (lvl_L > AUDIO_MAX_LEVEL) lvl_L = AUDIO_MAX_LEVEL;
    if (lvl_R > AUDIO_MAX_LEVEL) lvl_R = AUDIO_MAX_LEVEL;

    // Temporary debug print to diagnose the ground hum parameters
    static uint8_t print_cnt = 0;
    if (++print_cnt >= 30) {
        print_cnt = 0;
        printf("DBG: p2p=%d, diff=%d, max=%d, min=%d\r\n", ema_p2p_L, diff_L, dynamic_max, dynamic_min);
    }

    result.left_level = lvl_L;
    result.right_level = lvl_R;

    return result;
}
