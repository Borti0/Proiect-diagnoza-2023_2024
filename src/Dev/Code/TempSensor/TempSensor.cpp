#include "TempSensor.hpp"

GenericAnalogTempSensor::GenericAnalogTempSensor()
{
    this->adc_gpio = ADC_OFFSET_GPIO;
    this->adc_nr_channel = ADC_CH1;

    adc_init();
    adc_gpio_init(this->adc_gpio);

    return;
}

GenericAnalogTempSensor::GenericAnalogTempSensor(unsigned int adc_ch)
{
    this->adc_gpio = ADC_OFFSET_GPIO + adc_ch;
    this->adc_nr_channel = adc_ch;

    adc_init();
    adc_gpio_init(this->adc_gpio);

    return;
}

GenericAnalogTempSensor::~GenericAnalogTempSensor()
{
    return;
}

void GenericAnalogTempSensor::CheckChannelSensor(void)
{
    if(adc_get_selected_input() != this->adc_nr_channel)
    {
        adc_select_input(this->adc_nr_channel);
    }
    return;
}

uint16_t GenericAnalogTempSensor::ReadTempRaw(void)
{
    this->CheckChannelSensor();
    return adc_read();
}

float GenericAnalogTempSensor::ReadTempAsVoltage(void)
{
    this->CheckChannelSensor();
    return __SAMPLE_TO_VOLTEGE__((float)adc_read());
}

void TempSensor::__fint_interva_limits(float resistance)
{

    float local_resistace = resistance;
    /*
    out of range check dtc
    */
    if(local_resistace < this->mem_table[0].resistace)
    {
        this->_temp_sensor_dtc_ = TEMP_SENSOR_OOF_HIGH;
        return;
    }

    if(local_resistace > this->mem_table[this->len_mem_table-1].resistace)
    {
        this->_temp_sensor_dtc_ = TEMP_SENSOR_OOF_LOW;
        return;
    }
    
    /*
    check interval
    */
    for(unsigned int index = 0; index < this->len_mem_table; index++)
    {
        if(this->mem_table[index].resistace <= local_resistace &&
        this->mem_table[index+1].resistace >= local_resistace)
        {
            memset(this->mem_table_interval, 0, (sizeof(graph_point_t) * LEN_INTERVAL));
            memcpy(this->mem_table_interval, (this->mem_table + index), (sizeof(graph_point_t) * LEN_INTERVAL));
            return;
        }
    }
}

float TempSensor::ReadTempAsResistance(void)
{
    return __VOLTAGE_TO_RES(this->ReadTempAsVoltage());
}

float TempSensor::ReadTempAsCelsius(void)
{
    float temperature = 0.0f;
    float resistance = 0.0f;

    resistance = this->ReadTempAsResistance();

    this->__fint_interva_limits(resistance);

    temperature = __RES_TO_TEMP__ (
        resistance,
        this->mem_table_interval[0].resistace,
        this->mem_table_interval[1].resistace,
        this->mem_table_interval[0].temperature_celsius,
        this->mem_table_interval[1].temperature_celsius
    );

    return temperature;
}

void TempSensor::show_map(void)
{
    printf("Sensor mem table [index: R - T]\n");
    for(unsigned int index = 0; index < this->len_mem_table; index++)
    {
        printf("%d: %f - %f\n", index, this->mem_table[index].resistace, this->mem_table[index].temperature_celsius);
    }
    printf("\n");

    return;
}

void TempSensor::show_limits(void)
{
    printf("Temperature interval [index: R - T]\n");
    for(unsigned int index = 0; index < LEN_INTERVAL; index++)
    {
        printf("%d: %f - %f\n", index, this->mem_table_interval[index].resistace, this->mem_table_interval[index].temperature_celsius);
    }
    printf("\n");
    return;
}

unsigned int TempSensor::get_len_map(void)
{
    return this->len_mem_table;
}

unsigned int TempSensor::get_dtc(void)
{
    return this->_temp_sensor_dtc_;
}

TempSensor::~TempSensor()
{
    return;
}



