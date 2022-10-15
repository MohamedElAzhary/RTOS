#ifndef EDF_H
#define EDF_H

#include "EDF_cfg.h"





typedef struct{
  unsigned int nominalDeadline;
  unsigned int nextDeadline;
  unsigned int execCount;
  unsigned int period;

} EDFStruct_t;




#endif /* End of #ifndef EDF_H */
