#ifndef TSI_H_
#define TSI_H_

#include "MCUType.h"
#include "os.h"

void TSIInit(void);
void tsiChCalibration(void);
void TSITask(void);
void TSISwap(void);
INT8U TSITouchGet(void);

/* Function prototypes */

#endif /* TSI_H_ */
