/*******************************************************************************
* FRDM_MCXN947_GPIO.c - FRDM-MCXN947 GPIO module
*
* Provides GPIO implementation for the red, green, and blue LEDs, sw2, and sw3
* at the CMSIS Peripheral Access Layer level. Also provides 6 debug bits on P2.
*
* Todd Morton, 02/09/2025
 ******************************************************************************/
#include "MCUType.h"
#include "FRDM_MCXN947_GPIO.h"

/*******************************************************************************
* GpioSw3Init - Initialization for SW3 on the FRDM-MCXN947 board
*               SW3 is wired active-low, there is an external pull-up on the
*               board so an internal pull-up is not required.
* IRQC Parameters:
*  GPIO_IRQ_OFF       Interrupts disabled
*  GPIO_IRQ_DMA_RE    ISF and DMA on rising-edge
*  GPIO_IRQ_DMA_FE    ISF and DMA on falling-edge
*  GPIO_IRQ_DMA_EE    ISF and DMA on either edge
*  GPIO_IRQ_ISF_RE    ISF on rising-edge
*  GPIO_IRQ_ISF_FE    ISF on falling-edge
*  GPIO_IRQ_ISF_EE    ISF on either edge
*  GPIO_IRQ_ZERO      ISF and Interrupt when 0
*  GPIO_IRQ_RE        ISF and Interrupt on rising-edge
*  GPIO_IRQ_FE        ISF and Interrupt on falling-edge
*  GPIO_IRQ_EE        ISF and Interrupt on either edge
*  GPIO_IRQ_ONE       ISF and Interrupt when 1
*  GPIO_TRIG_RE       Active-high trigger on rising-edge
*  GPIO_TRIG_FE       Active-low trigger on falling-edge
 ******************************************************************************/
void GpioSw3Init(INT8U irqc){

	SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRL0_PORT0(1); /* Enable clock gate for PORT0 */
	SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRL0_GPIO0(1); /* Enable clock gate for GPIO0 */
    PORT0->PCR[SW3_BIT] = PORT_PCR_MUX(0)|PORT_PCR_IBE(1);
    GPIO0->ICR[SW3_BIT] = ((GPIO0->ICR[SW3_BIT] & ~GPIO_ICR_IRQC_MASK) | GPIO_ICR_IRQC(irqc));
}

/*******************************************************************************
* GpioSw3Init - Initialization for SW2 on the FRDM-MCXN947 board
*               SW2 is wired active-low, there is an external pull-up on the
*               board so an internal pull-up is not required.
* IRQC Parameters:
*  GPIO_IRQ_OFF       Interrupts disabled
*  GPIO_IRQ_DMA_RE    ISF and DMA on rising-edge
*  GPIO_IRQ_DMA_FE    ISF and DMA on falling-edge
*  GPIO_IRQ_DMA_EE    ISF and DMA on either edge
*  GPIO_IRQ_ISF_RE    ISF on rising-edge
*  GPIO_IRQ_ISF_FE    ISF on falling-edge
*  GPIO_IRQ_ISF_EE    ISF on either edge
*  GPIO_IRQ_ZERO      ISF and Interrupt when 0
*  GPIO_IRQ_RE        ISF and Interrupt on rising-edge
*  GPIO_IRQ_FE        ISF and Interrupt on falling-edge
*  GPIO_IRQ_EE        ISF and Interrupt on either edge
*  GPIO_IRQ_ONE       ISF and Interrupt when 1
*  GPIO_TRIG_RE       Active-high trigger on rising-edge
*  GPIO_TRIG_FE       Active-low trigger on falling-edge
 ******************************************************************************/
void GpioSw2Init(INT8U irqc){

	SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRL0_PORT0(1); /* Enable clock gate for PORT0 */
	SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRL0_GPIO0(1); /* Enable clock gate for PORT0 */
    PORT0->PCR[SW2_BIT] = PORT_PCR_MUX(0)|PORT_PCR_IBE(1);
    GPIO0->ICR[SW2_BIT] = ((GPIO0->ICR[SW2_BIT] & ~GPIO_ICR_IRQC_MASK) | GPIO_ICR_IRQC(irqc));

}

/*******************************************************************************
* GpioLEDxxxInit - Initialization for the three RGB LEDs.
* 05/09/2024, TDM
 ******************************************************************************/
void GpioLEDREDInit(void){

	SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRL0_PORT0(1); /* Enable clock gate for PORT0 */
	SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRL0_GPIO0(1); /* Enable clock gate for GPIO0 */
    PORT0->PCR[LED_RED_BIT] = PORT_PCR_MUX(0);
    GPIO0->PSOR = GPIO_PIN(LED_RED_BIT);     /* Initialize off, activelow */
    GPIO0->PDDR |= GPIO_PIN(LED_RED_BIT);
}

void GpioLEDGREENInit(void){

	SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRL0_PORT0(1); /* Enable clock gate for PORT0 */
	SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRL0_GPIO0(1); /* Enable clock gate for GPIO0 */
    PORT0->PCR[LED_GREEN_BIT] = PORT_PCR_MUX(0);
    GPIO0->PSOR = GPIO_PIN(LED_GREEN_BIT);     /* Initialize off, activelow */
    GPIO0->PDDR |= GPIO_PIN(LED_GREEN_BIT);
}
void GpioLEDBLUEInit(void){

	SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRL0_PORT1(1); /* Enable clock gate for PORT1 */
	SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRL0_GPIO1(1); /* Enable clock gate for GPIO1 */
    PORT1->PCR[LED_BLUE_BIT] = PORT_PCR_MUX(0);
    GPIO1->PSOR = GPIO_PIN(LED_BLUE_BIT);     /* Initialize off, activelow */
    GPIO1->PDDR |= GPIO_PIN(LED_BLUE_BIT);
}
void GpioDBugBitsInit(void){
	SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRL0_PORT2(1); /* Enable clock gate for PORT2 */
	SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRL0_GPIO2(1); /* Enable clock gate for GPIO2 */
    GPIO2->PCOR = GPIO_PIN(DB0_BIT)|GPIO_PIN(DB1_BIT)|GPIO_PIN(DB2_BIT)|
                  GPIO_PIN(DB3_BIT)|GPIO_PIN(DB4_BIT)|GPIO_PIN(DB5_BIT);
    GPIO2->PDDR = GPIO_PIN(DB0_BIT)|GPIO_PIN(DB1_BIT)|GPIO_PIN(DB2_BIT)|
                  GPIO_PIN(DB3_BIT)|GPIO_PIN(DB4_BIT)|GPIO_PIN(DB5_BIT);
}
