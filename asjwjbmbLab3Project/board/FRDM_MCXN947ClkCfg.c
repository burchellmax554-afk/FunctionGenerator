/***********************************************************************************************************************
 * FRDM_MCXN947ClkCfg
 * This is a low level version of the clock initialization using only MCXN947_cm33_core0.h hardware abstraction layer.
 * Can ONLY be used at startup as it assumes reset values in all registers.
 * Will not work with NXP clock config tools.
 * Sets main clock to 150MHz from a 24MHz XTAL.
 *
 * 24MHZ XTAL ---> PLL ---> AHB (main) clock 150MHz
 *
 * TDM, 05/07/2024
 **********************************************************************************************************************/
#include "MCUType.h"
#include "FRDM_MCXN947ClkCfg.h"

/*********************************************************************************************************************/

void FRDM_MCXN947InitBootClock(void)
{
	/* Enable SCG clock */
	SYSCON->AHBCLKCTRLSET[2] |= SYSCON_AHBCLKCTRL2_SCG(1);

    /* Set the DCDC VDD regulator to 1.2 V voltage level*/
    SPC0->ACTIVE_CFG = (SPC0->ACTIVE_CFG & (~SPC_ACTIVE_CFG_DCDC_VDD_LVL_MASK)) | SPC_ACTIVE_CFG_DCDC_VDD_LVL(3);
    /* Set the LDO_CORE VDD regulator to 1.2 V voltage level*/
    SPC0->ACTIVE_CFG = (SPC0->ACTIVE_CFG & (~SPC_ACTIVE_CFG_CORELDO_VDD_LVL_MASK)) | SPC_ACTIVE_CFG_CORELDO_VDD_LVL(3);
    /* Configure Flash wait-states to support 1.2V voltage level and 150000000Hz frequency */;
    FMU0->FCTRL = (FMU0->FCTRL & ~((uint32_t)FMU_FCTRL_RWSC_MASK)) | (FMU_FCTRL_RWSC(0x3U));
    /* Specifies the 1.2V operating voltage for the SRAM's read/write timing margin */
    SPC0->SRAMCTL |= SPC_SRAMCTL_REQ_MASK;
    while ((SPC0->SRAMCTL & SPC_SRAMCTL_ACK_MASK) == 0UL){
        /* Wait until acknowledged */
    }
    SPC0->SRAMCTL &= ~SPC_SRAMCTL_REQ_MASK;

    //CLOCK_SetupExtClocking(BOARD_XTAL0_CLK_HZ);
    /* Enable LDO */
    SCG0->LDOCSR |= SCG_LDOCSR_LDOEN_MASK;
    /* Select SOSC source (internal crystal oscillator) and Configure SOSC range */
    SCG0->SOSCCFG = SCG_SOSCCFG_EREFS_MASK | SCG_SOSCCFG_RANGE(1);
    /* Unlock SOSCCSR */
    SCG0->SOSCCSR &= ~SCG_SOSCCSR_LK_MASK;
    /* Enable SOSC */
    SCG0->SOSCCSR |= SCG_SOSCCSR_SOSCEN_MASK;
    while ((SCG0->SOSCCSR & SCG_SOSCCSR_SOSCVLD_MASK) == 0U) {
        /* Wait for SOSC clock to be valid. */
    }

    /* Configure PLL0 */
    SCG0->APLLCSR &= ~(SCG_APLLCSR_APLLPWREN_MASK | SCG_APLLCSR_APLLCLKEN_MASK);
    /* Write PLL setup data */
    SCG0->APLLCTRL  = SCG_APLLCTRL_SOURCE(0U) | SCG_APLLCTRL_SELI(27U) | SCG_APLLCTRL_SELP(13U);
    SCG0->APLLNDIV  = SCG_APLLNDIV_NDIV(4U);
    SCG0->APLLNDIV  = SCG_APLLNDIV_NDIV(4U) | (1UL << SCG_APLLNDIV_NREQ_SHIFT);
    SCG0->APLLPDIV  = SCG_APLLPDIV_PDIV(1U);
    SCG0->APLLPDIV  = SCG_APLLPDIV_PDIV(1U) | (1UL << SCG_APLLPDIV_PREQ_SHIFT);
    SCG0->APLLMDIV  = SCG_APLLMDIV_MDIV(50U);
    SCG0->APLLMDIV  = SCG_APLLMDIV_MDIV(50U) | (1UL << SCG_APLLMDIV_MREQ_SHIFT);
    SCG0->APLLSSCG0 = 0;
    SCG0->APLLSSCG1 = 0;

    /* Unlock APLLLOCK_CNFG register */
    SCG0->TRIM_LOCK = 0x5a5a0001;

    SCG0->APLLLOCK_CNFG = SCG_APLLLOCK_CNFG_LOCK_TIME(3300);
    SCG0->APLLCSR |= (SCG_APLLCSR_APLLPWREN_MASK | SCG_APLLCSR_APLLCLKEN_MASK);
    while((SCG0->APLLCSR & SCG_APLLCSR_APLL_LOCK_MASK) == 0UL){}

    /* Set up clock selectors  */
    SCG0->RCCR = (SCG0->RCCR & ~(SCG_RCCR_SCS_MASK)) | SCG_RCCR_SCS(5);
    while ((SCG0->CSR & SCG_CSR_SCS_MASK) != SCG_CSR_SCS(5)){}

    /* Set SystemCoreClock variable */
    SystemCoreClock = BOARD_BOOTCLOCKPLL150M_CORE_CLOCK;
	//Set pll_clk_div to pll0_clk/3 = 150MHz/3 = 50MHz
    //This is default for FlexCOMM clocks
	SYSCON->PLLCLKDIVSEL = SYSCON_PLLCLKDIVSEL_SEL(0);
	SYSCON->PLLCLKDIV = SYSCON_PLLCLKDIV_DIV(2);

}

