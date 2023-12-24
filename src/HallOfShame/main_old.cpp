#include <stdio.h>
#include <stdlib.h>

#include "./TempSensor/TempSensor.hpp"
#include "./ParmsBoard/Board.hpp"
#include "./TaskScheguler/TaskScheguler.hpp"
#include "./DigitalSignals/DigitalSignals.hpp"

#include "hardware/rtc.h"
#include "pico/util/datetime.h"

#define GRAPH_LEN 8
#define TASK_POOL_LEN 3

#define TASK_TIME_MS 100
#define DEBOUNCE_TIME_MS 100

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

volatile unsigned int counter = 0;
volatile unsigned int taskexec = 0;
bool timer_callback(struct repeating_timer *timer)
{
    counter++;
    if(!(counter % TASK_TIME_MS))
        taskexec++;

    if(taskexec == 3)
        taskexec = 0;

    return true;
}

volatile bool kl15_flag = false;
volatile bool can_read_kl15 = true;

volatile bool vss_flag = false;
volatile bool can_read_vss = true;

int64_t alarm_callback_vss(alarm_id_t id, void *user_data) {
    can_read_vss = true;
    return 0;
}

int64_t alarm_callback_kl15(alarm_id_t id, void *user_data) {
    can_read_kl15 = true;
    return 0;
}

void vss_irq_callback(void) {
    
    if(can_read_vss == false)
        return;
    
    if (gpio_get_irq_event_mask(VSS_GPIO) & (GPIO_IRQ_EDGE_RISE)) {
        gpio_acknowledge_irq(VSS_GPIO, GPIO_IRQ_EDGE_RISE);
        can_read_vss = false;
        add_alarm_in_ms(DEBOUNCE_TIME_MS, &alarm_callback_vss, nullptr, false);

        vss_flag = gpio_get(VSS_GPIO);
        return;
    }
    
    if (gpio_get_irq_event_mask(VSS_GPIO) & (GPIO_IRQ_EDGE_FALL)) {
        gpio_acknowledge_irq(VSS_GPIO, GPIO_IRQ_EDGE_FALL);
        can_read_vss = false;
        add_alarm_in_ms(DEBOUNCE_TIME_MS, &alarm_callback_vss, nullptr, false);

        vss_flag = gpio_get(VSS_GPIO);
        return;
    }

    return;
}

void kl_irq_callback(void) {

    if(can_read_kl15 == false)
        return;
    
    if ((gpio_get_irq_event_mask(KL15_GPIO) & (GPIO_IRQ_EDGE_RISE))) {
        gpio_acknowledge_irq(KL15_GPIO, GPIO_IRQ_EDGE_RISE);
        can_read_kl15 = false;
        add_alarm_in_ms(DEBOUNCE_TIME_MS, &alarm_callback_kl15, nullptr, false);

        kl15_flag = gpio_get(KL15_GPIO);
        return;
    }

    if (gpio_get_irq_event_mask(KL15_GPIO) & (GPIO_IRQ_EDGE_FALL)) {
        gpio_acknowledge_irq(KL15_GPIO, GPIO_IRQ_EDGE_FALL);
        can_read_kl15 = false;
        add_alarm_in_ms(DEBOUNCE_TIME_MS, &alarm_callback_kl15, nullptr, false);

        kl15_flag = gpio_get(KL15_GPIO);
        return;
    }

    return;
}

int main(void)
{
    // TO DO
    // -> Change Taks to check project's conditions.
    stdio_init_all();

    // TO DO
    // -> split gpios object in two object (gpio_in and gpio_out) which inherate an base gpio object.
    DigitalSignal debug_led(DEBUG_GPIO, GPIO_OUT, LOW, false);
    DigitalSignal dtc_lamp(DTC_GPIO, GPIO_OUT, LOW, false);

    DigitalSignal kl15(KL15_GPIO, GPIO_IN, LOW, true);
    DigitalSignal vss(VSS_GPIO, GPIO_IN, LOW, true);


    //TO DO
    // -> define more clear all init DTC of modules and use to check if the init state was executet as expected.
    // -> If no block the main loop and 
    if(kl15.get_dtc() != __GPIO_OK__ || vss.get_dtc() != __GPIO_OK__)
    {
        printf("error gpio");
        return 0;
    }


    //TO DO:
    // -> Check if the raw gpio irqs can receive arguments via pointer
    kl15.create_irq(GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, &kl_irq_callback);
    vss.create_irq(GPIO_IRQ_EDGE_RISE  | GPIO_IRQ_EDGE_FALL, &vss_irq_callback);

    sleep_ms(10000);

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

    
    TaskScheguler Tasks(TASK_POOL_LEN);

    int arg2 = 20;

    printf("%d task 1 atach\n",Tasks.attach_task((TaskScheguler_Status_t *)&read_temp_task, &temp_sensor));
    printf("%d task 2 atach\n",Tasks.attach_task((TaskScheguler_Status_t *)&check_conditions, &arg2));
    printf("%d task 2 atach\n",Tasks.attach_task((TaskScheguler_Status_t *)&read_time_task, &rtc_time_old));

    sleep_ms(1000);

    struct repeating_timer timer;

    //TO DO:
    //-> timer interupts can pass arguments via void * user_data from strunct repeating_timer or from an simple void *
    add_repeating_timer_ms(1, timer_callback, NULL, &timer);

    TaskScheguler_Status_t val;

    rtc_init();
    rtc_set_datetime(&rtc_time_old); 

    bool drive_value = true;
    debug_led.drive_gpio(drive_value);

halt:
    while(!kl15_flag)
    {   
        printf("%d\n", kl15_flag);
        sleep_ms(100);
    }

    while(1)
    {
        val = Tasks.execute_task(taskexec);
        arg2++;
        printf("return value is %d\n", val.done_execution);
        
        sleep_ms(1000);
        debug_led.drive_gpio(drive_value);
        dtc_lamp.drive_gpio(!drive_value);
        drive_value = !drive_value;

        printf("KL15 %d vss %d\n", kl15_flag, vss_flag);
        printf("\n\n");

        if(!kl15_flag)
            goto halt;
    }

    return 0;
}