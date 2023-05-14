#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/iom8.h>

#ifndef __AVR_ATmega8__
#define __AVR_ATmega8__
#endif

#define MAX_TASKS (10)

#define RUNNABLE (0x00)
#define RUNNING  (0x01)
#define STOPPED  (0x02)
#define ERROR    (0x03)

typedef void (*task_t)(void);
typedef struct __tcb_t
{
    uint8_t id; // task ID
    task_t task; // pointer to the task
    // delay before execution
    uint16_t delay;
    uint16_t period;
    uint8_t status; // status of task
} tcb_t;

// the task list
tcb_t task_list[MAX_TASKS];

inline void io_init (void)__attribute__((always_inline));

void initScheduler(void);
void addTask(uint8_t, task_t, uint16_t);
void deleteTask(uint8_t);
uint8_t getTaskStatus(uint8_t);
void dispatchTasks(void);

void Task1(void);
void Task2(void);

ISR(TIMER0_OVF_vect, ISR_BLOCK)
{
	static int count = 0;

	count++;
	if (count == 392)
	{
		count = 0;
		// cycle through available tasks
		for (uint8_t i = 0; i < MAX_TASKS; i++)
		{
			if (task_list[i].status == RUNNABLE)
			{
				task_list[i].delay--;
			}
		}
	}
}

void main (void)
{
	io_init();

    while (1)
    {
   	 	 dispatchTasks();
    }
}

void io_init (void)
{
	// use 1/8 of system clock frequency
	 TCCR0 = 0x02;
	 // inital timer value = 0
	 TCNT0 = 0;
	 // enable Timer0 Overflow interrupt
	 TIMSK = _BV(TOIE0);
	 // set PORTD bit0 and bit1 as outputs
	 DDRD = _BV(PD0)|_BV(PD1);
}

// initialises the task list
void initScheduler(void)
{
	for (uint8_t i = 0; i < MAX_TASKS; i++)
	{
		task_list[i].id = 0;
		task_list[i].task = (task_t) 0x00;
		task_list[i].delay = 0;
		task_list[i].period = 0;
		task_list[i].status = STOPPED;
	}
}

// adds a new task to the task list
// scans through the list and
// places the new task data where
// it finds free space
void addTask(uint8_t id, task_t task, uint16_t period)
{
	uint8_t idx = 0;
	uint8_t done = 0;
	while (idx < MAX_TASKS)
	{
		if (task_list[idx].status == STOPPED)
		{
			task_list[idx].id = id;
			task_list[idx].task = task;
			task_list[idx].delay = period;
			task_list[idx].period = period;
			task_list[idx].status = RUNNABLE;
			done = 1;
		}
		if (done)
		{
			break;
		}
		idx++;
	}
}

// remove task from task list
// note STOPPED is equivalent
// to removing a task
void deleteTask(uint8_t id)
{
	for (uint8_t i = 0; i < MAX_TASKS; i++)
	{
		if (task_list[i].id == id)
		{
			task_list[i].status = STOPPED;
			break;
		}
	}
}

// gets the task status
// returns ERROR if id is invalid
uint8_t getTaskStatus(uint8_t id)
{
	for (uint8_t i = 0; i < MAX_TASKS; i++)
	{
		if (task_list[i].id == id)
			return task_list[i].status;
	}
	return ERROR;
}

// dispatches tasks when they are ready to run
void dispatchTasks(void)
{
	for (uint8_t i = 0; i < MAX_TASKS; i++)
	{
		// check for a valid task ready to run
		if (!task_list[i].delay && task_list[i].status == RUNNABLE)
		{
			// task is now running
			task_list[i].status = RUNNING;
			// call the task
			(*task_list[i].task)();

			// reset the delay
			task_list[i].delay = task_list[i].period;
			// task is runnable again
			task_list[i].status = RUNNABLE;
		}
	}
}

// Task definitions
void Task1(void)
{
	static uint8_t status = 1;
	if (status)
	{
		PORTD |= _BV(PD0);
	}
	else
	{
		PORTD &= ~_BV(PD0);
	}
	status = !status;
}


void Task2(void)
{
	static uint8_t status = 1;
	if (status)
	{
		PORTD |= _BV(PD1);
	}
	else
	{
		PORTD &= ~_BV(PD1);
	}
	status = !status;
}
