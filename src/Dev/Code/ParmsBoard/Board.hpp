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

#define __RES_TO_TEMP__(RES, RES0, RES1, TEMP0, TEMP1) ( TEMP1 + ( ((RES - RES1) * (TEMP1 - TEMP0)) / (RES1 - RES0)))

#define KL15_GPIO   22 //in
#define VSS_GPIO    21 //in
#define DEBUG_GPIO  25 //out
#define DTC_GPIO    20 //out

#endif