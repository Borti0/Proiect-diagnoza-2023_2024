#ifndef _TEMPSENSOR_
#define _TEMPSENSOR_

#include "stdio.h"
#include "stdlib.h"

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"

//#include "../types.hpp"
#include "../ParmsBoard/Board.hpp"


class TempSensor
{
private:
    unsigned int adc_gpio = 0;
public:
    TempSensor(unsigned int nr_adc_gpio);
    ~TempSensor();

    uint16_t ReadTempRaw(void);
    float ReadTempAsVoltage(void);
    float ReadTempAsResistance(void);
    float ReadTempAsCelsisus(void);    
};


#endif