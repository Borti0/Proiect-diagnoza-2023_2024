#include "TempSensor.hpp"

TempSensor::TempSensor(unsigned int nr_adc_gpio)
{
    this->adc_gpio = nr_adc_gpio;

    adc_init();
    adc_gpio_init(this->adc_gpio);
}

TempSensor::~TempSensor()
{
}