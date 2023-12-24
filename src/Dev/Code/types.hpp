#ifndef __TYPES__
#define __TYPES__

typedef struct task_counter_s
{
    unsigned int counter = 0;
    unsigned char task_index = 0;
}task_counter_t;

typedef struct app_dtc_conditions_s
{
    bool dtc_temp = false;
    bool dtc_kl15 = false;
    bool dtc_vss = false;
}app_dtc_conditions_t;

typedef struct modules_dtc_values_s
{
    unsigned int temp_dtc_value = 0;
    unsigned int gpio_dtc_value_debug = 0;
    unsigned int gpio_dtc_value_lamp = 0;
    unsigned int gpio_dtc_value_kl15 = 0;
    unsigned int gpio_dtc_value_vss = 0;
    unsigned int task_dtc_value = 0;
}modules_dtc_values_t;

#endif