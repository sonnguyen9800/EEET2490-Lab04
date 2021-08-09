#include "mbox.h"
#include "uart.h"


void main()
{
    // set up serial console
	uart_init();

	// say hello
	uart_puts("Hello World!\n");

	// mailbox data buffer: Read ARM frequency
	mbox[0] = 8*4;          // Message Buffer Size in bytes (8 elements * 4 bytes (32 bit) each)
	mbox[1] = MBOX_REQUEST; // Message Request Code (this is a request message)
	mbox[2] = 0x00030002;   // TAG Identifier: Get clock rate
	mbox[3] = 8;            // Value buffer size in bytes (max of request and response lengths)
	mbox[4] = 0;			// REQUEST CODE = 0
	mbox[5] = 3;			// clock id: ARM system clock
	mbox[6] = 0;            // clear output buffer (response data are mbox[5] & mbox[6])
	mbox[7] = MBOX_TAG_LAST;

	if (mbox_call(ADDR(mbox), MBOX_CH_PROP)) {
		uart_puts("Response Code for whole message: ");
		uart_hex(mbox[1]);
		uart_puts("\n");

		uart_puts("Response Code in Message TAG: ");
		uart_hex(mbox[4]);
		uart_puts("\n");

		uart_puts("DATA: ARM clock rate = ");
		uart_dec(mbox[6]);
		uart_puts("\n\n");
	} else {
		uart_puts("Unable to query!\n");
	}



	// mailbox data buffer: Read ARM frequency
	mbox[0] = 8*4;          // Message Buffer Size in bytes (8 elements * 4 bytes (32 bit) each)
	mbox[1] = MBOX_REQUEST; // Message Request Code (this is a request message)
	mbox[2] = 0x00030002;   // TAG Identifier: Get clock rate
	mbox[3] = 8;            // Value buffer size in bytes (max of request and response lengths)
	mbox[4] = 0;			// REQUEST CODE = 0
	mbox[5] = 2;			// clock id: UART clock rate
	mbox[6] = 0;            // clear output buffer (response data are mbox[5] & mbox[6])
	mbox[7] = MBOX_TAG_LAST;

	if (mbox_call(ADDR(mbox), MBOX_CH_PROP)) {
		uart_puts("Response Code for whole message: ");
		uart_hex(mbox[1]);
		uart_puts("\n");

		uart_puts("Response Code in Message TAG: ");
		uart_hex(mbox[4]);
		uart_puts("\n");

		uart_puts("DATA: UART clock rate = ");
		uart_dec(mbox[6]);
		uart_puts("\n\n");
	} else {
		uart_puts("Unable to query!\n");
	}


	//Read Board Revision
	mbox[0] = 7*4;          // Message Buffer Size in bytes (9 elements * 4 bytes (32 bit) each)
	mbox[1] = MBOX_REQUEST; // Message Request Code (this is a request message)

	mbox[2] = 0x00010002;   // TAG Identifier: Get clock rate
	mbox[3] = 4;            // Value buffer size in bytes (max of request and response lengths)
	mbox[4] = 0;			// REQUEST CODE = 0
	mbox[5] = 0;            // clear output buffer
	mbox[6] = MBOX_TAG_LAST;

	if (mbox_call(ADDR(mbox), MBOX_CH_PROP)) {
		uart_puts("Response Code for whole message: ");
		uart_hex(mbox[1]);
		uart_puts("\n");

		uart_puts("Response Code in Message TAG: ");
		uart_hex(mbox[4]);
		uart_puts("\n");

		uart_puts("DATA: board revision = ");
		uart_hex(mbox[5]);
		uart_puts("\n\n");

	} else {
		uart_puts("Unable to query!\n");
	}


    // echo everything back
    while(1) {
    	//read each char
    	char c = uart_getc();

    	//send back twice
    	uart_sendc(c);
    	uart_sendc(c);
    }
}
