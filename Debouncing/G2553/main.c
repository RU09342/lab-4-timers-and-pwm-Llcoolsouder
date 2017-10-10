#include <msp430g2553.h>


/**
 * main.c
 */

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	P1DIR |= BIT0;				// Set P1.0 (LED) to out
	P1OUT |= BIT0;				// Init LED to HIGH
	P1REN |= BIT3;
	P1IE |= BIT3;				// Enable interrupt on P1.3 (Btn)
	P1IFG &= ~BIT3;				//Clear Btn interrupt flag

	TACTL |=    TASSEL_2 |      // Select clock source, clk divider, clk mode, overflow interrupt
		            ID_1 |
		            MC_1;
	TACCR0 = 15000;				// Set clock period
	TACCTL0 |= CCIE;			// Enable Timer A capture/compare interrupt 0
	TACCR1 = 60000;				// Set clock period
	TACCTL1 |= MC_0 | CCIE;		// Enable Timer A capture/compare interrupt 1
	_BIS_SR(GIE | LPM1_bits);

	return 0;
}


// P1.3 (Button) Service routine
#pragma vector = PORT1_VECTOR
__interrupt void PORT_1(void) {
	P1IE &= ~BIT3;
	TA1CTL = TASSEL_2 | MC_1 | TACLR;	//SMCLk, Up mode, clear TA1R
	switch (TACCR0) {
		case (65000) : TACCR0 = 15000; break;
		case (15000) : TACCR0 = 30000; break;
		case (30000) : TACCR0 = 65000; break;
	};
	P1IFG &= ~BIT3;
}

// TimerA0 Interrupt Service Routine
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0(void) {
	P1OUT ^= BIT0;				// Toggle LED
	TACCTL0 &= ~CCIFG;
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void Timer_A1(void) {
	TA1CTL &= ~(MC1 | MC0); 	// Stop TimerA1
	P1IE |= BIT3;
	TACCTL1 &= ~CCIFG;
}
