#include "./TaskScheguler.hpp"

TaskScheguler::TaskScheguler(unsigned int prioritys_levels, unsigned int function_number)
{
    /** arguments **/
    if(priority_lvls < 1)
    {
        this->task_scheduler_dtc = __TASK_PRIORITY_LEVELS_TO_LOW__;
        return;
    }

    if(function_number < 1)
    {
        this->task_scheduler_dtc = __TASK_NR_FUNCTION_TO_LOW__;
    }

    this->priority_lvls = prioritys_levels;
    this->functions_per_level = function_number;

    this->task_queue_len = (this->priority_lvls * this->functions_per_level);

    /** memory allocation task matrix **/
    this->functions_queue = 
    (TaskScheguler_Function_Status_t **)malloc(sizeof(TaskScheguler_Function_Status_t *) * this->priority_lvls);

    if(!this->functions_queue)
    {
        this->task_scheduler_dtc = __TASK_QUEUE_ALLOCATION_ERR__;
        return;
    }

    for(unsigned int i = 0; this->functions_per_level; i++){
        this->functions_queue[i] =
        (TaskScheguler_Function_Status_t *)malloc(sizeof(TaskScheguler_Function_Status_t) * this->functions_per_level);
        if(!this->functions_queue[i])
        {
            this->task_scheduler_dtc = __TASK_QUEUE_ALLOCATION_ERR__;
            return;
        }
    }

    /** memory allocation for index attach of all priority levels **/
    this->attach_index = (unsigned int *)malloc(sizeof(unsigned int) * this->priority_lvls);

    /** memory allocation for args matrix **/
    this->args_queue = (void***)malloc(sizeof(void **) * this->priority_lvls);

    if(!this->args_queue)
    {
        this->task_scheduler_dtc = __ARG_QUEUE_ALLOCATION_ERR__;
        return;
    }

    for(unsigned int i = 0; i < this->functions_per_level; i++)
    {
        this->args_queue[i] = (void **)malloc(sizeof(void*) * this->functions_per_level);
        if(!this->args_queue[i])
        {
            this->task_scheduler_dtc = __TASK_QUEUE_ALLOCATION_ERR__;
            return;
        }
    }

    for(unsigned int i = 0; i < this->priority_lvls; i ++)
    {
        for(unsigned int j =0; j < this->functions_per_level; j++)
        {
            this->args_queue[i][j] = nullptr;
        }
    }

    this->refereance_time_counter = &this->time_counter;

    //create clock signal for task scheguler
    if(!add_repeating_timer_ms(SOFTWARE_CLOCK_PERIOD, this->software_clock_iqr, nullptr, &this->timer))
    {
        this->task_scheduler_dtc = __TASK_FAILD_TO_CREATE_TIMER__;
        return;
    }

    return;
}

bool TaskScheguler::software_clock_iqr(struct repeating_timer *t)
{
    printf("generate clock");
    *this->refereance_time_counter += 1;
    return true;
}

bool TaskScheguler::attach_task(TaskScheguler_Status_t * task_function, void * arguments, bool repetable, unsigned int priority_lvl)
{
    if(!task_function)
        return false;

    if(priority_lvl < this->priority_lvls)
        return false;

    int index = this->check_for_empty_spaces(priority_lvl);
    if(index != NO_IDLE_TASK)
    {
        this->functions_queue[priority_lvl][this->attach_index[priority_lvl]].Repetable = repetable;
        this->functions_queue[priority_lvl][this->attach_index[priority_lvl]].State = ready;
        this->functions_queue[priority_lvl][this->attach_index[priority_lvl]].function = task_function;

        this->args_queue[priority_lvl][this->attach_index[priority_lvl]] = arguments;
        return true;
    }

    if(this->attach_index[priority_lvl] < this->functions_per_level)
    {
        this->functions_queue[priority_lvl][this->attach_index[priority_lvl]].Repetable = repetable;
        this->functions_queue[priority_lvl][this->attach_index[priority_lvl]].State = ready;
        this->functions_queue[priority_lvl][this->attach_index[priority_lvl]].function = task_function;

        this->args_queue[priority_lvl][this->attach_index[priority_lvl]] = arguments;
        this->attach_index[priority_lvl]++;
    }
    else
        return false;

    return true;
}

unsigned int TaskScheguler::get_dtc(void)
{
    return this->task_scheduler_dtc;
}

bool TaskScheguler::busy_wait_ms(unsigned int miliseconds)
{
    TaskScheguler_Status_t val;

    this->functions_queue[this->current_task_in_execution.priority][this->current_task_in_execution.function_counter].State = waiting;

    unsigned int local_task_scheguler_counter_old = *this->refereance_time_counter;
    unsigned int milis;
    
    current_task_t task_in_wait;
    task_in_wait.priority = this->current_task_in_execution.priority;
    task_in_wait.function_counter = this->current_task_in_execution.function_counter;

    bool finish_wait = false;

    unsigned int local_priority = 0;
    unsigned int local_function_counter = 0;

    while(!finish_wait)
    {
        if(this->functions_queue[local_priority][local_function_counter] != waiting)
        {
            val = this->execute_task(local_priority, local_function_counter);
            if(val.done_execution)
                this->task_scheduler_dtc = __TASK_EXEC_ERROR__;
        }

        local_function_counter++;

        if(local_function_counter == this->attach_index[local_priority])
        {
            local_function_counter=0;
            local_priority++;
        }

        if(local_priority == this->priority_lvls)
            local_priority = 0;

        milis = this->calc_dif[
            local_task_scheguler_counter_old < *this->refereance_time_counter
        ](local_task_scheguler_counter_old, *this->refereance_time_counter);

        if(milis > miliseconds)
            finish_wait = true;
        
    }
    
    this->current_task_in_execution.priority = task_in_wait.priority;
    this->current_task_in_execution.function_counter = task_in_wait.function_counter;

    this->functions_queue[this->current_task_in_execution.priority][this->current_task_in_execution.function_counter].State = running;

    return true;
}

TaskScheguler_Status_t TaskScheguler::execute_task(unsigned int priority, unsigned int index)
{
    return this->functions_queue[priority][index].function(this->args_queue[priority][index]);
}

unsigned int TaskScheguler::calc_dif_met1(unsigned int low, unsigned int high)
{
    return (high - low);
}

unsigned int TaskScheguler::calc_dif_met2(unsigned int low, unsigned int high)
{
    return (uint32_max - low) + high;
}

int TaskScheguler::check_for_empty_spaces(unsigned int priority_index)
{
    for(unsigned int index = 0; index < this->attach_index[priority_index]; index)
    {
        if(this->functions_queue[priority_index][index].State == idle)
            return index;
    }

    return -1;
}

TaskScheguler_Status_t TaskScheguler::execute_tasks(void)
{
    TaskScheguler_Status_t val;
    unsigned int prio = this->current_task_in_execution.priority;
    unsigned int f_counter = this->current_task_in_execution.function_counter;

    this->functions_queue[prio][f_counter].State = running;

    void * arg = (void*)this->args_queue[prio][f_counter];
    val = this->functions_queue[prio][f_counter].function(arg);

    this->functions_queue[prio][f_counter].State = ready;

    if(!this->functions_queue[prio][f_counter].Repetable)
        this->erase_task(prio, f_counter);

    return val;
}

TaskScheguler::~TaskScheguler()
{
}


/* TO DO - TEST erase task function */

void TaskScheguler::erase_task(unsigned int priority, unsigned int index)
{
    this->functions_queue[priority][index].State = idle;
    this->functions_queue[priority][index].function() = nullptr;
    this->args_queue[priority][index] = nullptr;
}


