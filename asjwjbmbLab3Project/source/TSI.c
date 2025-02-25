#include "MCUType.h"
#include "FRDM_MCXN947ClkCfg.h"
#include "FRDM_MCXN947_GPIO.h"
#include "TSI.h"
#include "os.h"

volatile uint32_t touch_value = 0; // Stores the latest touch reading

void TSI_Init(void) {
    // Enable clock for TSI module
    SYSCON->CLOCK_CTRL |= SYSCON_CLOCK_CTRL_CLKIN_ENA_FM_USBH_LPT(1);

    // Configure TSI in Self-Capacitance Mode (Set TSICH to desired channel, e.g., Channel 3)
    TSI0->CONFIG = (TSI0->CONFIG & ~TSI_CONFIG_TSICH_MASK) | TSI_CONFIG_TSICH(3);

    // Enable End-of-Scan Interrupt
    TSI0->GENCS = TSI_GENCS_TSIEN_MASK |  // Enable TSI module
                  TSI_GENCS_ESOR_MASK  |  // Enable End-Of-Scan interrupt
                  TSI_GENCS_STM_MASK;    // Hardware trigger mode

    // Enable NVIC interrupt
    NVIC_ClearPendingIRQ(CTI0_IRQn);
    NVIC_EnableIRQ(CTI0_IRQn);
}

void LPTMR1_Init(void) {
    // Enable clock for LPTMR1
    SYSCON->CLOCK_CTRL |= SYSCON_CLOCK_CTRL_CLKIN_ENA_FM_USBH_LPT(1);

    // Configure LPTMR1 to trigger TSI
    INPUTMUX->TSI_TRIG = 1;  // Set LPTMR1 as hardware trigger for TSI

    // Select the OSC_SYS clock and bypass prescaler
    LPTMR1->PSR = LPTMR_PSR_PCS(3) | LPTMR_PSR_PBYP(1);

    // Set Compare Register for a 100ms period (adjust as needed)
    LPTMR1->CMR = (24000000 / 10) - 1;

    // Clear CSR and interrupt flag
    LPTMR1->CSR = LPTMR_CSR_TCF(1);

    // Enable NVIC interrupt for LPTMR1
    NVIC_ClearPendingIRQ(LPTMR1_IRQn);
    NVIC_EnableIRQ(LPTMR1_IRQn);

    // Enable LPTMR1 and its interrupt
    LPTMR1->CSR |= LPTMR_CSR_TEN(1) | LPTMR_CSR_TIE(1);
}

void TSI0_IRQHandler(void) {
    if (TSI0->GENCS & TSI_GENCS_ESOR_MASK) {
        // Read touch value
        touch_value = TSI0->DATA & TSI_DATA_TSICNT_MASK;

        // Clear End-of-Scan flag
        TSI0->GENCS |= TSI_GENCS_ESOR_MASK;
    }
}

void LPTMR1_IRQHandler(void) {
    // Trigger TSI conversion
    TSI0->GENCS |= TSI_GENCS_SWTS_MASK;

    // Clear LPTMR interrupt flag
    LPTMR1->CSR |= LPTMR_CSR_TCF(1);
}
