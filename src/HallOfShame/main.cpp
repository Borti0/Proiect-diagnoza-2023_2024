#include <stdio.h>
#include <stdlib.h>

#include <time.h>

#include "./TempSensor/TempSensor.hpp"
#include "./ParmsBoard/Board.hpp"
#include "./TaskScheguler/TaskScheguler.hpp"

#include "hardware/rtc.h"
#include "pico/util/datetime.h"

#define GRAPH_LEN 8
#define TASK_POOL_LEN 3

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

TaskScheguler_Status_t read_temp_task(void * params)
{
    printf("Execute read temperature task ...\n");

    TempSensor * local_temp_sensor = (TempSensor *)params;
    
    float temp = local_temp_sensor->ReadTempAsCelsius();
    float temp_lim = -35.0f;

    local_temp_sensor->show_limits();

    printf("Temperature is %0.2f\n", temp);

    TaskScheguler_Status_t ret;
    ret.done_execution = true;
    if(temp < temp_lim)
        ret.done_execution = false;
    
    return ret;
}

TaskScheguler_Status_t check_conditions(void * params)
{
    int * local_var = (int *)params;
    printf("test 2 received %d\n", * local_var);
    TaskScheguler_Status_t var;
    var.done_execution = false;
    return var;
}

TaskScheguler_Status_t read_time_task(void *params)
{
    printf("Execute read_time_task ...\n");
    datetime_t * local_date_time = (datetime_t *)params;

    char buffer[256];

    rtc_get_datetime(local_date_time);
    datetime_to_str(buffer, sizeof(buffer), local_date_time);
    printf("Time is: %s\n", buffer);
    
    TaskScheguler_Status_t var;
    var.done_execution = true;
    return var;
}

/*
gpio_callback()
{
    if(i_can_read == true)
    {
        value of the gpio
        allarm_kl15(500us);
        i_can_read = false
    }

    return;
}
*/

void callback(uint gpio_nr)
{
    printf("test\n");
    switch (gpio_nr)
    {
    case 22:
        printf("kl15 changed\n");
        break;
    case 21::
        printf("vss changed\n");
        break;
    
    default:
        break;
    }

    return;
}

int main(void)
{

    stdio_init_all();
    sleep_ms(5000);

    gpio_init(22);
    gpio_init(21);
    gpio_set_dir(22, GPIO_IN);
    gpio_set_dir(21, GPIO_IN);

    gpio_pull_up(22);
    gpio_pull_up(21);

    gpio_set_irq_enabled(22, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(21, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_callback(&callback);

    printf("start test\n");

    datetime_t rtc_time_old;
    rtc_time_old.year = 2023;
    rtc_time_old.month = 11;
    rtc_time_old.day = 26;
    rtc_time_old.dotw = 0;
    rtc_time_old.hour = 0;
    rtc_time_old.min = 0;
    rtc_time_old.sec = 0;
    


    graph_point_t * temp_map_ref = Temp_Graph;
    TempSensor temp_sensor(ADC_CH1, temp_map_ref, GRAPH_LEN);

    if(TEMP_SENSOR_OK != temp_sensor.get_dtc())
    {
        printf("err\n");
        return 0;
    }

    temp_sensor.show_map();

    sleep_ms(100);

    TaskScheguler Tasks(2, 2);

    int arg2 = 20;

    printf("%d task 1 atach\n",Tasks.attach_task((TaskScheguler_Status_t *)&read_temp_task, &temp_sensor, REPETABLE, 0));
    printf("%d task 2 atach\n",Tasks.attach_task((TaskScheguler_Status_t *)&check_conditions, &arg2, REPETABLE, 0));
    printf("%d task 2 atach\n",Tasks.attach_task((TaskScheguler_Status_t *)&read_time_task, &rtc_time_old, REPETABLE, 0));

    sleep_ms(100);

    TaskScheguler_Status_t val;

    rtc_init();
    rtc_set_datetime(&rtc_time_old); 

    while(1)
    {
        val = Tasks.execute_tasks();
        arg2++;
        printf("return value is %d\n", val.done_execution);
        
        sleep_ms(100);

        printf("\n\n");
    }

    return 0;
}