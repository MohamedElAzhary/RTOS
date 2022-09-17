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
#include <string.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "lpc21xx.h"
#include "task.h"
#include "semphr.h"
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


/*-----------------------------------------------------------*/
/*                   User-Defined-Macros                     */
/*-----------------------------------------------------------*/

#define NULL_PTR  		( (void*) 0U )			/**> Defines NULL Pointer */


/*-----------------------------------------------------------*/
/*                   User-Defined-Global-Variables           */
/*-----------------------------------------------------------*/

TaskHandle_t xTask_100ms_Handle = NULL;			/**> Defines Handler For Task 1 100 ms */

TaskHandle_t xTask_500ms_Handle = NULL; 		/**> Defines Handler For Task 2 500 ms */

SemaphoreHandle_t xUart1SemaphoreMutex;			/**> Define Semaphore object for Mutex Creation */

unsigned char i_100 = 0, i_500 = 0;					/**> Defines indices for strings printing */
unsigned int h1 = 0U, h2 = 0U;							/**> Defines indices for delays */

const signed char *const String1 = "Hi\n";	/**> Defines string 1 */
const signed char *const String2 = "Bye\n";	/**> Defines string 2 */

#define STRING_1_SIZE		(3U)								/**> Defines string 1 Size */
#define STRING_2_SIZE		(4U)								/**> Defines string 2 Size */


/*-----------------------------------------------------------*/
/*                   User-Defined-Tasks                      */
/*-----------------------------------------------------------*/


/*-----------------------------------------------------------
** Task_Name: Task_100ms
** Task_Description: Task that sends a string 10 times every 100 ms periodcally.
** Task_Periodicity: 100 <ms>
** Task_Communication:
** Task_Synchronization:
** Independent_Task_Execution_Time: 98.05 <us>
------------------------------------------------------------*/
void Task_100ms( void * pvParameters )
{

    for( ;; ){			
			GPIO_write( PORT_0, PIN1, PIN_IS_HIGH);
			if( xUart1SemaphoreMutex != NULL ){
				if( xSemaphoreTake( xUart1SemaphoreMutex, ( TickType_t ) 0  ) == pdTRUE ){
					for(i_100 = 0U; i_100 < 10U; i_100++){																								
						vSerialPutString(String1, STRING_1_SIZE);		
						for(h1=0; h1< 5000; h1++);						
					}
					xSemaphoreGive(xUart1SemaphoreMutex);										
				}																				
			}
			GPIO_write( PORT_0, PIN1, PIN_IS_LOW);			
			vTaskDelay(100);
    }
}

/*-----------------------------------------------------------
** Task_Name: Task_500ms
** Task_Description: Task that sends a string 10 times every 500 ms periodcally.
** Task_Periodicity: 500 <ms>
** Task_Communication:
** Task_Synchronization:
** Independent_Task_Execution_Time: 0.418691 <s>
------------------------------------------------------------*/
void Task_500ms( void * pvParameters )
{
	 for( ;; )
	{	
				GPIO_write( PORT_0, PIN2, PIN_IS_HIGH);
				if( xUart1SemaphoreMutex != NULL ){			
					if( xSemaphoreTake( xUart1SemaphoreMutex, ( TickType_t ) 0  ) == pdTRUE ){
						for(i_500 = 0U; i_500 < 10U; i_500++){															
							vSerialPutString(String2, STRING_2_SIZE);
							for(h2=0; h2< 100000; h2++);
						}
						xSemaphoreGive(xUart1SemaphoreMutex);
					}																	
				}
				GPIO_write( PORT_0, PIN2, PIN_IS_LOW);
			
			vTaskDelay(500);
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
	
	xTaskCreate(    
							Task_100ms, 	/* Function that Implements the Task */
							"Task100", /* Task Descriptive Name */
							100, 						/* Stack Word Size */
							NULL_PTR, 			/* Address To passed Parameter */
							2, 							/* Priority */
							&xTask_100ms_Handle /* Used to pass out the the created task's handle . */
						);
	
	xTaskCreate(    
							Task_500ms, 	/* Function that Implements the Task */
							"Task500", /* Task Descriptive Name */
							100, 								/* Stack Word Size */
							NULL_PTR, 					/* Address To passed Parameter */
							1, 									/* Priority */
							&xTask_500ms_Handle /* Used to pass out the the created task's handle . */
						);
	
}


/*-----------------------------------------------------------
** Function_Name: Semph_CreateSemaphores
** Function_Description: Function that creates user semaphores
** Function_Inputs:		<void>
** Function_Outputs:	<void>
------------------------------------------------------------*/
void Semph_CreateSemaphores(void){
	/* Create a semaphore object */
	xUart1SemaphoreMutex = xSemaphoreCreateMutex();
}

/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */
int main( void )
{
	/* Setup the hardware for use with the Keil demo board. */
	prvSetupHardware();

	/* Create Mutex here */
	Semph_CreateSemaphores();
	
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

static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/


