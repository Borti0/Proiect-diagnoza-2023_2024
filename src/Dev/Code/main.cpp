#include <stdio.h>
#include <stdlib.h>

#include "./TempSensor/TempSensor.hpp"
#include "./ParmsBoard/Board.hpp"
#include "./TaskScheguler/TaskScheguler.hpp"
#include "./DigitalSignals/DigitalSignals.hpp"
#include "./types.hpp"

#include "hardware/rtc.h"
#include "pico/util/datetime.h"

#define GRAPH_LEN 8

#define TASK_POOL_LEN 3
#define TASK_TIME_MS 100
#define DEBOUNCE_TIME_MS 100

#define TEMP_S_LOW 12
#define TEMP_LOW_LIM -35
#define KL15_MIN_HIGH 4

#define __MIN_IN_SEC__  60
#define __H_IN_SEC__    3600

//#define __DEBUG__ true
//#define __DEBUG_DTC_RUNNING__ true


typedef struct temp_task_args_s
{
    unsigned char        * ref_sensor_time  = nullptr; //sorage for time value
    datetime_t           * ref_rtc_sensor   = nullptr; //as an last read
    app_dtc_conditions_t * ref_dtc_app      = nullptr;
    TempSensor           * ref_sensor       = nullptr;
}temp_task_args_t;

typedef struct time_task_args_s
{
    datetime_t              * ref_rtc_kl15  = nullptr;
    app_dtc_conditions_t    * ref_dtc_app   = nullptr;
}time_task_args_t;

typedef struct dtc_check_task_args_s
{
    app_dtc_conditions_t    * ref_dtc_app   = nullptr;
    DigitalSignal           * lamp_ref      = nullptr;
}dtc_check_task_args_t;


TaskScheguler_Status_t read_temp_task(void * params);
//check time for kl15 and signal for vss
TaskScheguler_Status_t read_time_task(void * params);
TaskScheguler_Status_t check_DTC_task(void * params);

volatile bool kl15_flag     = false;
volatile bool vss_flag      = false;
volatile bool kl15_can_read = true;
volatile bool vss_can_read  = true;
void vss_irq_callback(void);
void kl15_irq_callback(void);

int64_t vss_alarm_callback(alarm_id_t id, void *user_data);
int64_t kl15_alarm_callback(alarm_id_t id, void *user_data);

bool timer_callback(struct repeating_timer *timer);

int main(void)
{

    /* stdio init to unlock serial monitor */
    stdio_init_all();

#ifdef __DEBUG__
    sleep_ms(5000);
    printf(" ... START BOOTING ...\n");
#endif 


    /* init dtc varables */
    app_dtc_conditions_t app_conditons;
    modules_dtc_values_t dtc_modules;
    bool return_check = false;

    graph_point_t Sensor_Graph[] = {
        {100.0f, 185.0f},
        {70.0f, 450.0f},
        {38.0f, 1800.0f},
        {20.0f, 3400.0f},
        {4.0f, 7500.0f},
        {-7.0f, 13500.0f},
        {-18.0f, 25000.0f},
        {-40.0f, 100700.0f}
    };
    graph_point_t * sensor_graph_ref = Sensor_Graph;

    datetime_t rtc_general;
    rtc_general.year    = 2024;
    rtc_general.month   = 1;
    rtc_general.day     = 1;
    rtc_general.dotw    = 0;
    rtc_general.hour    = 0;
    rtc_general.min     = 0;
    rtc_general.sec     = 0;

    datetime_t rtc_temp_sensor = {0};
    datetime_t rtc_kl15 = {0};

    /* gpio init with dtc check */
    DigitalSignal gpio_debug_led(DEBUG_GPIO, GPIO_OUT, LOW, false);
    dtc_modules.gpio_dtc_value_debug = gpio_debug_led.get_dtc();

    DigitalSignal gpio_dtc_lamp(DTC_GPIO, GPIO_OUT, LOW, false);
    dtc_modules.gpio_dtc_value_lamp = gpio_dtc_lamp.get_dtc();

    DigitalSignal gpio_kl15(KL15_GPIO, GPIO_IN, LOW, true);
    dtc_modules.gpio_dtc_value_kl15 = gpio_kl15.get_dtc();

    DigitalSignal gpio_vss(VSS_GPIO, GPIO_IN, LOW, true);
    dtc_modules.gpio_dtc_value_vss = gpio_vss.get_dtc();

    /* gpio attach irqs */
    return_check = gpio_kl15.create_irq(
        GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
        &kl15_irq_callback
    );
    if(!return_check)
        dtc_modules.gpio_dtc_value_kl15 = gpio_kl15.get_dtc();

    return_check = gpio_vss.create_irq(
        GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
        &vss_irq_callback
    );
    if(!return_check)
        dtc_modules.gpio_dtc_value_vss = gpio_vss.get_dtc();
        
    /* init sensor temp with dtc checks */
    TempSensor temp_sensor(ADC_CH1, sensor_graph_ref, GRAPH_LEN);
    dtc_modules.temp_dtc_value = temp_sensor.get_dtc();

    /* init task scheguler with dtc check */
    TaskScheguler Tasks(TASK_POOL_LEN);
    dtc_modules.task_dtc_value = Tasks.get_dtc();

    /* prepare argumentsfor tasks */
    /*
        temp sensor for temp read task + rtc_timer + temp dtc condition referance
        rtc for time read task + dtc condition sensors
        dtc conditions referance + lamp reference for check condition task
    */
    unsigned char sensor_time = 0;
    temp_task_args_t args_temp_task;
    args_temp_task.ref_sensor_time  = &sensor_time;
    args_temp_task.ref_sensor       = &temp_sensor;
    args_temp_task.ref_dtc_app      = &app_conditons;
    args_temp_task.ref_rtc_sensor   = &rtc_temp_sensor;

    time_task_args_t args_time_task;
    args_time_task.ref_dtc_app  = &app_conditons;
    args_time_task.ref_rtc_kl15 = &rtc_kl15;
    

    dtc_check_task_args_t args_check_task;
    args_check_task.ref_dtc_app = &app_conditons;
    args_check_task.lamp_ref    = &gpio_dtc_lamp;

    /* attach tasks and argumants with dtc checks */
    return_check = false;
    return_check = Tasks.attach_task((TaskScheguler_Status_t *)&read_temp_task, &args_temp_task);
    if(!return_check)
        dtc_modules.task_dtc_value = Tasks.get_dtc();

    return_check = Tasks.attach_task((TaskScheguler_Status_t *)&read_time_task, &args_time_task);
    if(!return_check)
        dtc_modules.task_dtc_value = Tasks.get_dtc();

    return_check = Tasks.attach_task((TaskScheguler_Status_t *)&check_DTC_task, &args_check_task);
    if(!return_check)
        dtc_modules.task_dtc_value = Tasks.get_dtc();
    


    #ifdef __DEBUG__
        sleep_ms(1000);
    #endif
    /* drive debug led */
    bool value_gpio_debugg = true;
    if(!gpio_debug_led.drive_gpio(value_gpio_debugg))
        dtc_modules.gpio_dtc_value_debug = gpio_debug_led.get_dtc();

    /* print all modules dtc and sensor map*/
#ifdef __DEBUG__ 
    printf(">> MODULES DTC AT BOOTING: \n");
    printf("\t-> GPIO Debug DTC: %x\n", dtc_modules.gpio_dtc_value_debug);
    printf("\t-> GPIO KL15  DTC: %x\n", dtc_modules.gpio_dtc_value_kl15);
    printf("\t-> GPIO VSS   DTC: %x\n", dtc_modules.gpio_dtc_value_vss);
    printf("\t-> GPIO LAMP  DTC: %x\n", dtc_modules.gpio_dtc_value_lamp);
    printf("\t-> TEMP SENS  DTC: %x\n", dtc_modules.temp_dtc_value);
    printf("\t-> TASK SCH   DTC: %x\n", dtc_modules.task_dtc_value);

    printf("\n\n\n");

    printf(">> USED SENSOR MAP\n");
    temp_sensor.show_map();
    printf("\n\n");
#endif

    struct repeating_timer timer_data;
    volatile task_counter_t _taks_index_;

    TaskScheguler_Status_t task_return_value = {0};

    /* init rtc module */
    rtc_init();
    
    /* wait for kl15 high */
halt:
    while(!kl15_flag)
    {
        sleep_ms(100);
    }

    /* set data app */
    rtc_set_datetime(&rtc_general);

    /* add task scheguler clock source */
    add_repeating_timer_ms(1, timer_callback, (void *)&_taks_index_, &timer_data);

    /* while
        execute task
        check returned value
        drive debug led
        check kl15 value
            clear all thread_rtcs
            erase task scheguler clock source
            goto halt
    */
    
    while (true)
    {

        #ifdef __DEBUG__
            printf("--- MAIN IS RUNNING ---\n");
            #ifdef __DEBUG_DTC_RUNNING__
                printf(">> MODULES DTC AT RUNNING: \n");
                printf("\t-> GPIO Debug DTC: %x\n", dtc_modules.gpio_dtc_value_debug);
                printf("\t-> GPIO KL15  DTC: %x\n", dtc_modules.gpio_dtc_value_kl15);
                printf("\t-> GPIO VSS   DTC: %x\n", dtc_modules.gpio_dtc_value_vss);
                printf("\t-> GPIO LAMP  DTC: %x\n", dtc_modules.gpio_dtc_value_lamp);
                printf("\t-> TEMP SENS  DTC: %x\n", dtc_modules.temp_dtc_value);
                printf("\t-> TASK SCH   DTC: %x\n", dtc_modules.task_dtc_value);
            #endif
            sleep_ms(500);
        #endif

        #ifndef __DEBUG__
            sleep_ms(50);
        #endif


        task_return_value = Tasks.execute_task(_taks_index_.task_index);
        if(task_return_value.done_execution != true)
        {
            printf("Error to execute task %d\n", _taks_index_.task_index);
            dtc_modules.task_dtc_value = Tasks.get_dtc();
            //return 1;
        }
        value_gpio_debugg = !value_gpio_debugg; 
        if(!gpio_debug_led.drive_gpio(value_gpio_debugg))
            dtc_modules.gpio_dtc_value_debug = gpio_debug_led.get_dtc();
        if(!kl15_flag)
        {
            rtc_temp_sensor = {0};
            rtc_kl15 = {0};
            cancel_repeating_timer(&timer_data);
            goto halt;
        }
    }
   
   return 0;
}

TaskScheguler_Status_t read_temp_task(void *args)
{
    TaskScheguler_Status_t ret_value;
    ret_value.done_execution = true;
    
    datetime_t curent_time = {0};
    unsigned delta_time = 0;

    temp_task_args_t * local_arguments = (temp_task_args_t *)args;
    /* read temp*/
    float temp_C = local_arguments->ref_sensor->ReadTempAsCelsius();
    
    rtc_get_datetime(&curent_time);

    if(curent_time.sec < local_arguments->ref_rtc_sensor->sec)
        delta_time = (60 - local_arguments->ref_rtc_sensor->sec) + curent_time.sec;
    else    
    {
        delta_time = (curent_time.sec - local_arguments->ref_rtc_sensor->sec);
        if (curent_time.min != local_arguments->ref_rtc_sensor->min)
            delta_time += __MIN_IN_SEC__;
        if (curent_time.hour != local_arguments->ref_rtc_sensor->hour)
            delta_time += __H_IN_SEC__;
    }


    /* check temp value*/
    if(temp_C < TEMP_LOW_LIM)
    {
        *(local_arguments->ref_sensor_time) = *(local_arguments->ref_sensor_time) + delta_time;
    }
    else
    {
        /* Reseting last read fo the time */
        // local_arguments->ref_rtc_sensor->hour   = 0;
        // local_arguments->ref_rtc_sensor->min    = 0;
        // local_arguments->ref_rtc_sensor->sec    = 0;
        /* Reseting sensor time */
        *(local_arguments->ref_sensor_time)     = 0;
    }

    if(*(local_arguments->ref_sensor_time) >= TEMP_S_LOW)
        local_arguments->ref_dtc_app->dtc_temp = true;
    else
        local_arguments->ref_dtc_app->dtc_temp = false;

#ifdef __DEBUG__
    printf("Read Temperature: %.2f %d\n", temp_C, local_arguments->ref_dtc_app->dtc_temp);
    printf("D_Time value:  %d [s]\n", delta_time);
    printf("Time value:    %d [s]\n", *(local_arguments->ref_sensor_time));
    printf("CS Time value: %d [s]\n", local_arguments->ref_rtc_sensor->sec);
#endif
    
    *(local_arguments->ref_rtc_sensor) = curent_time;

    return ret_value;
}

TaskScheguler_Status_t read_time_task(void *args)
{
    TaskScheguler_Status_t ret_value;
    ret_value.done_execution = true;

    time_task_args_t * local_arguments = (time_task_args_t *)args;

    /* check kl15 flag condition  */
    /* read time */
    if(kl15_flag)
    {
        rtc_get_datetime(local_arguments->ref_rtc_kl15);
    }
    else
    {
        local_arguments->ref_rtc_kl15->hour = 0;
        local_arguments->ref_rtc_kl15->min  = 0;
        local_arguments->ref_rtc_kl15->sec  = 0;
    }

    /* change condition */
    if(local_arguments->ref_rtc_kl15->min >= KL15_MIN_HIGH)
        local_arguments->ref_dtc_app->dtc_kl15 = true;
    else
        local_arguments->ref_dtc_app->dtc_kl15 = false;

    /* read condition flag vss */
    if(!vss_flag)
        local_arguments->ref_dtc_app->dtc_vss = true;
    else
        local_arguments->ref_dtc_app->dtc_vss = false;

#ifdef __DEBUG__
    printf("Switch DTC Task 2: VSS %d KL15 %d min %d:%d\n",
        local_arguments->ref_dtc_app->dtc_vss, local_arguments->ref_dtc_app->dtc_kl15,
        local_arguments->ref_rtc_kl15->min,
        local_arguments->ref_rtc_kl15->sec);
#endif
    return ret_value;
}

TaskScheguler_Status_t check_DTC_task(void *args)
{
    TaskScheguler_Status_t ret_value;
    ret_value.done_execution = true;
    
    dtc_check_task_args_t * local_arguments = (dtc_check_task_args_t *)args;

#ifdef __DEBUG__
    printf("LAMP THREAD CONDITIONS:");
    printf(">>Condition sensor %d\n", local_arguments->ref_dtc_app->dtc_temp);
    printf(">>Condition KL15   %d\n", local_arguments->ref_dtc_app->dtc_kl15);
    printf(">>Condition VSS    %d\n", local_arguments->ref_dtc_app->dtc_vss);
#endif

    /* check conditions */
    /* drive led */
    if (local_arguments->ref_dtc_app->dtc_kl15 && 
        local_arguments->ref_dtc_app->dtc_temp &&
        local_arguments->ref_dtc_app->dtc_vss)

        local_arguments->lamp_ref->drive_gpio(true);
    else
        local_arguments->lamp_ref->drive_gpio(false);

    return ret_value;
}

bool timer_callback(struct repeating_timer *timer)
{
    task_counter_t * local_task_counters_ref = (task_counter_t *)timer->user_data;
    
    local_task_counters_ref->counter++;
    
    if(!(local_task_counters_ref->counter % TASK_TIME_MS))
        local_task_counters_ref->task_index++;
    
    if(local_task_counters_ref->task_index == TASK_POOL_LEN)
        local_task_counters_ref->task_index = 0;

    return true;
}

int64_t vss_alarm_callback(alarm_id_t id, void *user_data) {
    vss_can_read = true;
    return 0;
}

int64_t kl15_alarm_callback(alarm_id_t id, void *user_data) {
    kl15_can_read = true;
    return 0;
}

void vss_irq_callback(void)
{
    if(vss_can_read == false)
        return;
    
    if (gpio_get_irq_event_mask(VSS_GPIO) & (GPIO_IRQ_EDGE_RISE)) {
        gpio_acknowledge_irq(VSS_GPIO, GPIO_IRQ_EDGE_RISE);
        vss_can_read = false;
        add_alarm_in_ms(DEBOUNCE_TIME_MS, &vss_alarm_callback, nullptr, false);

        vss_flag = gpio_get(VSS_GPIO);
        return;
    }
    
    if (gpio_get_irq_event_mask(VSS_GPIO) & (GPIO_IRQ_EDGE_FALL)) {
        gpio_acknowledge_irq(VSS_GPIO, GPIO_IRQ_EDGE_FALL);
        vss_can_read = false;
        add_alarm_in_ms(DEBOUNCE_TIME_MS, &vss_alarm_callback, nullptr, false);

        vss_flag = gpio_get(VSS_GPIO);
        return;
    }

    return;
}

void kl15_irq_callback(void)
{
    if(kl15_can_read == false)
        return;
    
    if ((gpio_get_irq_event_mask(KL15_GPIO) & (GPIO_IRQ_EDGE_RISE))) {
        gpio_acknowledge_irq(KL15_GPIO, GPIO_IRQ_EDGE_RISE);
        kl15_can_read = false;
        add_alarm_in_ms(DEBOUNCE_TIME_MS, &kl15_alarm_callback, nullptr, false);

        kl15_flag = gpio_get(KL15_GPIO);
        return;
    }

    if (gpio_get_irq_event_mask(KL15_GPIO) & (GPIO_IRQ_EDGE_FALL)) {
        gpio_acknowledge_irq(KL15_GPIO, GPIO_IRQ_EDGE_FALL);
        kl15_can_read = false;
        add_alarm_in_ms(DEBOUNCE_TIME_MS, &kl15_alarm_callback, nullptr, false);

        kl15_flag = gpio_get(KL15_GPIO);
        return;
    }

    return;
}
