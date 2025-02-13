/*******************************************************************************
* FRDM_MCXN947_GPIO.h - FRDM-MCXN947 GPIO module
*
* Provides GPIO implementation for the red, green, and blue LEDs, sw2, and sw3
* at the CMSIS Peripheral Access Layer level. Also provides 6 debug bits on P2.
*
* Todd Morton, 05/13/2024
 ******************************************************************************/

#ifndef GPIO_H_
#define GPIO_H_

/*******************************************************************************
* GpioSw3Init - Initialization for SW3 on the FRDM-MCXN947 board
*               SW3 is wired active-low, there is NO external pull-up on the
*               board so an internal pull-up is required.
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
void GpioSw3Init(INT8U irqc);

/*******************************************************************************
* GpioSw3Init - Initialization for SW2 on the FRDM-MCXN947 board
*               SW2 is wired active-low, there is NO external pull-up on the
*               board so an internal pull-up is required.
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
void GpioSw2Init(INT8U irqc);

/*******************************************************************************
* Function prototypes for the initialization LEDs GPIO and PORT.
 ******************************************************************************/
void GpioLEDGREENInit(void);
void GpioLEDREDInit(void);
void GpioLEDBLUEInit(void);

/*******************************************************************************
* Function prototype for initialization of the Debug bits GPIO and PORT registers.
 ******************************************************************************/
void GpioDBugBitsInit(void);

/*******************************************************************************
 * GPIO interrupt control macros - value passed to IRQC bits
 ******************************************************************************/
#define GPIO_IRQ_OFF       0    //Interrupts disabled
#define GPIO_IRQ_DMA_RE    1    //ISF and DMA on rising-edge
#define GPIO_IRQ_DMA_FE    2    //ISF and DMA on falling-edge
#define GPIO_IRQ_DMA_EE    3    //ISF and DMA on either edge
#define GPIO_IRQ_ISF_RE    5    //ISF on rising-edge
#define GPIO_IRQ_ISF_FE    6    //ISF on falling-edge
#define GPIO_IRQ_ISF_EE    7    //ISF on either edge
#define GPIO_IRQ_ZERO      8    //ISF and Interrupt when 0
#define GPIO_IRQ_RE        9    //ISF and Interrupt on rising-edge
#define GPIO_IRQ_FE       10    //ISF and Interrupt on falling-edge
#define GPIO_IRQ_EE       11    //ISF and Interrupt on either edge
#define GPIO_IRQ_ONE      12    //ISF and Interrupt when 1
#define GPIO_TRIG_RE      13    //Active-high trigger on rising-edge
#define GPIO_TRIG_FE      14    //Active-low trigger on falling-edge

/*******************************************************************************
 * Pin macro
 ******************************************************************************/
#define GPIO_PIN(x) (((1)<<(x & 0x1FU)))

/*******************************************************************************
 * Switch defines for SW2 (P0_23), SW3 (PT0_6),
 *                    RED_LED (PT0_10), GREEN_LED (P0_27), BLUE LED (PT1_2)
 * 					  LEDs active-low
 ******************************************************************************/
#define LED_RED_BIT 10U
#define LED_GREEN_BIT 27U
#define LED_BLUE_BIT 2U

#define RED_TURN_OFF()     (GPIO0->PSOR = GPIO_PIN(LED_RED_BIT))
#define RED_TURN_ON()      (GPIO0->PCOR = GPIO_PIN(LED_RED_BIT))
#define RED_TOGGLE()       (GPIO0->PTOR = GPIO_PIN(LED_RED_BIT))

#define GREEN_TURN_OFF()   (GPIO0->PSOR = GPIO_PIN(LED_GREEN_BIT))
#define GREEN_TURN_ON()    (GPIO0->PCOR = GPIO_PIN(LED_GREEN_BIT))
#define GREEN_TOGGLE()     (GPIO0->PTOR = GPIO_PIN(LED_GREEN_BIT))

#define BLUE_TURN_OFF()     (GPIO1->PSOR = GPIO_PIN(LED_BLUE_BIT))
#define BLUE_TURN_ON()      (GPIO1->PCOR = GPIO_PIN(LED_BLUE_BIT))
#define BLUE_TOGGLE()       (GPIO1->PTOR = GPIO_PIN(LED_BLUE_BIT))

#define SW2_BIT 23U
#define SW2_INPUT           (GPIO0->PDIR & GPIO_PIN(SW2_BIT))
#define SW2_ISF             (GPIO0->ISFR[0] & GPIO_PIN(SW2_BIT))

#define SW3_BIT 6U
#define SW3_INPUT           (GPIO0->PDIR & GPIO_PIN(SW3_BIT))
#define SW3_ISF             (GPIO0->ISFR[0] & GPIO_PIN(SW3_BIT))

/*******************************************************************************
 * Initialize SW2 and SW3 interrupt. irqc is one of the IRQC macros defined above
 * Also enables a pull-up since there are no external pull-up resistors
 ******************************************************************************/
#define SW2_INIT_IRQ(irqc)  (GPIO0->ICR[SW2_BIT] = GPIO_ICR_IRQC(irqc))
#define SW2_CLR_ISF()       (GPIO0->ISFR[0] = GPIO_PIN(SW2_BIT))
#define SW3_INIT_IRQ(irqc)  (GPIO0->ICR[SW3_BIT] = GPIO_ICR_IRQC(irqc))
#define SW3_CLR_ISF()       (GPIO0->ISFR[0] = GPIO_PIN(SW3_BIT))

/*******************************************************************************
 * #defines for  6 debug bits
 * On Port 2 bits 2 - 7, which are connected to J3 as follows:
 *     DB0 J3-15
 *     DB1 J3-13
 *     DB2 J3-11
 *     DB3 J3-9
 *     DB4 J3-7
 *     DB5 J3-5
 ******************************************************************************/
#define DB0_BIT 6U
#define DB1_BIT 7U
#define DB2_BIT 4U
#define DB3_BIT 5U
#define DB4_BIT 2U
#define DB5_BIT 3U

#define DB0_TURN_ON() (GPIO2->PSOR = GPIO_PIN(DB0_BIT))
#define DB1_TURN_ON() (GPIO2->PSOR = GPIO_PIN(DB1_BIT))
#define DB2_TURN_ON() (GPIO2->PSOR = GPIO_PIN(DB2_BIT))
#define DB3_TURN_ON() (GPIO2->PSOR = GPIO_PIN(DB3_BIT))
#define DB4_TURN_ON() (GPIO2->PSOR = GPIO_PIN(DB4_BIT))
#define DB5_TURN_ON() (GPIO2->PSOR = GPIO_PIN(DB5_BIT))

#define DB0_TURN_OFF() (GPIO2->PCOR = GPIO_PIN(DB0_BIT))
#define DB1_TURN_OFF() (GPIO2->PCOR = GPIO_PIN(DB1_BIT))
#define DB2_TURN_OFF() (GPIO2->PCOR = GPIO_PIN(DB2_BIT))
#define DB3_TURN_OFF() (GPIO2->PCOR = GPIO_PIN(DB3_BIT))
#define DB4_TURN_OFF() (GPIO2->PCOR = GPIO_PIN(DB4_BIT))
#define DB5_TURN_OFF() (GPIO2->PCOR = GPIO_PIN(DB5_BIT))

#define DB0_TOGGLE() (GPIO2->PTOR = GPIO_PIN(DB0_BIT))
#define DB1_TOGGLE() (GPIO2->PTOR = GPIO_PIN(DB1_BIT))
#define DB2_TOGGLE() (GPIO2->PTOR = GPIO_PIN(DB2_BIT))
#define DB3_TOGGLE() (GPIO2->PTOR = GPIO_PIN(DB3_BIT))
#define DB4_TOGGLE() (GPIO2->PTOR = GPIO_PIN(DB4_BIT))
#define DB5_TOGGLE() (GPIO2->PTOR = GPIO_PIN(DB5_BIT))
#endif /* DBUGBITS_H_ */
