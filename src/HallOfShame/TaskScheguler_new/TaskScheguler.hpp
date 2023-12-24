#ifndef __TASKSCHEDULER__
#define __TASKSCHEDULER__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pico/stdlib.h>

#define SOFTWARE_CLOCK_PERIOD 1
#define uint32_max 4294967295
/* An object which can take task functions 
   and execute them in order depending on 
   how often the time is incremented.
 */
#define __TASK_SCHEDULER_OK__           0x00
#define __WRONG_TIME_REFERENCE__        0x01
#define __TASK_QUEUE_ALLOCATION_ERR__   0x02
#define __ARG_QUEUE_ALLOCATION_ERR__    0x03
#define __TASK_PRIORITY_LEVELS_TO_LOW__ 0x04
#define __TASK_NR_FUNCTION_TO_LOW__     0x05
#define __TASK_FAILD_TO_CREATE_TIMER__  0x06
#define __TASK_EXEC_ERROR__             0x07
/*
 -> add task
 -> execute task
 -> erase task
 -> give priority to task
 -> execute tasks via an time reference (timer 1 ms) and priority
*/

#define REPETABLE    true
#define NONREPETABLE false
#define NO_IDLE_TASK -1

enum task_state {
    ready = 1,
    running = 2,
    waiting = 4,
    finish = 8,
    idle = 16
};

typedef struct TaskScheguler_Status_s
{
    bool done_execution;
}TaskScheguler_Status_t;

typedef struct TaskScheguler_Function_Status_s
{
    bool Repetable = false;
    unsigned char State = ready;
    TaskScheguler_Status_t * function(void *);

}TaskScheguler_Function_Status_t;

typedef struct current_task_s
{
    unsigned int priority = 0;
    unsigned int function_counter = 0;
}current_task_t;


class TaskScheguler
{
private:
    TaskScheguler_Function_Status_t ** functions_queue = nullptr;
    void *** args_queue = nullptr;
    unsigned int * attach_index = nullptr; //used for index reference

    unsigned int priority_lvls = 0;
    unsigned int functions_per_level = 0;

    unsigned int task_queue_len = 0;

    unsigned int task_scheduler_dtc = 0;

    volatile unsigned int time_counter = 0;
    volatile unsigned int * refereance_time_counter = nullptr; //used for clock reference

    current_task_t current_task_in_execution = {0,0};

    struct repeating_timer timer;
    bool software_clock_iqr(struct repeating_timer *t);

    unsigned int calc_dif_met1(unsigned int low, unsigned int high);
    unsigned int calc_dif_met2(unsigned int low, unsigned int high);

    unsigned int calc_dif[2] = {&this->calc_dif_met1, &this->calc_dif_met2};

    TaskScheguler_Status_t execute_task(unsigned int priority, unsigned int index);

    int check_for_empty_spaces(unsigned int priority_index);
public:
    TaskScheguler(unsigned int priority_levels, unsigned int function_number);

    ~TaskScheguler();

    bool attach_task(TaskScheguler_Status_t * task_function, void * arguments, bool repetable, unsigned int priority_lvl);

   
    TaskScheguler_Status_t execute_tasks(void);

    unsigned int get_dtc(void);
    
    bool busy_wait_ms(unsigned long miliseconds);

    void erase_task(unsigned int priority, unsigned int index);
};


#endif