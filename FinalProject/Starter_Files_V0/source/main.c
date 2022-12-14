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
#include "task.h"

/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"
#include "queue.h"
#include "semphr.h"


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




/*-----------------------------------------------------------*/
/*                   User-Defined-Types			                 */
/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/
/*                   User-Defined-Global-Variables           */
/*-----------------------------------------------------------*/

TaskHandle_t xTask_A_Handle = NULL;			/**> Defines Handler For Task 1 */
TaskHandle_t xTask_B_Handle = NULL;			/**> Defines Handler For Task 2 */

TickType_t TaskA_StartTime=0, TaskB_StartTime=0, TaskA_EndTime=0, TaskB_EndTime=0;
TickType_t TaskA_TotalTime=0, TaskB_TotalTime=0, System_Time=0;
TickType_t TaskA_Nominal_ExecTime=0, TaskB_Nominal_ExecTime=0;
TickType_t TaskA_S1=0,TaskA_E1=0,TaskB_S1=0,TaskB_E1=0;
unsigned int CPU_Load = 0;

char runTimeStatusBuff[200];

/*-----------------------------------------------------------*/
/*                   User-Defined-Macros                     */
/*-----------------------------------------------------------*/
#define NULL_PTR  		( (void*) 0U )			/**> Defines NULL Pointer */

#define TASK_A_DELAY_MS					(10U) 	/**> Defines Delay for Button A Task */
#define TASK_B_DELAY_MS					(20U) 	/**> Defines Delay for Button B Task */




/*-----------------------------------------------------------*/
/*                   User-Defined-Prototypes                 */
/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/
/*                   User-Defined-Tasks                      */
/*-----------------------------------------------------------*/


/*-----------------------------------------------------------
** Task_Name: Task_GetButtonA
** Task_Description: Task that gets input from button A
** Task_Periodicity: TASKS_BUTTONA_DELAY_MS <3>
** Task_Communication:
** Task_Synchronization:
** Task_Independent_Execution_Time: 4.01485 <ms>
------------------------------------------------------------*/
void Task_A( void * pvParameters )
{
	
		TickType_t xLastWakeTime = xTaskGetTickCount();
	
		vTaskSetApplicationTaskTag( NULL, (TaskHookFunction_t) 1 );
	
    for( ;; )
    {
			unsigned int i=0;
						
			for(i=0; i<= 30000;i++);		
			
			vTaskGetRunTimeStats(runTimeStatusBuff);
																		
			vTaskDelayUntil( &xLastWakeTime, TASK_A_DELAY_MS);
			
    }
}


/*-----------------------------------------------------------
** Task_Name: Task_GetButtonB
** Task_Description: Task that gets input from button B
** Task_Periodicity: TASKS_BUTTONB_DELAY_MS <4>
** Task_Communication:
** Task_Synchronization:
** Task_Independent_Execution_Time: 4.008 <ms>
------------------------------------------------------------*/
void Task_B( void * pvParameters )
{
		TickType_t xLastWakeTime = xTaskGetTickCount();
	
		vTaskSetApplicationTaskTag( NULL, (TaskHookFunction_t) 2 );
	
    for( ;; )
    {
			unsigned int i=0;
			
			for(i=0; i<= 30000;i++);		
						
			vTaskGetRunTimeStats(runTimeStatusBuff);
			
			xSerialPutChar('\n');
			
			vSerialPutString( (const signed char *) ((char*)&runTimeStatusBuff[0]), 65);
						
			vTaskDelayUntil( &xLastWakeTime, TASK_B_DELAY_MS);
			
    }
}




/*-----------------------------------------------------------*/
/*                   User-Defined-Functions                  */
/*-----------------------------------------------------------*/


/*-----------------------------------------------------------
** Function_Name: Task_CreateTasks
** Function_Description: Function that creates user tasks
** Function_Inputs:		<void>
** Function_Outputs:	<void>
------------------------------------------------------------*/
void Task_CreateTasks(void){
	
	#if configUSE_EDF_SCHEDULER == 1
	
	xTaskPeriodicCreate(    
							Task_B, 	/* Function that Implements the Task */
							"Task_B", /* Task Descriptive Name */
							150, 						/* Stack Word Size */
							NULL_PTR, 			/* Address To passed Parameter */
							1, 							/* Priority */
							&xTask_B_Handle, /* Used to pass out the the created task's handle . */
							TASK_B_DELAY_MS
						);
	
	xTaskPeriodicCreate(    
							Task_A, 	/* Function that Implements the Task */
							"Task_A", /* Task Descriptive Name */
							150, 						/* Stack Word Size */
							NULL_PTR, 			/* Address To passed Parameter */
							1, 							/* Priority */
							&xTask_A_Handle, /* Used to pass out the the created task's handle . */
							TASK_A_DELAY_MS
						);
						
	#else
	
		xTaskCreate(    
							Task_B, 	/* Function that Implements the Task */
							"Task_B", /* Task Descriptive Name */
							150, 						/* Stack Word Size */
							NULL_PTR, 			/* Address To passed Parameter */
							1, 							/* Priority */
							&xTask_B_Handle /* Used to pass out the the created task's handle . */
						);
	
	xTaskCreate(    
							Task_A, 	/* Function that Implements the Task */
							"Task_A", /* Task Descriptive Name */
							150, 						/* Stack Word Size */
							NULL_PTR, 			/* Address To passed Parameter */
							1, 							/* Priority */
							&xTask_A_Handle /* Used to pass out the the created task's handle . */
						);
	
	#endif
	
	
}



/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */
int main( void )
{
	/* Setup the hardware for use with the Keil demo board. */
	prvSetupHardware();
	
  /* Create Tasks here */
	Task_CreateTasks();
	
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
void configTimer1(void)
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


