#ifndef __TASKSCHEDULER__
#define __TASKSCHEDULER__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* An object which can take task functions 
   and execute them in order depending on 
   how often the time is incremented.
 */
#define __TASK_SCHEDULER_OK__         0x00
#define __WRONG_TIME_REFERENCE__      0x01
#define __TASK_QUEUE_ALLOCATION_ERR__ 0x02
#define __ARG_QUEUE_ALLOCATION_ERR__  0x03
#define __TASK_QUEUE_TO_LOW__         0x04


typedef struct TaskScheguler_Status_s
{
    bool done_execution;
}TaskScheguler_Status_t;

typedef TaskScheguler_Status_t (*function)(void *);

/*
 -> add task
 -> execute task
 -> erase task
 -> give priority to task
 -> execute tasks via an time reference (timer 1 ms) and priority
*/


class TaskScheguler
{
private:
    function * functions_queue = nullptr;
    void ** args_queue = nullptr;

    unsigned int task_queue_len = 0;
    unsigned int * time_counter = nullptr;

    unsigned int task_scheduler_dtc = 0;
    unsigned int attach_index = 0;

public:
    TaskScheguler(unsigned int nr_of_tasks, unsigned int * time_counter);
    ~TaskScheguler();

    bool attach_task(TaskScheguler_Status_t * task_function, void * arguments);

    TaskScheguler_Status_t execute_task(unsigned int task_index);

    bool erase_task(unsigned int task_index);
};


#endif