#include <msp430g2553.h>


/**
 * main.c
 */

enum Trigger {Rising_Edge, Falling_Edge};
enum Trigger edge = Rising_Edge;

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	P1DIR |= BIT0 | BIT6;		// Set P1.0 (LED) and P1.6 (Ind) to out
	P1OUT |= BIT0;				// Init LED to HIGH
	P1OUT &= ~BIT6;				// Init Indicator to LOW
	P1REN |= BIT3;
	P1IE |= BIT3;				// Enable interrupt on P1.3 (Btn)
	P1IFG &= ~BIT3;				// Clear Btn interrupt flag

	TA0CTL |=   TASSEL_2 |      // Select clock source, clk divider, clk mode, overflow interrupt
		        ID_1 |
		        MC_3;
	TA0CCR0 = 500;				// Set clock period
	TA0CCTL0 |= CCIE;			// Enable Timer A capture/compare interrupt 0

	TA0CCR1 = 0;
	TA0CCTL1 |= CCIE;

	TA1CTL |=	TASSEL_2 |
				ID_3 |
				MC_1;
	TA1CCR0 = 60000;			// Set clock period
	TA1CCTL0 |= CCIE;			// Enable Timer A capture/compare interrupt 1

	_BIS_SR(GIE);

	return 0;
}


// P1.3 (Button) Service routine
#pragma vector = PORT1_VECTOR
__interrupt void PORT_1(void) {
	P1OUT ^= BIT6;
	P1IE &= ~BIT3;
	TA1CTL = TASSEL_2 | MC_1 | TACLR;	//SMCLK, Up mode, clear TA1R
	if (edge == Rising_Edge){
		edge = Falling_Edge;
		P1IES |= BIT3;					//trigger on neg edge
		TA0CCR1 += 50;
		if (TA0CCR1 == 550)
			TA0CCR1 = 0;
	}
	else {
		edge = Rising_Edge;
		P1IES &= ~BIT3;					//trigger on pos edge
	}

	P1IFG &= ~BIT3;

}

// TimerA0 Interrupt Service Routine
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0(void) {
	//P1OUT ^= BIT0;				// Toggle LED
	TA0CCTL0 &= ~CCIFG;
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void TimerA1(void) {
	P1OUT ^= BIT0;
	TA0CCTL1 &= ~CCIFG;
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void Timer_1A1(void) {
	TA1CTL &= ~(MC1 | MC0); 	// Stop TimerA1
	P1IE |= BIT3;
	TA1CCTL0 &= ~CCIFG;
}
