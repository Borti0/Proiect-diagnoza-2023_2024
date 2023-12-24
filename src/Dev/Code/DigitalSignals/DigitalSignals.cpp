#include "./DigitalSignals.hpp"

DigitalSignal::DigitalSignal(unsigned int gpio_nr, bool direction, bool default_state, bool pull)
{
    this->gpio = gpio_nr;
    this->default_state = default_state;
    this->direction = direction;

    gpio_init(this->gpio);
    gpio_set_dir(this->gpio, this->direction);
    
    if(this->direction == IN)
    {
        this->set_pull_resistor(pull);
        bool current_state = gpio_get(this->gpio);
        if(current_state != this->default_state)
            this->gpio_dtc = __GPIO_NOT_DEFAULT__;
    }
    else
        gpio_put(this->gpio, this->default_state);

    return;
}

unsigned int DigitalSignal::get_dtc(void)
{
    return this->gpio_dtc;
}

void DigitalSignal::set_pull_resistor(bool pull)
{
    if(pull)
        gpio_pull_up(this->gpio);
    else
        gpio_pull_down(this->gpio);
    return;
}

bool DigitalSignal::drive_gpio(bool value)
{
    gpio_put(this->gpio, value);
    sleep_ms(1);
    if (value != this->read_value())
    {
        this->gpio_dtc = __GPIO_DRIVE_PROBLEM__;
        return false;
    }
    return true;
}

bool DigitalSignal::read_value(void)
{
    return gpio_get(this->gpio);
}

bool DigitalSignal::create_irq(unsigned int event, irq_handler_t function)
{
    if(this->direction != IN)
    {
        this->gpio_dtc = __GPIO_IRQ_ATTACH_ERR__;
        return false;
    }

    if(!function)
    {
        this->gpio_dtc = __GPIO_IRQ_ATTACH_ERR__;
        return false;
    }

    this->current_irq_function = function;
    this->event = event;
    
    gpio_set_irq_enabled(this->gpio, event, true);
    gpio_add_raw_irq_handler(this->gpio, function);
    irq_set_enabled(IO_IRQ_BANK0, true);

    return true;
}

bool DigitalSignal::disable_irq(void)
{
    if(this->event == 0)
    {
        this->gpio_dtc = __GPIO_IRQ_DISABLE_ERR__;
        return false;
    }
    irq_set_enabled(IO_IRQ_BANK0, false);
    gpio_set_irq_enabled(this->gpio, event, false);	
    irq_set_enabled(IO_IRQ_BANK0, true);
    return true;
}

bool DigitalSignal::remove_irq(void)
{
    if(!this->current_irq_function)
    {
        this->gpio_dtc = __GPIO_IRQ_REMOVE_ERR__;
        return false;
    }

    irq_set_enabled(IO_IRQ_BANK0, false);
    gpio_remove_raw_irq_handler(this->gpio, this->current_irq_function);
    irq_set_enabled(IO_IRQ_BANK0, true);
    
    this->current_irq_function = nullptr;
    return true;
}

DigitalSignal::~DigitalSignal()
{
}