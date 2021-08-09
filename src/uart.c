#include "gpio.h"
#include "uart.h"
#include "mbox.h"


/**
 * Set baud rate and characteristics (115200 8N1) and map to GPIO
 */
void uart_init()
{
    register unsigned int r;

#if UART_TYPE == 1
    /* initialize UART */
    *AUX_ENABLE |= 1;       //enable UART1, AUX mini uart (AUXENB)
    *AUX_MU_CNTL = 0;		//stop transmitter and receiver
    *AUX_MU_LCR  = 3;       //8-bit mode
    *AUX_MU_MCR  = 0;		//RTS (request to send)
    *AUX_MU_IER  = 0;		//disable interrupts
    *AUX_MU_IIR  = 0xc6;    //clear FIFOs
    *AUX_MU_BAUD = 270;    	// 115200 baud

    /* map UART1 to GPIO pins */
    r = *GPFSEL1;
    r &= ~( (7 << 12)|(7 << 15) ); //Clear bits 12-17 (gpio14, gpio15)
    r |=    (2 << 12)|(2 << 15);   //Set value 2 (select ALT5: UART1)
    *GPFSEL1 = r;

    /* enable GPIO 14, 15 */
    *GPPUD = 0;            //No pull up/down control
    r = 150; while(r--) { asm volatile("nop"); } //waiting 150 cycles
    *GPPUDCLK0 = (1 << 14)|(1 << 15); //enable clock for GPIO 14, 15
    r = 150; while(r--) { asm volatile("nop"); } //waiting 150 cycles
    *GPPUDCLK0 = 0;        // flush GPIO setup

    *AUX_MU_CNTL = 3;      //Enable transmitter and receiver (Tx, Rx)

#else //UART 0

    /* Turn off UART0 */
	*UART0_CR = 0x0;

	/* Set up clock for consistent divisor values
	 * Updated Note: setting clock rate have no effect in QEMU (get clock rate always receive a fixed value).
	 * With latest test, the uart0 still work well without clock setting.
	 * However, we can keep it for reference (more meaningful with real board to have fixed clock rate) */

    mbox[0] = 9*4;					// Message Buffer Size in bytes (8 elements * 4 bytes (32 bit) each)
    mbox[1] = MBOX_REQUEST;			// Message Request Code (this is a request message)
    mbox[2] = MBOX_TAG_SETCLKRATE; 	// TAG Identifier: set clock rate
    mbox[3] = 12;					// Value buffer size in bytes (max of request and response lengths)
    mbox[4] = 0;			// REQUEST CODE = 0
    mbox[5] = 2;           	// clock id: UART clock
    mbox[6] = 4000000;     	// rate (in Hz): 4Mhz
    mbox[7] = 0;           	// skip setting turbo
    mbox[8] = MBOX_TAG_LAST;
    mbox_call(ADDR(mbox), MBOX_CH_PROP);

	/* Map UART0 to GPIO pins 14 and 15 */

	/* Set GPIO14 and GPIO15 to be pl011 TX/RX which is ALT0	*/
	r = *GPFSEL1;
	r &= ~((7 << 12) | (7 << 15));		// Clear the gpio 14 and 15
	r |=    (4 << 12)|(4 << 15);   //Set value 4 (select ALT0: UART0)
	*GPFSEL1 = r;

	/* enable GPIO 14, 15 */
	*GPPUD = 0;            //No pull up/down control
	r = 150; while(r--) { asm volatile("nop"); } //waiting 150 cycles
	*GPPUDCLK0 = (1 << 14)|(1 << 15); //enable clock for GPIO 14, 15

	r = 150; while(r--) { asm volatile("nop"); } //waiting 150 cycles
	*GPPUDCLK0 = 0;        // flush GPIO setup

	/* Mask all interrupts. Clear all the interrupts */
	*UART0_IMSC = 0;

	/* Clear pending interrupts. Clear all the interrupts */
	*UART0_ICR = 0x7FF;


	/* Set integer & fractional part of baud rate.
	 Divider = UART_CLOCK/(16 * Baud)
	 Integer part register UART0_IBRD  = integer part of Divider
	 Fraction part register UART0_FBRD = (Fractional part * 64) + 0.5
	 UART_CLOCK = 4MHz (default is 3MHz); Baud = 115200.

	 --> Divider = 4000000/(16*115200) = 2.170138889
	 --> UART0_IBRD = 2
	 	 UART0_FBRD = (0.170138889 * 64) + 0.5 = 11.3 (take 11)*/

	*UART0_IBRD = 2;       // 115200 baud
	*UART0_FBRD = 0xB;


	/* Set up the Line Control Register */
	/* Enable FIFO */
	/* Set length to 8 bit */
	/* Defaults for other bit are No parity, 1 stop bit */
    *UART0_LCRH = UART0_LCRH_FEN | UART0_LCRH_WLEN_8BIT;

	/* Enable UART0, receive, and transmit */
	*UART0_CR = 0x301;     // enable Tx, Rx, FIFO 0b0011 0000 0001

#endif

}

/**
 * Send a character
 */
//void uart_sendc(unsigned char c) {

void uart_sendc(unsigned char c){
    /* wait until transmitter is empty */

#if UART_TYPE == 1
    do {
    	asm volatile("nop");
    } while ( !(*AUX_MU_LSR & 0x20) );

    /* write the character to the buffer */
    *AUX_MU_IO = c;
#else //UART 0
    /* Check Flags Register */
   	/* And wait until transmitter is not full */
   	do {
   		asm volatile("nop");
   	} while (*UART0_FR & UART0_FR_TXFF);

   	/* Write our data byte out to the data register */
   	*UART0_DR = c ;

#endif

}

/**
 * Receive a character
 */
char uart_getc() {
    char c;

#if UART_TYPE == 1
    /* wait until data is ready (one symbol) */
    do {
    	asm volatile("nop");
    } while ( !(*AUX_MU_LSR & 0x01) );

    /* read it and return */
    c = (char)(*AUX_MU_IO);

#else //UART 0
    /* Check Flags Register */
       /* Wait until Receiver is not empty
        * (at least one byte data in receive fifo)*/
   	do {
   		asm volatile("nop");
       } while(*UART0_FR&0x10); //while ( *UART0_FR & UART0_FR_RXFE );

       /* read it and return */
       //c = (unsigned char) (*UART0_DR);
       c = (unsigned char) (*UART0_DR);
#endif

    /* convert carriage return to newline */
    return (c == '\r' ? '\n' : c);
}

/**
 * Display a string
 */
void uart_puts(char *s) {
    while (*s) {
        /* convert newline to carriage return + newline */
        if (*s == '\n')
            uart_sendc('\r');
        uart_sendc(*s++);
    }
}



/**
 * Display a binary value in hexadecimal
 */
void uart_hex(unsigned int d) {
    unsigned int n;
    int c;

    uart_puts("0x");
    for (c = 28; c >= 0; c = c - 4) {
        // Get highest 4-bit nibble
        n = (d >> c) & 0xF;

        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n += (n > 9) ? (- 10 + 'A') : '0';
        uart_sendc(n);
    }
}

/**
 * Display a value in decimal by
 */
void uart_dec(int num)
{
	char str[33] = "";
	int i, rem, len = 0, n;

    n = num;
    while (n != 0){
        len++;
        n /= 10;
    }

    if (num == 0)
    	len = 1;

    for (i = 0; i < len; i++){
        rem = num % 10;
        num = num / 10;
        str[len - (i + 1)] = rem + '0';
    }
    str[len] = '\0';

    uart_puts(str);
}

