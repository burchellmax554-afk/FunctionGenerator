/******************************************************************************************
 * BasicIO.c - is a module with public functions used to send and receive
 * information from a serial port. In this case UART2 configured for the
 * MCULink debug USB serial port. FRDM-MCXN947 board.
 * v1.1
 *  Created by: Todd Morton, 05/04/2024
 *******************************************************************************************
* Project master header file
********************************************************************/
#include "MCUType.h"
#include "BasicIO.h"
#include "math.h"

/*******************************************************************************************
* Private Resources
*******************************************************************************************/
static INT8C bioHtoA(INT8U hnib);   //Convert nibble to ascii
static INT8U bioIsHex(INT8C c);
static INT8U bioHtoB(INT8C c);
/*******************************************************************************************
 * void BIOOpen(INT8U rate) - Initializes UART to operate at a specified rate.
 * MCU: MCXN947, LPUART4 configured for debugger USB.
 * Clock: Assumes connection to pll_clk_div is set to pll0_clk/3 = 150MHz/3 = 50MHz
 * Acceptable rates:
 *  BIO_BIT_RATE_9600
 *  BIO_BIT_RATE_19200
 *  BIO_BIT_RATE_38400
 *  BIO_BIT_RATE_57600
 *  BIO_BIT_RATE_115200
 ******************************************************************************************/
void BIOOpen(INT8U rate){

	SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRL0_PORT1(1);
	SYSCON->FCCLKSEL[4] = SYSCON_FCCLKSEL_SEL(1);			//PLL div clk, 50MHz
	SYSCON->AHBCLKCTRLSET[1] = SYSCON_AHBCLKCTRL1_FC4(1);

    PORT1->PCR[8]=PORT_PCR_MUX(2)|PORT_PCR_IBE(1);    //ties P1_8 to RxD, enable buffer
    PORT1->PCR[9]=PORT_PCR_MUX(2)|PORT_PCR_IBE(1);    //ties P1_9 to TxD, enable buffer
    LP_FLEXCOMM4->PSELID = LP_FLEXCOMM_PSELID_PERSEL(1); //make flexcomm 4 a uart
    //Software reset
    LPUART4->GLOBAL |= LPUART_GLOBAL_RST_MASK;
    LPUART4->GLOBAL &= ~LPUART_GLOBAL_RST_MASK;

    switch(rate){
    case(BIO_BIT_RATE_9600):
        LPUART4->BAUD = LPUART_BAUD_SBR(168)|LPUART_BAUD_OSR(30); //30 results in OSR of 31
        break;
    case(BIO_BIT_RATE_19200):
		LPUART4->BAUD = LPUART_BAUD_SBR(84)|LPUART_BAUD_OSR(30);
        break;
    case(BIO_BIT_RATE_38400):
		LPUART4->BAUD = LPUART_BAUD_SBR(42)|LPUART_BAUD_OSR(30);
        break;
    case(BIO_BIT_RATE_57600):
		LPUART4->BAUD = LPUART_BAUD_SBR(28)|LPUART_BAUD_OSR(30);
        break;
    case(BIO_BIT_RATE_115200):
		LPUART4->BAUD = LPUART_BAUD_SBR(14)|LPUART_BAUD_OSR(30);
        break;
    default:    //Default to 115200bps
		LPUART4->BAUD = LPUART_BAUD_SBR(14)|LPUART_BAUD_OSR(30);
        break;
    }

    /* Enable tx/rx FIFO */
    /* The FIFO is only 8 words so this sends the first 8 words without delay.
     * But after that, there's a delay for each character. */

    LPUART4->WATER = LPUART_WATER_RXWATER(0) | LPUART_WATER_TXWATER(7);
    LPUART4->FIFO |= (LPUART_FIFO_TXFE_MASK | LPUART_FIFO_RXFE_MASK);

    /* Flush FIFO */
    LPUART4->FIFO |= (LPUART_FIFO_TXFLUSH_MASK | LPUART_FIFO_RXFLUSH_MASK);

    /* Enable RxD and TxD */
    LPUART4->CTRL |= LPUART_CTRL_TE_MASK|LPUART_CTRL_RE_MASK;

}

/*******************************************************************************************
* BIORead() - Checks for a character received
* MCU: MCXN947, LPUART4
*    return: ASCII character received or 0 if no character received
*******************************************************************************************/
INT8C BIORead(void){
    INT8C c;
    if ((LPUART4->STAT & LPUART_STAT_RDRF_MASK) != 0){   //check if char received
        c = (INT8C)(LPUART4->DATA);
    }else{
        c = '\0';                           //If not return 0
    }
    return (c);
}
/*******************************************************************************************
* BIOGetChar() - Blocks until character is received
*    return: INT8C ASCII character
*******************************************************************************************/
INT8C BIOGetChar(void){
    INT8C c;
    do{
        c = BIORead();
    }while(c == '\0');
    return c;
}

/*******************************************************************************************
* BIOWrite() - Sends an ASCII character
*              Blocks as much as one character time after FIFO is full
* MCU: MCXN947, LPUART4
*    parameter: c is the ASCII character to be sent
*******************************************************************************************/
void BIOWrite(INT8C c){
    while ((LPUART4->STAT & LPUART_STAT_TDRE_MASK)==0){} //waits for space on FIFO
    LPUART4->DATA = (INT32U)c;
}

/*******************************************************************************************
* BIOPutStrg() - Writes a string to monitor
*    parameter: strg is a pointer to the ASCII string
*******************************************************************************************/
void BIOPutStrg(const INT8C *const strg){
    const INT8C *strgptr = strg;
    while (*strgptr != '\0'){              //until a null is reached
        BIOWrite(*strgptr);
        strgptr++;
    }
}

/*******************************************************************************************
* BIOOutDecWord() - Outputs a decimal value of a 32-bit word.
*    Parameters: binword is the word to be sent,
*                field is the maximum number of digits to be shown. Range 1-10. field
*                   starts at cursor.
*                mode determines the behavior of field and binword,
*                3 modes:
*                   1. BIO_OD_MODE_LZ: Shows leading zeros (digits will use the entire field).
*                   2. BIO_OD_MODE_AR: Aligns binword to rightmost field digits.
*                   3. BIO_OD_MODE_AL: Aligns binword to leftmost field digits.
*    Examples:
*    binword = 123, field = 5, mode = BIO_OD_MODE_LZ, Result: 00123
*    binword = 123, field = 5, mode = BIO_OD_MODE_AR, Result: __123 (_'s are spaces)
*    binword = 123, field = 5, mode = BIO_OD_MODE_AL, Result: 123__
*    binword = 123, field = 2, mode = BIO_OD_MODE_LZ, Result: --    (binword exceeds field)

*******************************************************************************************/
void BIOOutDecWord (INT32U binword, INT8U field, BIO_OUTDEC_MODE mode){
    INT8C digitstrg[11];
    INT32U lbinword = binword;
    INT8U num_digits = field;
    INT8U digit_index;
    INT8U val_index;

    //Clamp field size to acceptable values
    if(num_digits > 10){
        num_digits = 10;
    }else if(num_digits < 1){
        num_digits = 1;
    }else{
    }
   //Calculate the digits needed for lbinword
    digit_index = num_digits + 1;
    digit_index--;
    digitstrg[digit_index] = '\0';   //always ends in null
    while((digit_index > 0) && (lbinword > 0)){
        digit_index--;
        digitstrg[digit_index] = (INT8C)((lbinword % 10) +'0');
        lbinword = lbinword/10;
    }

    if(digit_index == num_digits){       //always at least a '0'
        digit_index--;
        digitstrg[digit_index] = '0';
    }else{
    }
    if(lbinword > 0){  //Writes '-' to all field slots if bin length exceeded field
        digit_index = 0;
        while(digit_index < num_digits){
            digitstrg[digit_index] = '-';
            digit_index++;
        }
        digitstrg[digit_index] = '\0';
        BIOPutStrg(digitstrg);
    }else{
        if((mode == BIO_OD_MODE_AR) || (mode == BIO_OD_MODE_LZ)){   //align right so fill rest with spaces to clear
            while(digit_index > 0){
                digit_index--;
                if(mode == BIO_OD_MODE_AR){
                    digitstrg[digit_index] = ' ';
                }else if(mode == BIO_OD_MODE_LZ){
                    digitstrg[digit_index] = '0';
                }else{
                }
            }
            BIOPutStrg(digitstrg);

        }else if(mode == BIO_OD_MODE_AL){
            val_index = digit_index;
            digit_index = 0;
            while(val_index < (num_digits)){
                digitstrg[digit_index] = digitstrg[val_index];
                val_index++;
                digit_index++;
            }
            while(digit_index < num_digits){
                digitstrg[digit_index] = ' ';
                digit_index++;
            }
            digitstrg[(digit_index)] = '\0';
            BIOPutStrg(digitstrg);
        }else{
        }
    }
}

/*******************************************************************************************
* BIOGetStrg() - Inputs a string and stores it into an array.
*
* Description: A routine that inputs a character string to an array until a carriage return
*              is received or strglen is exceeded.
*              Only printable characters are recognized except carriage return and backspace
*              Backspace erases displayed character and array character.
*              A NULL is always placed at the end of the string.
*              All printable characters are echoed.
* Return value: 0 -> if ended with CR
*               1 -> if strglen exceeded.
* Arguments: *strg is a pointer to the string array
*            strglen is the max string length, includes CR/NULL.
*******************************************************************************************/
INT8U BIOGetStrg(INT8U strglen,INT8C *const strg){
   INT8U charnum = 0;
   INT8C c;
   INT8U rvalue;
   c = BIOGetChar();
   while((c != '\r') && ((charnum < (strglen)))){
       if((' ' <= c) && ('~' >= c) && (charnum != (strglen-1))){
           BIOWrite(c);
           strg[charnum] = c;
           charnum++;
           c=BIOGetChar();
       }else if((c == '\b') && (charnum > 0)){
           BIOWrite('\b');
           BIOWrite(' ');
           BIOWrite('\b');
    	   charnum--;
           c=BIOGetChar();
       }else if((' ' <= c) && ('~' >= c) && (charnum == (strglen-1))){
    	   charnum++;
       }else{ /*non-printable character or BS at first character - ignore */
           c=BIOGetChar();
       }
   }
   BIOOutCRLF();
   if(c == '\r'){
       rvalue = 0;
       strg[charnum] = '\0';
   }else{
       rvalue = 1;
   }
   return rvalue;
}

/*******************************************************************************************
* BIOOutCRLF() - Outputs a carriage return and line feed.
*
********************************************************************/
void BIOOutCRLF(void){
    BIOPutStrg("\r\n");
}

/*******************************************************************************************
* BIOHexStrgtoWord() - Converts a string of hex characters to a 32-bit
*                      word until NULL is reached.
* Return value: 0 -> if no error.
*               1 -> if string is too long for word.
*               2 -> if a non-hex character is in the string.
*               3 -> No characters in string. Started with NULL.
* Arguments: *strg is a pointer to the string array
*            *bin is the word that will hold the converted string.
*******************************************************************************************/
INT8U BIOHexStrgtoWord(INT8C *const strg,INT32U *bin){
    INT8U cnt = 0;
    INT32U lbin = 0;
    INT8C *strgptr = strg;
    INT8U rval = 0;
    if(*strgptr == '\0'){
        rval = 3;
    }else{
        while(*strgptr != '\0'){
            if(bioIsHex(*strgptr) != 0){
                lbin = (lbin << 4) | (INT32U)(bioHtoB(*strgptr));
            }else{
                rval = 2;
            }
            strgptr++;
            cnt++;
            if(cnt > 8){
                rval = 1;
            }else{
            }
        }
        *bin = lbin;
    }
    return rval;
}

/*******************************************************************************************
* BIOOutHexByte() - Output one byte in hex.
* bin is the byte to be sent
*******************************************************************************************/
void BIOOutHexByte(INT8U bin){
    BIOWrite(bioHtoA(bin>>4));
    BIOWrite(bioHtoA(bin & 0x0fu));
}

/*******************************************************************************************
* BIOOutHexHWord() - Output 16-bit word in hex.
* bin is the word to be sent
*******************************************************************************************/
void BIOOutHexHWord(INT16U bin){
    BIOOutHexByte((INT8U)(bin>>8));
    BIOOutHexByte((INT8U)(bin & 0x00ffu));
}
/*******************************************************************************************
* BIOOutHexWord() - Output 32-bit word in hex.
* bin is the word to be sent
* Todd Morton, 10/14/2014
*******************************************************************************************/
void BIOOutHexWord(INT32U bin){
    BIOOutHexByte((INT8U)(bin>>24));
    BIOOutHexByte((INT8U)(bin>>16));
    BIOOutHexByte((INT8U)(bin>>8));
    BIOOutHexByte((INT8U)(bin & 0x000000ff));
}
/*******************************************************************************************
* bioIsHex() - Checks for hex ascii character - private
* returns 1 if hex and 0 if not hex.
* Todd Morton, 10/14/2014
*******************************************************************************************/
static INT8U bioIsHex(INT8C c){
    INT8U rval;
    if((('0' <= c) && ('9' >= c)) || (('a' <= c) && ('f' >= c)) || (('A' <= c) && ('F' >= c))){
        rval = 1;
    }else{
        rval = 0;
    }
    return rval;
}

/*******************************************************************************************
* bioHtoB() - Converts a hex ascii character to a binary byte - private
* c is the ascii character to be converted.
* returns the binary value.
* Note: it returns a 0 if it is not a hex character - this should be fixed.
* Todd Morton, 10/14/2014
*******************************************************************************************/
static INT8U bioHtoB(INT8C c){
    INT8U bin;
    if(('0' <= c) && ('9' >= c)){
        bin = (INT8U)(c - '0');
    }else if(('a' <= c) && ('f' >= c)){
        bin = (INT8U)(c - 'a' + 0xa);
    }else if(('A' <= c) && ('F' >= c)){
        bin = (INT8U)(c - 'A' + 0xa);
    }else{
        bin = 0;
    }
    return bin;
}
/*******************************************************************************************
* bioHtoA() - Converts a hex nibble to ASCII - private
* hnib is the byte with the LSN to be sent
* Todd Morton, 10/14/2014
*******************************************************************************************/
static INT8C bioHtoA(INT8U hnib){
    INT8C asciic;
    INT8U hnmask = hnib & 0x0fu; /* Take care of any upper nibbles */
    if(hnmask <= 9U){
        asciic = (INT8C)(hnmask + 0x30U);
    }else{
        asciic = (INT8C)(hnmask + 0x37U);
    }
    return asciic;
}
