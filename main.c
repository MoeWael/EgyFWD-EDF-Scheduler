
/*
 * FreeRTOS Kernel V10.2.0
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "lpc21xx.h"
#include "queue.h"

/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"


/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )


/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware( void );
/*-----------------------------------------------------------*/
 
TaskHandle_t Button_1_Monitor_Handler = NULL;
TaskHandle_t Button_2_Monitor_Handler = NULL;
TaskHandle_t Periodic_Transmitter_Handler = NULL;
TaskHandle_t Uart_Receiver_Handler = NULL; 
TaskHandle_t Load_1_Simulation_Handler = NULL;
TaskHandle_t Load_2_Simulation_Handler = NULL;

int total_time;
int cpu_load;
int total_execution_time = 0;
int task_in;
int task_out;

QueueHandle_t MyQueue;

char pcWriteBuffer[300];
/*
void vApplicationTickHook( void ){
	
	GPIO_write(PORT_0, PIN1, PIN_IS_HIGH);
	GPIO_write(PORT_0, PIN1, PIN_IS_LOW);
	
}
*/

/* idle task callback function */

void vApplicationIdleHook( void )
{
	GPIO_write (PORT_0, PIN9, PIN_IS_HIGH);

}


	void Button_1_Monitor( void * pvParameters )
{
	
	pinState_t Button1_OldState = 1;
	pinState_t Button1_NewState;
	char* Message_Button1_Rising = "Button 1 Rising\n";
	char* Message_Button1_Falling = "Button 1 Falling\n";
	
	TickType_t Wake_Time;
	Wake_Time = xTaskGetTickCount();
	
	vTaskSetApplicationTaskTag( NULL, ( void * ) 0 );

    for( ;; )
    {
			vTaskDelayUntil(&Wake_Time, 50);
			
      Button1_NewState = GPIO_read(PORT_0, PIN0);
			if(Button1_NewState == 0 && Button1_OldState == 1)
			{
				
				 if( MyQueue != 0 )
				{
					
					xQueueSend( MyQueue,( void * ) &Message_Button1_Falling,( TickType_t ) 0 );

				}
				
			}
			if(Button1_NewState == 1 && Button1_OldState == 0)
			{
				 if( MyQueue != 0 )
				{
					
					xQueueSend( MyQueue,( void * ) &Message_Button1_Rising,( TickType_t ) 0 );

				}
			}
				
				Button1_OldState = Button1_NewState;

				
    }
		
		
}

	void Button_2_Monitor( void * pvParameters )
{
	
	pinState_t Button2_OldState = 1;
	pinState_t Button2_NewState;
	char* Message_Button2_Rising = "Button 2 Rising\n";
	char* Message_Button2_Falling = "Button 2 Falling\n";
	
	TickType_t Wake_Time;
	Wake_Time = xTaskGetTickCount();
	
	vTaskSetApplicationTaskTag( NULL, ( void * ) 1 );
    for( ;; )
    {
			
      Button2_NewState = GPIO_read(PORT_0, PIN1);
			if(Button2_NewState == 0 && Button2_OldState == 1)
			{
				
				 if( MyQueue != 0 )
				{
					
					xQueueSend( MyQueue,( void * ) &Message_Button2_Falling,( TickType_t ) 10 );

				}
				
			}
			if(Button2_NewState == 1 && Button2_OldState == 0)
			{
				 if( MyQueue != 0 )
				{
					
					xQueueSend( MyQueue,( void * ) &Message_Button2_Rising,( TickType_t ) 10 );

				}
			}
				
				Button2_OldState = Button2_NewState;
				vTaskDelayUntil(&Wake_Time, 50);
    }
		
		
}
	void Periodic_Transmitter( void * pvParameters )
{
	
	char* Periodic_Message = "Periodic_Message\n";
	
  TickType_t Wake_Time;
	Wake_Time = xTaskGetTickCount();  
	
	vTaskSetApplicationTaskTag( NULL, ( void * ) 2 );
	
	for( ;; )
    {
			
			if(MyQueue != 0)
			{
				xQueueSend( MyQueue,( void * ) &Periodic_Message,( TickType_t ) 0 );
			}
			vTaskDelayUntil(&Wake_Time, 100);
    }
}
	void Uart_Receiver( void * pvParameters )
{
	
	char* RxBuffer = NULL;
	
	TickType_t Wake_Time;
	Wake_Time = xTaskGetTickCount();
	
	vTaskSetApplicationTaskTag( NULL, ( void * ) 3 );
	
    for( ;; )
    {
		/*		vTaskGetRunTimeStats(pcWriteBuffer);
				vSerialPutString(pcWriteBuffer, 500);
				xSerialPutChar('a');*/
					
			if(xQueueReceive(MyQueue, &RxBuffer, ( TickType_t ) 0) == pdTRUE)
			{
				
				vSerialPutString((const signed char *)RxBuffer, 20);
			}
			vTaskDelayUntil(&Wake_Time, 20);
    }
}
	void Load_1_Simulation( void * pvParameters )
{
	
	uint16_t DelayCounter;
	
	TickType_t Wake_Time;
	Wake_Time = xTaskGetTickCount();
	
	vTaskSetApplicationTaskTag( NULL, ( void * ) 4 );
	
	for( ;; )
	{
		vTaskDelayUntil(&Wake_Time, 10);

    for(DelayCounter = 0; DelayCounter < 27000; DelayCounter++);

	}
}
	void Load_2_Simulation( void * pvParameters )
{
	
	uint32_t DelayCounter;
	
	TickType_t Wake_Time;
	Wake_Time = xTaskGetTickCount();

	vTaskSetApplicationTaskTag( NULL, ( void * ) 5 );
	
	for( ;; )
	{
		vTaskDelayUntil(&Wake_Time, 100);

    for(DelayCounter = 0; DelayCounter < 65800; DelayCounter++)
    {
				int i = 0;
    }
	}
}
/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */
int main( void )
{
	/* Setup the hardware for use with the Keil demo board. */
	prvSetupHardware();

	MyQueue = xQueueCreate( 10, sizeof( char* ) );
    /* Create Tasks here */

	
	xTaskPeriodicCreate( Button_1_Monitor,
											 "Button1", 
											 100,
											 0,
											 1,
											 &Button_1_Monitor_Handler,
											 50
											 );
	
	xTaskPeriodicCreate( Button_2_Monitor,
											 "Button2", 
											 100,
											 0,
											 1,
											 &Button_2_Monitor_Handler,
											 50
											 );

	xTaskPeriodicCreate( Periodic_Transmitter,
											 "Transmitter", 
											 100,
											 0,
											 1,
											 Periodic_Transmitter_Handler,
											 100);
											 
	xTaskPeriodicCreate( Uart_Receiver,
											 "Receiver", 
											 100,
											 0,
											 1,
											 Uart_Receiver_Handler,
											 20);
	
		xTaskPeriodicCreate( Load_1_Simulation,
											 "Load1", 
											 100,
											 0,
											 1,
											 Load_1_Simulation_Handler,
											 10);
	
			xTaskPeriodicCreate( Load_2_Simulation,
											 "Load2", 
											 100,
											 0,
											 1,
											 Load_2_Simulation_Handler,
											 100);
	/* Now all the tasks have been started - start the scheduler.

	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used here. */
	vTaskStartScheduler();

	/* Should never reach here!  If you do then there was not enough heap
	available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/

/* Function to reset timer 1 */
void timer1Reset(void)
{
	T1TCR |= 0x2;
	T1TCR &= ~0x2;
}

/* Function to initialize and start timer 1 */
static void configTimer1(void)
{
	T1PR = 1000;
	T1TCR |= 0x1;
}

static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();
	
	/* Config trace timer 1 and read T1TC to get current tick */
	configTimer1();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/


