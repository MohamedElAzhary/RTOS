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
/*                   User-Defined-Types                      */
/*-----------------------------------------------------------*/

/* Enum Used for Button States Definition */
typedef enum{
	state_ButtonPressed = 1,
	state_ButtonReleased = 2,
} ButtonState_t;


/*-----------------------------------------------------------*/
/*                   User-Defined-Global-Variables           */
/*-----------------------------------------------------------*/

TaskHandle_t xTaskLedCtrl_Handle = NULL;			/**> Defines Handler For Task 1 */

TaskHandle_t xTaskCheckButton_Handle = NULL; 	/**> Defines Handler For Task 2*/

pinState_t xButtonControl_State = PIN_IS_LOW;				/**> Defines Global Variable holding Button State */

ButtonState_t global_xButtonPreviousState = state_ButtonReleased;	/**> Defines Global Variable holding Button Previous State */
ButtonState_t global_xButtonCurrentState = state_ButtonReleased;	/**> Defines Global Variable holding Button Current State */
uint8_t global_StateCounter = 0U;		/**> Defines Global Variable holding state counter */

/*-----------------------------------------------------------*/
/*                   User-Defined-Tasks                      */
/*-----------------------------------------------------------*/


/*-----------------------------------------------------------
** Task_Name: Task_LedCtrl
** Task_Description: Task that controls LED flashing periodically
** Task_Periodicity: 120 <ms>
** Task_Communication: --
** Task_Synchronization: --
------------------------------------------------------------*/

void Task_LedCtrl( void * pvParameters )
{

    for( ;; )
    {
			/* If Button Is High */
			if(xButtonControl_State == PIN_IS_HIGH){				
				/* Set LED as High */
				GPIO_write( PORT_0, PIN1, PIN_IS_HIGH);
			}
			else{
				/* Set LED as Low */
				GPIO_write( PORT_0, PIN1, PIN_IS_LOW);
			}
			
			vTaskDelay(120);
    }
}

/*-----------------------------------------------------------
** Task_Name: Task_CheckButton
** Task_Description: Task that gets input of a button periodically
** Task_Periodicity: 50 <ms>
** Task_Communication: --
** Task_Synchronization: --
------------------------------------------------------------*/
void Task_CheckButton( void * pvParameters )
{
	 for( ;; )
	{
		/* Read Pin State */
		pinState_t local_xPinState = GPIO_read(PORT_0, PIN0);
		
		/* Set Previous State as Current state */
		global_xButtonPreviousState = global_xButtonCurrentState;
		
		/* Check if Pin is High */
		if( PIN_IS_HIGH == local_xPinState ){
			/* Change Current state to pressed */
			global_xButtonCurrentState = state_ButtonPressed;
		}
		else{
			/* Change Current state to released */
			global_xButtonCurrentState = state_ButtonReleased;			
		}
		
		/* Check if Button is pressed then released */
		if( (global_xButtonCurrentState == state_ButtonReleased) && (global_xButtonPreviousState == state_ButtonPressed) ){
			/* Set Button Control as High */
			xButtonControl_State ^= PIN_IS_HIGH;
		}

					
		vTaskDelay(50);
				
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
							Task_LedCtrl, 	/* Function that Implements the Task */
							"Task_LedCtrl", /* Task Descriptive Name */
							100, 						/* Stack Word Size */
							NULL_PTR, 			/* Address To passed Parameter */
							1, 							/* Priority */
							&xTaskLedCtrl_Handle /* Used to pass out the the created task's handle . */
						);
	
	xTaskCreate(    
							Task_CheckButton, 	/* Function that Implements the Task */
							"Task_CheckButton", /* Task Descriptive Name */
							100, 								/* Stack Word Size */
							NULL_PTR, 					/* Address To passed Parameter */
							1, 									/* Priority */
							&xTaskCheckButton_Handle /* Used to pass out the the created task's handle . */
						);
	
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


