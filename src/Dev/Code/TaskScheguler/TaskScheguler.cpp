#include "./TaskScheguler.hpp"

TaskScheguler::TaskScheguler(unsigned int nr_of_tasks, unsigned int * time_counter)
{
    if(!time_counter)
    {
        this->task_scheduler_dtc = __WRONG_TIME_REFERENCE__;
        return;
    }

    if(nr_of_tasks <= 1)
    {
        this->task_scheduler_dtc = __TASK_QUEUE_TO_LOW__;
        return;
    }

    this->task_queue_len = nr_of_tasks;
    this->functions_queue = (function *)malloc(sizeof(function) * this->task_queue_len);
    if(!this->functions_queue)
    {
        this->task_scheduler_dtc = __TASK_QUEUE_ALLOCATION_ERR__;
        return;
    }
    memset(this->functions_queue, 0, this->task_queue_len);

    this->args_queue = (void**)malloc(sizeof(void *) * this->task_queue_len);
    if(!this->args_queue)
    {
        this->task_scheduler_dtc = __ARG_QUEUE_ALLOCATION_ERR__;
        return;
    }
    memset(this->args_queue, 0, this->task_queue_len);

    return;
}

TaskScheguler::~TaskScheguler()
{
}

bool TaskScheguler::attach_task(TaskScheguler_Status_t * task_function, void * arguments)
{
    if(!task_function)
        return false;

    if(this->attach_index < this->task_queue_len)
    {
        this->functions_queue[this->attach_index] = (function)task_function;
        this->args_queue[this->attach_index] = arguments;
        this->attach_index++;
    }
    else
        return false;

    return true;
}

TaskScheguler_Status_t TaskScheguler::execute_task(unsigned int task_index)
{
    return this->functions_queue[task_index](this->args_queue[task_index]);
}


/* TO DO - TEST erase task function */

bool TaskScheguler::erase_task(unsigned int task_index)
{
    unsigned int local_task_index = task_index - 1;

    if(local_task_index >= this->attach_index)
    {
        return false;
    }

    if(local_task_index < (this->attach_index - 2))
    {
        memcpy( (this->functions_queue + local_task_index),
                (this->functions_queue + local_task_index + 1),
                (this->attach_index - local_task_index));

        memcpy( (this->args_queue + local_task_index),
                (this->args_queue + local_task_index + 1),
                (this->attach_index - local_task_index));

        this->functions_queue[this->attach_index - 1] = nullptr;
        this->args_queue[this->attach_index - 1] = nullptr;
        
        this->attach_index--;
    }
    else
    {
        this->functions_queue[local_task_index] = nullptr;
        this->args_queue[local_task_index] = nullptr;
        
        this->attach_index--;
    }

    return true;
}
