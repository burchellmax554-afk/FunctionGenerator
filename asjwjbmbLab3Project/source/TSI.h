#ifndef TSI_H_
#define TSI_H_

#include "MCUType.h"
#include "os.h"

void TSIInit(void);
static void tsiChCalibration(void);
void TSITask(void);
INT8U TSITouchGet(void);

/* Function prototypes */

#endif /* TSI_H_ */
