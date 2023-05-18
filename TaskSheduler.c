#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/iom8.h>
#include <stdlib.h>

#define BAUD 38400
#define F_CPU 8000000
#include <util/delay.h>
#include <util/setbaud.h>

#ifndef __AVR_ATmega8__
#define __AVR_ATmega8__
#endif

#define MAX_TASKS (4)

#define RUNNABLE (0x00)
#define RUNNING  (0x01)
#define STOPPED  (0x02)
#define ERROR    (0x03)

#define TICK 1 //one tick equals to 32.768 ms

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

char* buff;

inline void io_init (void)__attribute__((always_inline));

//UART INITIALIZATION
static void set_uart_baud(void);
static void init_uart(void);
static void put_char_to_udr(char data_byte);
void send_message_to_UDR(char * message, int integer);
void init_integer_buff(void);
//

void initScheduler(void);
void addTask(uint8_t, task_t, uint16_t);
void deleteTask(uint8_t);
uint8_t getTaskStatus(uint8_t);
void dispatchTasks(void);

void Task1(void);
void Task2(void);
void Task3(void);
void Task4(void);
void Task5(void);

ISR(TIMER0_OVF_vect, ISR_BLOCK)
{
	// cycle through available tasks
	for (uint8_t i = 0; i < MAX_TASKS; i++)
	{
		if (task_list[i].status == RUNNABLE)
		{
			task_list[i].delay--;
		}
	}
}

void main (void)
{


	//UART
	init_uart();
	init_integer_buff();

	io_init();
	initScheduler();

	addTask(0, Task1, TICK*10);
	addTask(1, Task2, TICK*30);
	addTask(2, Task3, TICK*40);
	addTask(3, Task4, TICK*50);

	sei();

    while (1)
    {
   	 	dispatchTasks();
   	 	//put_char_to_udr('1');
    }
}

void io_init (void)
{
	// P = 1024 * 256 / 8 000 000 = 0,032768 s
	 TCCR0 = 0x05;//consider how to calculate value automatically
	 // inital timer value = 0
	 TCNT0 = 0;
	 // enable Timer0 Overflow interrupt
	 TIMSK = _BV(TOIE0);
	 // set PORTD bit0 and bit1 as outputs
	 //DDRD = _BV(PD0) | _BV(PD1);
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
		{
			return task_list[i].status;
		}
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
	send_message_to_UDR("T1 T2d", task_list[1].delay);
}


void Task2(void)
{
	send_message_to_UDR("T2 T1d", task_list[0].delay);
}

void Task3(void)
{
	send_message_to_UDR("T3 T4d", task_list[3].delay);
}

void Task4(void)
{
	send_message_to_UDR("T4 T3d", task_list[2].delay);
}

//UART
static void set_uart_baud(void)
{
   UBRRH = UBRRH_VALUE;
   UBRRL = UBRRL_VALUE;
   #if USE_2X
   UCSRA |= (1 << U2X);
   #else
   UCSRA &= ~(1 << U2X);
   #endif
}

static void init_uart(void)
{
   /* Enable receiver and transmitter */
   UCSRB = (1<<RXEN)|(1<<TXEN);
   /* Set frame format: 8data, 1stop bit */
   UCSRC = (1<<URSEL)|(1<<USBS)|(1<<UCSZ1)|(1<<UCSZ0);
   set_uart_baud();
}

static void put_char_to_udr(char data_byte)
{
   while(!(UCSRA & (1 << UDRE)));
   {
      //??UCSRA |= (1 << UDRE);
      UDR = data_byte;
   }
}
void send_message_to_UDR(char * message, int integer)
{
   itoa(integer, buff,10);
   do
   {
     put_char_to_udr(*message);
   }while(*++message);
   do
   {
     put_char_to_udr(*buff);
   }while(*++buff);
   put_char_to_udr('\n');
   put_char_to_udr('\r');
}

void init_integer_buff(void)
{
	buff = (char*) malloc(sizeof(int8_t)*8+1);
	if(buff == ((void *)0))
	{
		char* error_message = "Malloc err.\n\r";
		do
		{
			put_char_to_udr(*error_message);
		}while(*++error_message);
	}
}
