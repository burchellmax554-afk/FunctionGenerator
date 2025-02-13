/*
 * Assert code that uses the BasicIO.
 * Does not use fsl. This simplifies and reduces code.
 * 10/18/2024 Todd Morton
 */

#include "MCUType.h"
#include "assert.h"
#include "BasicIO.h"

#ifndef NDEBUG
void __assertion_failed(char *failedExpr)
{

	BIOPutStrg("ASSERT ERROR ");
	BIOPutStrg(failedExpr);
	BIOOutCRLF();
    for (;;)
    {
        __BKPT(0);
    }
}
#endif /* NDEBUG */
