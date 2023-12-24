#ifndef __DIGITALSIGNALS__
#define __DIGITALSIGNALS__

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define HIGH    true;
#define LOW     false

#define IN      GPIO_IN
#define OUT     GPIO_OUT

#define UP      true
#define DOWN    false

#define __GPIO_OK__                 0x00
#define __GPIO_NOT_DEFAULT__        0x01
#define __GPIO_DRIVE_PROBLEM__      0x02
#define __GPIO_IRQ_ATTACH_ERR__     0x03
#define __GPIO_IRQ_DISABLE_ERR__    0x04
#define __GPIO_IRQ_REMOVE_ERR__     0x05

class DigitalSignal
{
private:
    /* data */
    unsigned int gpio = 0;
    bool direction = IN;
    bool default_state = LOW;
    bool pull_resistance = UP;

    unsigned int gpio_dtc = __GPIO_OK__;

    void set_pull_resistor(bool pull);

    irq_handler_t current_irq_function = nullptr;
    unsigned int event = 0;

public:
    DigitalSignal(unsigned int gpio_nr, bool direction, bool default_state, bool pull);
    ~DigitalSignal();

    bool create_irq(unsigned int event, irq_handler_t function);
    bool disable_irq(void);
    bool remove_irq(void);

    unsigned int get_dtc(void);
    bool drive_gpio(bool value);
    bool read_value(void);
};


#endif