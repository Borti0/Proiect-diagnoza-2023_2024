#ifndef _TEMPSENSOR_
#define _TEMPSENSOR_

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "pico/stdlib.h"
#include "hardware/adc.h"
//#include "hardware/gpio.h"

//#include "../types.hpp"
#include "../ParmsBoard/Board.hpp"
#include "SensorMap.hpp"

#define __NEL__(map) (sizeof(map) / sizeof((map)[0]))

#define LEN_INTERVAL 2

#define TEMP_SENSOR_OK              0x00
#define TEMP_SENSOR_MEMORY_DTC      0x01
#define TEMP_SENSOR_OOF_HIGH        0x02
#define TEMP_SENSOR_OOF_LOW         0x03
#define TEMP_SENSOR_TEMP_LOW_DTC    0x04
#define TEMP_SENSOR_GRAPH_TO_SHOR   0x05

class GenericAnalogTempSensor
{
private:
    unsigned int adc_gpio = 0;
    unsigned int adc_nr_channel = 0;

protected:
    void CheckChannelSensor(void);

public:
    GenericAnalogTempSensor();
    GenericAnalogTempSensor(unsigned int adc_ch);

    ~GenericAnalogTempSensor();

    uint16_t ReadTempRaw(void);
    float ReadTempAsVoltage(void);
};


class TempSensor : public GenericAnalogTempSensor
{
private:
    graph_point_t * mem_table = nullptr;
    unsigned int len_mem_table = 0;

    graph_point_t * mem_table_interval = nullptr;

    void __fint_interva_limits(float resistance);

    unsigned int _temp_sensor_dtc_ = TEMP_SENSOR_OK;

public:

    TempSensor(unsigned int adc_ch, graph_point_t * graph_table, unsigned int len_graph) : GenericAnalogTempSensor(adc_ch)
    {
        this->mem_table = graph_table;
        if(!this->mem_table)
        {
            this->_temp_sensor_dtc_ = TEMP_SENSOR_MEMORY_DTC;
            return;
        }   

        this->len_mem_table = len_graph;
        if(this->len_mem_table <= 1)
        {
            this->_temp_sensor_dtc_ = TEMP_SENSOR_GRAPH_TO_SHOR;
            return;
        }

        this->mem_table_interval = (graph_point_t *)malloc(LEN_INTERVAL * sizeof(graph_point_t));
        
        if(!this->mem_table_interval)
        {
            this->_temp_sensor_dtc_ = TEMP_SENSOR_MEMORY_DTC;
            return;
        }
        
        memset(this->mem_table_interval, 0, LEN_INTERVAL * sizeof(graph_point_t));

        return;
    };

    ~TempSensor();

    float ReadTempAsResistance(void);
    float ReadTempAsCelsius(void);

    void show_map(void);
    void show_limits(void);
    
    unsigned int get_len_map(void);
    unsigned int get_dtc(void);
};


#endif