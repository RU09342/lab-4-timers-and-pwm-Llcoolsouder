# Software Debouncing
In previously labs, we talked about how objects such as switches can cause some nasty effects since they are actually a mechanical system at heart. This part of the lab addresses one way to solve the problem of _bouncing_ which is where a button literally bounces due to elasticity of the contact material being quickly released from tension. The resulting signal is shown below.

![Scope screenshot of a button release](https://github.com/RU09342/lab-4-timers-and-pwm-Llcoolsouder/blob/master/Debouncing/Bouncing.png)

One way to eliminate bouncing is through a hardware implemented RC filter. Unfortunately, this is not always an option. Another option, is to write code that debounces a button. This is what is shown in the code below. The code below is written specifically for the MSP430G2553.

```
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	P1DIR |= BIT0;				// Set P1.0 (LED) to out
	P1OUT |= BIT0;				// Init LED to HIGH
	P1REN |= BIT3;
	P1IE |= BIT3;				// Enable interrupt on P1.3 (Btn)
	P1IFG &= ~BIT3;				// Clear Btn interrupt flag

	TA0CTL |=   TASSEL_2 |      // Select clock source, clk divider, clk mode, overflow interrupt
		        ID_1 |
		        MC_1;
	TA0CCR0 = 15000;			// Set clock period
	TA0CCTL0 |= CCIE;			// Enable Timer A capture/compare interrupt 0

	TA1CTL |=	TASSEL_2 |
				ID_2 |
				MC_0;
	TA1CCR0 = 60000;			// Set clock period
	TA1CCTL0 |= CCIE;			// Enable Timer A capture/compare interrupt 1

	_BIS_SR(GIE | LPM1_bits);

	return 0;
}


// P1.3 (Button) Service routine
#pragma vector = PORT1_VECTOR
__interrupt void PORT_1(void) {
	P1IE &= ~BIT3;
	TA1CTL = TASSEL_2 | MC_1 | TACLR;	//SMCLK, Up mode, clear TA1R
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
	TA0CCTL0 &= ~CCIFG;
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void Timer_A1(void) {
	TA1CTL &= ~(MC1 | MC0); 	// Stop TimerA1
	P1IE |= BIT3;
	TA1CCTL0 &= ~CCIFG;
}
```
The code above isn't anything that hasn't already been seen in Lab 3.  It is simply a new combination of things already learned. As an __extra__ bonus, this code usesa second timer in order to blink the LED and cycle through 3 different speeds which are toggled by the newly debounced button. The basical algorithm is this:
1. Set up two timers: one to toggle the LED on interrupts, T0 and another to disable the button for some amount of time after being pushed, T1. (This one is not started on initialization. It is only set up.)
2. When the button is pushed, disable the button interrupt, start T1, and toggle the new frequency for T0.
3. When T1 triggers an interrupt, it reenables the button's interrupt and stops itself. NOTE: The period for T1 should be sufficiently long to wait out the bouncing on the button.
4. When T0 triggers an interrupt it, toggles the LED. This runs independent of everything else, so the LED will keep blinking regardless of what else is going on.
