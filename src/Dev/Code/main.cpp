#include <stdio.h>
#include <time.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"

#include "./TempSensor/TempSensor.hpp"

#define TEMPERATURE_UNIT 'C'
#define Rx 56000
#define VDD 5.0f
#define Vref 3.3f

#define __conv_fact__ (Vref / (1 << 12))
#define __sample_to_voltage__(a) (a*__conv_fact__)
#define __voltage_to_res__(a) ((a*Rx) / (VDD - a))
#define __get_temp__(rt, rt0, t0, rt1, t1) (t1 + (((rt - rt1) * (t1- t0))/(rt1 - rt0)))

#define temp_graph_len 8

typedef struct temp_graph_point_s
{
    float temp;
    float rt;
}
temp_graph_point_t;

temp_graph_point_t Temp_Graph[temp_graph_len] = 
{
    {100.0f, 185.0f},
    {70.0f, 450.0f},
    {38.0f, 1800.0f},
    {20.0f, 3400.0f},
    {4.0f, 7500.0f},
    {-7.0f, 13500.0f},
    {-18.0f, 25000.0f},
    {-40.0f, 100700.0f}
};

float read_RT(void)
{
    float RT = -1.0f;
    RT = __voltage_to_res__(__sample_to_voltage__((float)adc_read()));
    return RT;
}

#define ADC_Temp_GPIO 26
#define ADC_Temp_channel 0

#define led_pin 25
#define kl15 22
#define vss 21

#define irq_edge (GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL)

void init_adc(void)
{
    adc_init();
    adc_gpio_init(ADC_Temp_GPIO);
    adc_select_input(ADC_Temp_channel);
}

TempSensor Test((ADC_CH1 + ADC_OFFSET_GPIO));

void init_gpios(void)
{
    gpio_init(led_pin);
    gpio_init(kl15);
    gpio_init(vss);

    gpio_set_dir(led_pin, GPIO_OUT);
    gpio_set_dir(kl15, GPIO_IN);
    gpio_set_dir(vss, GPIO_IN);

    gpio_pull_up(kl15);
    gpio_pull_up(vss);
}

void kl15_irq_callback(uint gpio, uint32_t events)
{
    if(gpio_get(kl15))
        gpio_put(led_pin, true);
    else
        gpio_put(led_pin, false);

    printf("KL15 (ignition) was changed\n");
    return;
}

void vss_irq_callback(uint gpio, uint32_t events)
{
    printf("vss signal was changed %d\n", gpio_get(vss));
    return;
}

int main() {

    // Initialize LED pin
    init_gpios();

    //init usb connection for puty
    stdio_init_all();
    sleep_ms(10000);
    printf("Start Blinking\n");
    
    //init adc pins
    init_adc();

    gpio_set_irq_enabled_with_callback(
        kl15,
        irq_edge,
        true,
        &kl15_irq_callback);

    /*gpio_set_irq_enabled_with_callback(
        vss,
        irq_edge,
        true,
        &vss_irq_callback);
    */
    //loop
    while (true) {

        clock_t startTime = (clock_t) time_us_64();
        printf("startTime %d\n", startTime);

        float RT = read_RT();
        printf("resistance is %0.3f [ohm]\n", RT);

        float temp = 0.0f;
        for(unsigned int i = 0; i < temp_graph_len; i++)
        {
            if(RT < Temp_Graph[0].rt || RT > Temp_Graph[temp_graph_len-1].rt)
            {
                printf("Out of range\n");
                break;
            }

            if(RT >= Temp_Graph[i].rt && RT <= Temp_Graph[i+1].rt)
            {
                printf("interval %.2f %.2f\n",
                Temp_Graph[i].rt,
                Temp_Graph[i+1].rt);
                
                temp = __get_temp__(
                    RT, Temp_Graph[i].rt,
                    Temp_Graph[i].temp,
                    Temp_Graph[i+1].rt,
                    Temp_Graph[i+1].temp
                    );
                printf("Temp is %.2f [C]\n", temp);
                break;
            }
        }

        clock_t endTime = (clock_t) time_us_64();
        printf("endTime %d\n", endTime);

        double executionTime = ((double)(endTime - startTime)/10000) /(CLOCKS_PER_SEC);
        
        printf("%.8f sec\n", executionTime);

        sleep_ms(1000);
        printf("-----------------\n\n");
    }

    printf("Close");
}