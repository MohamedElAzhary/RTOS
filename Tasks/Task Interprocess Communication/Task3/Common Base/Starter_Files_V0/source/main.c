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
/*                   User-Defined-Macros                     */
/*-----------------------------------------------------------*/
#define NULL_PTR  		( (void*) 0U )			/**> Defines NULL Pointer */

#define TASKS_BUTTONA_DELAY_MS					(3U) 	/**> Defines Delay for Button A Task */

#define TASKS_BUTTONB_DELAY_MS					(4U) 	/**> Defines Delay for Button B Task */

#define TASKS_PERIODIC_DISPLAY_DELAY_MS	(5U) /**> Defines Delay for Periodic Display Task */

#define TASKS_UART_SENDER_DELAY_MS			(1U) 	/**> Defines Delay for UART Sender Task */

#define QUEUE_UART1_RISINGSTRING_SIZE_BYTES		(12U)		/**> Defines String Size in element inside Queue */

#define QUEUE_UART1_FALLINGSTRING_SIZE_BYTES		(13U)		/**> Defines String Size in element inside Queue */

#define QUEUE_UART1_PERIODICSTRING_SIZE_BYTES		(11U)		/**> Defines String Size in element inside Queue */

#define QUEUE_UART1_ELEMENTS_NUMBER			(20U)   /**> Defines Queue elements number */

/**> Defines simple delay */
#define DELAY_500us()					{\
																unsigned short i = 0;\
																for(i=4300; i>0; i--);\
															}


/*-----------------------------------------------------------*/
/*                   User-Defined-Types			                 */
/*-----------------------------------------------------------*/
typedef struct{
	/* Sender ID */
	unsigned short sender_id;
	/* Message ID */
	unsigned short message_id;	
} xQueueItem_t;

/* Enum Used for Button States Definition */
typedef enum{
	state_ButtonPressed = 1,
	state_ButtonReleased = 2,
} ButtonState_t;

/* Enum Used for Edge Definition */
typedef enum{
	state_rising = 1,
	state_falling = 2,	
} EdgeState_t;

/*-----------------------------------------------------------*/
/*                   User-Defined-Global-Variables           */
/*-----------------------------------------------------------*/

TaskHandle_t xTask_GetButtonA_Handle = NULL;			/**> Defines Handler For Task 1 */

TaskHandle_t xTask_GetButtonB_Handle = NULL;			/**> Defines Handler For Task 2 */

TaskHandle_t xTask_SendOnUart_Handle = NULL;			/**> Defines Handler For Task 3 */

TaskHandle_t xTask_PeriodicDisplay_Handle = NULL;			/**> Defines Handler For Task 4 */

SemaphoreHandle_t xUART1_Semaphore = NULL;		/**> Defines Handler UART1 Semaphore */

QueueHandle_t xUART1_Queue1 = NULL;						/**> Defines Handler For UART1 Queue */

pinState_t xButton_State = PIN_IS_LOW;				/**> Defines Global Variable holding Button State */

unsigned char message_RisingEdge[QUEUE_UART1_RISINGSTRING_SIZE_BYTES] = "Rising Edge\n";		/**> Defines a message on UART */

unsigned char message_FallingEdge[QUEUE_UART1_FALLINGSTRING_SIZE_BYTES] = "Falling Edge\n";  /**> Defines a message on UART */

unsigned char message_Periodic[QUEUE_UART1_PERIODICSTRING_SIZE_BYTES] = "UART ALIVE\n"; 		  /**> Defines a message on UART */
			
xQueueItem_t ButtonAItem = {1,0};			/* Queue Item for task */

xQueueItem_t ButtonBItem = {2,0};			/* Queue Item for task */

xQueueItem_t PeriodicDisplayItem = {3,0};			/* Queue Item for task */

ButtonState_t global_xButtonAPreviousState = state_ButtonReleased, global_xButtonBPreviousState = state_ButtonReleased ;	/**> Defines Global Variable holding Button Previous State */

ButtonState_t global_xButtonACurrentState = state_ButtonReleased, global_xButtonBCurrentState = state_ButtonReleased;	/**> Defines Global Variable holding Button Current State */

EdgeState_t global_ButtonAEdge = state_falling;		/**> Defines Button A Edge */

EdgeState_t global_ButtonBEdge = state_falling;		/**> Defines Button B Edge */


/*-----------------------------------------------------------*/
/*                   User-Defined-Prototypes                 */
/*-----------------------------------------------------------*/
unsigned char UART_DecodeMessage(xQueueItem_t *param_ItemAddress, unsigned short *param_StringSize, unsigned char **param_StringAddress);

/*-----------------------------------------------------------*/
/*                   User-Defined-Tasks                      */
/*-----------------------------------------------------------*/


/*-----------------------------------------------------------
** Task_Name: Task_GetButtonA
** Task_Description: Task that gets input from button A
** Task_Periodicity: TASKS_BUTTONA_DELAY_MS <3>
** Task_Communication:
** Task_Synchronization:
** Task_Independent_Execution_Time: 6.13 <us>
------------------------------------------------------------*/

void Task_GetButtonA( void * pvParameters )
{
    for( ;; )
    {
			/* Read Pin State */
			pinState_t local_xPinState = PIN_IS_LOW;
									
			GPIO_write(PORT_0, PIN2, PIN_IS_HIGH);
			
			/* Read Pin State */
			local_xPinState = GPIO_read(PORT_0, PIN0);
			
			/* Set Previous State as Current state */
			global_xButtonAPreviousState = global_xButtonACurrentState;
			
			/* Check if Pin is High */
			if( PIN_IS_HIGH == local_xPinState ){
				/* Change Current state to pressed */
				global_xButtonACurrentState = state_ButtonPressed;
			}
			else{
				/* Change Current state to released */
				global_xButtonACurrentState = state_ButtonReleased;			
			}
			
			/* Check if Button is pressed then released */
			if( (global_xButtonACurrentState == state_ButtonReleased) && (global_xButtonAPreviousState == state_ButtonPressed) ){
				/* Set Falling Edge State */
				ButtonAItem.message_id = 1;
				/* Send to Queue */
				xQueueSend( xUART1_Queue1, &ButtonAItem, 1);	
			}
			else if( (global_xButtonACurrentState == state_ButtonPressed) && (global_xButtonAPreviousState == state_ButtonReleased) ){
				/* Set Rising Edge State */
				ButtonAItem.message_id = 0;
				/* Send to Queue */
				xQueueSend( xUART1_Queue1, &ButtonAItem, 1);	
			}
			else{
				/* Do nothing */
			}
			
		
						
			GPIO_write(PORT_0, PIN2, PIN_IS_LOW);
						
			vTaskDelay(TASKS_BUTTONA_DELAY_MS);
    }
}


/*-----------------------------------------------------------
** Task_Name: Task_GetButtonB
** Task_Description: Task that gets input from button B
** Task_Periodicity: TASKS_BUTTONB_DELAY_MS <4>
** Task_Communication:
** Task_Synchronization:
** Task_Independent_Execution_Time: 6.13 <us>
------------------------------------------------------------*/

void Task_GetButtonB( void * pvParameters )
{
		for( ;; )
    {
			/* Read Pin State */
			pinState_t local_xPinState = PIN_IS_LOW;
	
			GPIO_write(PORT_0, PIN3, PIN_IS_HIGH);
			
			/* Read Pin State */
			local_xPinState = GPIO_read(PORT_0, PIN1);
			
			/* Set Previous State as Current state */
			global_xButtonBPreviousState = global_xButtonBCurrentState;
			
			/* Check if Pin is High */
			if( PIN_IS_HIGH == local_xPinState ){
				/* Change Current state to pressed */
				global_xButtonBCurrentState = state_ButtonPressed;
			}
			else{
				/* Change Current state to released */
				global_xButtonBCurrentState = state_ButtonReleased;			
			}
			
			/* Check if Button is pressed then released */
			if( (global_xButtonBCurrentState == state_ButtonReleased) && (global_xButtonBPreviousState == state_ButtonPressed) ){
				/* Set Falling Edge State */
				ButtonBItem.message_id = 1;
				/* Send to Queue */
				xQueueSend( xUART1_Queue1, &ButtonBItem, 1);		
			}
			else if( (global_xButtonBCurrentState == state_ButtonPressed) && (global_xButtonBPreviousState == state_ButtonReleased) ){
				/* Set Rising Edge State */
				ButtonBItem.message_id = 0;
				/* Send to Queue */
				xQueueSend( xUART1_Queue1, &ButtonBItem, 1);		
			}
			else{
				/* Do nothing */
			}
			
	
						
			GPIO_write(PORT_0, PIN3, PIN_IS_LOW);
						
			vTaskDelay(TASKS_BUTTONB_DELAY_MS);
    }
}


/*-----------------------------------------------------------
** Task_Name: Task_PeriodicDisplay
** Task_Description: Task that periodically sends string to uart task
** Task_Periodicity: TASKS_PERIODIC_DISPLAY_DELAY_MS <5>
** Task_Communication:
** Task_Synchronization:
** Task_Independent_Execution_Time: 
------------------------------------------------------------*/

void Task_PeriodicDisplay( void * pvParameters )
{
    for( ;; )
    {
			GPIO_write(PORT_0, PIN4, PIN_IS_HIGH);
				
			PeriodicDisplayItem.message_id = 2;
			
			/* Send to Queue */
			xQueueSend( xUART1_Queue1, &PeriodicDisplayItem, 1);			
			
			GPIO_write(PORT_0, PIN4, PIN_IS_LOW);
			
			vTaskDelay(TASKS_PERIODIC_DISPLAY_DELAY_MS);
    }
}


/*-----------------------------------------------------------
** Task_Name: Task_SendOnUart
** Task_Description: Task that extract data from queue and send it on UART
** Task_Periodicity: TASKS_UART_SENDER_DELAY_MS <50>
** Task_Communication:
** Task_Synchronization:
** Task_Independent_Execution_Time:
------------------------------------------------------------*/

void Task_SendOnUart( void * pvParameters )
{
    for( ;; )
    {
			/* New item in Queue */
			xQueueItem_t item;
			
			/* String Related Address */
			unsigned short stringSize = 0;
			unsigned char *stringPtr = NULL_PTR;
			
			GPIO_write(PORT_0, PIN5, PIN_IS_HIGH);
			
			/* Pop from Queue */
			if( pdTRUE  == xQueueReceive(xUART1_Queue1, &item, 1  )){
				/* Decode Message */
				if( 1 == UART_DecodeMessage(&item, &stringSize, &stringPtr) ){							
					/* Send String */
					vSerialPutString( (const signed char *) stringPtr, stringSize );
					/* Delay */
					DELAY_500us();
				}
			}
			
			GPIO_write(PORT_0, PIN5, PIN_IS_LOW);
			
			vTaskDelay(TASKS_UART_SENDER_DELAY_MS);
    }
}


/*-----------------------------------------------------------*/
/*                   User-Defined-Functions                  */
/*-----------------------------------------------------------*/

/*-----------------------------------------------------------
** Function_Name: UART_DecodeMessage
** Function_Description: Function that Decodes Messages from Queue
** Function_Inputs:		<xQueueItem_t *> param_ItemAddress : Queue item address
**										<unsigned int *> param_StringSize : address of variable holding string size
**										<unsigned char *> param_StringAddress : address of variable holding string
** Function_Outputs:	<unsigned char> decoding result
------------------------------------------------------------*/
unsigned char UART_DecodeMessage(xQueueItem_t *param_ItemAddress, unsigned short *param_StringSize, unsigned char **param_StringAddress){
	
	/* Decoding result */
	unsigned char result = 1U;
	/* Get Message ID */
	switch( param_ItemAddress->message_id ){
		case 0:
			*param_StringAddress = message_RisingEdge;
			*param_StringSize = QUEUE_UART1_RISINGSTRING_SIZE_BYTES;
			break;
		case 1:
			*param_StringAddress =  message_FallingEdge;
		  *param_StringSize = QUEUE_UART1_FALLINGSTRING_SIZE_BYTES;
			break;
		case 2:
			*param_StringAddress = message_Periodic;
			*param_StringSize = QUEUE_UART1_PERIODICSTRING_SIZE_BYTES;
			break;
		default:
			*param_StringAddress = NULL_PTR;
		  *param_StringSize = 0;
			result = 0U;
			break;
	} /* End of switch( param_ItemAddress->message_id ) */
	
	return result;
}	/* End of UART_DecodeMessage */

/*-----------------------------------------------------------
** Function_Name: Task_CreateTasks
** Function_Description: Function that creates user tasks
** Function_Inputs:		<void>
** Function_Outputs:	<void>
------------------------------------------------------------*/
void Task_CreateTasks(void){
	
	xTaskCreate(    
							Task_GetButtonA, 	/* Function that Implements the Task */
							"Task_GetButtonA", /* Task Descriptive Name */
							100, 						/* Stack Word Size */
							NULL_PTR, 			/* Address To passed Parameter */
							1, 							/* Priority */
							&xTask_GetButtonA_Handle /* Used to pass out the the created task's handle . */
						);
	
	xTaskCreate(    
							Task_GetButtonB, 	/* Function that Implements the Task */
							"Task_GetButtonB", /* Task Descriptive Name */
							100, 								/* Stack Word Size */
							NULL_PTR, 					/* Address To passed Parameter */
							1, 									/* Priority */
							&xTask_GetButtonB_Handle /* Used to pass out the the created task's handle . */
						);

	xTaskCreate(    
							Task_PeriodicDisplay, 	/* Function that Implements the Task */
							"Task_PeriodicDisplay", /* Task Descriptive Name */
							100, 								/* Stack Word Size */
							NULL_PTR, 					/* Address To passed Parameter */
							1, 									/* Priority */
							&xTask_PeriodicDisplay_Handle /* Used to pass out the the created task's handle . */
						);	
	
	xTaskCreate(    
							Task_SendOnUart, 	/* Function that Implements the Task */
							"Task_SendOnUart", /* Task Descriptive Name */
							100, 								/* Stack Word Size */
							NULL_PTR, 					/* Address To passed Parameter */
							1, 									/* Priority */
							&xTask_SendOnUart_Handle /* Used to pass out the the created task's handle . */
						);
	

	
}

/*-----------------------------------------------------------
** Function_Name: Task_CreateEnv
** Function_Description: Function that creates system environment
** Function_Inputs:		<void>
** Function_Outputs:	<void>
------------------------------------------------------------*/
void Task_CreateEnv(void){
	/* Create UART1 Queue */
	xUART1_Queue1 = xQueueCreate( QUEUE_UART1_ELEMENTS_NUMBER, sizeof( xQueueItem_t ) );
	
	/* Create UART1 Semaphore */
	xUART1_Semaphore = xSemaphoreCreateBinary();
	
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
	
	/* Create Environment here */
	Task_CreateEnv();

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


