#include <stdio.h>
#include <stdlib.h>

#include <time.h>

#include "./TempSensor/TempSensor.hpp"
#include "./ParmsBoard/Board.hpp"

graph_point_t Temp_Graph[] = {
    {100.0f, 185.0f},
    {70.0f, 450.0f},
    {38.0f, 1800.0f},
    {20.0f, 3400.0f},
    {4.0f, 7500.0f},
    {-7.0f, 13500.0f},
    {-18.0f, 25000.0f},
    {-40.0f, 100700.0f}
};

int main(void)
{

    stdio_init_all();

    graph_point_t * temp_map_ref = &Temp_Graph[0];
    TempSensor temp_sensor(ADC_OFFSET_GPIO + ADC_CH1, temp_map_ref);

    while(1)
    {
        sleep_ms(500);
        float temp = temp_sensor.ReadTempAsCelsius();
        printf("Temperature is %0.2f\n", temp);
    }
    return 0;
}