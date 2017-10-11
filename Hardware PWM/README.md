# Hardware PWM
This part of the lab is functionally an exact replica of [Software PWM](https://github.com/RU09342/lab-4-timers-and-pwm-Llcoolsouder/tree/master/Software%20PWM). However, this part uses the built in PWM hardware within the Timer A module on the MSP430. The code below is specifically for the MSP430G2553.

```
enum Trigger {Rising_Edge, Falling_Edge};
enum Trigger edge = Rising_Edge;

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	P1SEL |= BIT6;				// Set P1.6 to Timer Module
	P1DIR |= BIT0 | BIT6;		// Set P1.0 (LED) and P1.6 (Ind) to out
	P1OUT |= BIT6;				// Init LED to HIGH
	P1OUT &= ~BIT0;				// Init Indicator to LOW
	P1REN |= BIT3;
	P1IE |= BIT3;				// Enable interrupt on P1.3 (Btn)
	P1IFG &= ~BIT3;				// Clear Btn interrupt flag

	TA0CTL |=   TASSEL_2 |      // Select clock source, clk divider, clk mode, overflow interrupt
		        ID_1 |
		        MC_3;
	TA0CCR0 = 500;				// Set clock period

	TA0CCR1 = 0;
	TA0CCTL1 |= OUTMOD_3;		// Set/Reset mode

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
	P1OUT ^= BIT0;
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

#pragma vector = TIMER1_A0_VECTOR
__interrupt void Timer_1A1(void) {
	TA1CTL &= ~(MC1 | MC0); 	// Stop TimerA1
	P1IE |= BIT3;
	TA1CCTL0 &= ~CCIFG;
}
```

The only new things in the code above are 1.) I selected something other than GPIO for one of the pins, and 2.) TA0CCTL1 has an output mode other than default selected.

## Timer A Module
As mentioned before, you can select the function of a pin using PxSEL and PxSEL2. What exactly these bits need to be set to for different functions is different from chip to chip, so you'll have to go through the data sheet to find which pin has the function you need. For PWM, the function I needed was TA0.0. For the MSP430G2553, this function is available on P1.6 (and others) when PxSEL | PxSEL2 = 0b10. This connects the output of TimerA to P1.6.

## Out Mode
The output mode of the capture/compare module selects what happens to the signal upon a compare interrupt or on a timer overflow. This is what makes generating the PWM signal so easy. All of the signal toggling happens in the background through hardware without taking any extra computing power or cycles. The bits that control output mode are located in TAnCCTLx
