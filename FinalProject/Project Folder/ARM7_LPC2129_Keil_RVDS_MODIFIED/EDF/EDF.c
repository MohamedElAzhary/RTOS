// File for implementing Earlies DeadLine First Functionlities

#include "FreeRTOS.h"
#include "list.h"
#include "mpu_wrappers.h"
#include "EDF.h"



void CorrectDeadlinesAfterOV(void);
unsigned char CalculateNextDeadline(EDFStruct_t* param_PtrTaskDeadlineStruct);



extern EDFStruct_t Tasks_EDFStructsArray[EDF_NUMBER_OF_TASKS];

void CorrectDeadlinesAfterOV( void )
{
	unsigned char i=0;
	for(i=0; i<EDF_NUMBER_OF_TASKS; i++)
	{
		Tasks_EDFStructsArray[i].execCount = 0U;
		CalculateNextDeadline(&Tasks_EDFStructsArray[i]);
	}
}


unsigned char CalculateNextDeadline(EDFStruct_t* param_PtrTaskDeadlineStruct)
{
	unsigned int u32Temp = (unsigned int)( ( (param_PtrTaskDeadlineStruct->execCount)*(param_PtrTaskDeadlineStruct->period)) + param_PtrTaskDeadlineStruct->nominalDeadline);
	unsigned char evaluationStatus = 0;
	if( u32Temp < (param_PtrTaskDeadlineStruct->nextDeadline) ){
		/* Overflow occured */
		evaluationStatus = 1;
	}
	else{
		/* Overflow did not occur */
		evaluationStatus = 0;
	}
	
	/* Assign calculated value */
	param_PtrTaskDeadlineStruct->nextDeadline = u32Temp;
	
	return evaluationStatus;
}






