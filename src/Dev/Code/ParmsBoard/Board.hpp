#ifndef _BOARD_
#define _BOARD_

#include "stdio.h"
#include "stdlib.h"

#define VDD 5.0f
#define VREF 3.3f
#define Rb 56000

#define ADC_OFFSET_GPIO 26
#define ADC_CH1 0
#define ADC_CH2 1
#define ADC_CH3 2
#define ADC_CH4 3
#define ADC_TEMP_ON_BOARD 4

#define __ADC_CONV_FACTOR__ (VREF / (1 << 12))
#define __SAMPLE_TO_VOLTEGE__(sample) (sample * __ADC_CONV_FACTOR__)
#define __VOLTAGE_TO_RES(voltage) ( \
    (voltage*Rb)/ (VDD - voltage)\
    )

#endif