# Software PWM
Most microprocessors will have a Timer module, but depending on the device, some may not come with pre-built PWM modules. Instead, you may have to utilize software techniques to synthesize PWM on your own. In this part of the lab, I implemented a software PWM using the built in Timer to toggle an output at specific times, thus creating a PWM waveform. The specific code shown below is for the MSP430G2553.

```
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
```

Once again, the code above does not do anything that hasn't been done in previous labs. It is merely a new combination of concepts already learned. Here is how the code above works.
1. I initialize 2 pins as outputs for LEDs and one pin as an input for a button.
2. Timer A0 is initialized in Up/Down mode with a period of 1000 cycles (500 * 2)
3. Timer A1 is set up but not started as this will be used to debounce the button.
4. An interrupt is set on TimerA for TA0CCR1.
5. When TA0CCR1 triggers an interrupt, the LED toggles.
6. When the button is pushed, the value of TA0CCR1 is incremented by 50, thus increasing duty cycle by 10%. When duty cycle reaches 100%, it is reset to 0.

The resultant waveform is shown below.
![PWM Scope 1](https://github.com/RU09342/lab-4-timers-and-pwm-Llcoolsouder/blob/master/Software%20PWM/scope_PWM1.png)
![PWM Scope 2](https://github.com/RU09342/lab-4-timers-and-pwm-Llcoolsouder/blob/master/Software%20PWM/scope_PWM2.png)

## Duty Cycle
A PWM signal has two properties: _frequency_ and _duty cycle_. Frequency is simply the inverse of the time it takes for a full period of the wave. Duty cycle is what percentage of the period that the signal is HIGH. The general formula for duty cycle is given by:
<p align="center">
D = t<sub>H</sub> / T<sub>0</sub>
</p>
Duty cycle is directly proportional to power, so a lower duty cycle means less power from a signal. Duty cycle is often used to dim LEDs, control DC motors, and other things that require varying power.
