#ifndef TSI_H_
#define TSI_H_

#include "MCUType.h"
#include "os.h"

void TSI0_IRQHandler(void);
void TSI_Init(void);
void LPTMR1_Init(void);
void LPTMR1_IRQHandler(void);

/* Function prototypes */

#endif /* TSI_H_ */
