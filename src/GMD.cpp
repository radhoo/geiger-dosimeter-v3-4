/*
 * Geiger Dosimeter 3.4
 * http://www.pocketmagic.net 
 *
 * Copyright (c) 2013 by Radu Motisan , radu.motisan@gmail.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 *
 */

/*
 * CS can be configured in hw_enc28j60.h
 * Configure mymac and myip below
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include "timeout.h" // defines F_CPU

// lcd
#include "lcd/hd44780.h"

#include "spk.h"
#include "led.h"
volatile int
/* 
 * GEIGER 
 */
		g_nTimeSec = 0, g_nTimeMin = 0, g_nTimeHour = 0, 
		g_nTimeMilis = 0, gs_geiger_pulses = 0, /*gs_geiger_total = 0,*/
/* 
 * INVERTER
 */
		g_nPWMDuty = 100,//254 ,
		g_nTubeVoltage = 0,  
		g_nTubeVoltageThreshold = 400, tolv = 5,
		g_nCPM = 0, g_nLastCPM = 0, ready = 0;						
/* 
 * CPM TO uSv/h linear conversion : acceptable only for low doses of high energy gamma radiation
 * Background dose: 0.12uSv/h
 *
 * TUBE:
 * STS-1    18pulses/min=18cpm -> 0.12uSv/h;  factor = 0.12/18= 0.006
 * SBM-19   80pulses/min=80cpm -> 0.12uSv/h;  factor = 0.12/80= 0.0015
 */
float	usvhfactor = 0.006,	//preconfigured for given tube
		usvhold = 0;

bool 	bRefresh = true, bBeep = false;

// Reads the ADC port specified by i
uint16_t adcread(uint8_t i)
{
	ADMUX = i;
	ADCSRA = (1<<ADEN) | (1<<ADIF) | (1<<ADSC)|7;
	loop_until_bit_is_set(ADCSRA, ADIF);
	uint16_t v = ADCL;
	v|= (ADCH<<8);
	return v;
}

int readHV() {
	float	Vref		= 5.0; //Vref set to 5V
	float supplyadc = adcread(3);
	uint16_t R1 = 4600;//KO
	uint16_t R2 = 47;//KO
	return (int)( supplyadc / 1024.0 * (R1+R2) / R2 * Vref );
}

/*
	timer0 overflow interrupt
	event to be executed every  1.024ms here
*/
ISR (TIMER0_OVF_vect)  
{
	g_nTimeMilis ++;
	if (g_nTimeMilis >= 976) {
		bRefresh = true;
		 if (g_nTimeSec<59) {
			g_nTimeSec++; 
		 }			
		 else { 
			g_nTimeSec = 0; 
			// one full minute!
			g_nCPM = gs_geiger_pulses;
			ready = 1;
			//gs_geiger_total+=gs_geiger_pulses;
			// 
			gs_geiger_pulses = 0;
			
			if (g_nTimeMin<59) 
				g_nTimeMin++; 
			else {
				g_nTimeMin = 0;
				// one full hour!
				//gs_geiger_total = 0; // reset every hour
				if (g_nTimeHour<24)
					g_nTimeHour++;
				else 
					g_nTimeHour = 0;
			}				
		 }	
		 
		 g_nTimeMilis = 0; // 976 x 1.024ms ~= 1000 ms = 1sec
	}
}			

/*
	This interrupt handler is triggered each time a rising edge is
	detected on INT0, which on a ATmega8 is PD2
*/

ISR(INT0_vect) {
	gs_geiger_pulses++;
	bBeep = true;
}
 


/*
 * Main entry point
 */
int main(void) {
	
	_delay_ms(500);
	
	// Set ADCSRA Register in ATMega168
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1);

	// Set ADMUX Register in ATMega168
	ADMUX=0;
	
	_delay_ms(200);


	SPKInit();
	LEDInit();
	/*
	 *  1.CONFIGURE INTERRUPT INT0  to count pulses from Geiger Counter, connected on PIN PD2
	 */
	// Configure INT0 to trigger on rising edge
	MCUCR |= _BV(ISC00) | _BV(ISC01);
	// Configure INT0 to fire interrupts
	GICR |= _BV(INT0);
	
	/*
	 * 2.CREATE Timer T0 to count seconds, will trigger callback ISR (TIMER0_OVF_vect)  
	 */
	TIMSK |= (1 << TOIE0);
	// set prescaler to 64 and start the timer
	TCCR0 |= (1 << CS01) | (1 << CS00);
	// start timer and interrupts
	sei();
	// =====

	/*
	 * 3.CREATE PWM with Timer T1 to drive inverter for regulated voltage, needed for the Geiger tube
	 */
	TCCR1A = 0;     // disable all PWM on Timer1 whilst we set it up
	// set the frequency FCPU/(ICR1 * PRESCALLING) Hz 
	// 16MHz, prescalling 8x: 
	// ICR1 = 50; //40KHz
	// ICR1 = 100; //20KHz
	// ICR1 = 200; //10KHz
	// ICR1 = 1000;//2KHz
	
	// 16MHz, prescalling 1x:
	// ICR1 = 100;//160KHz
	// ICR1 = 400;//40KHz
	// ICR1 = 800;//20KHz
	ICR1 = 800;  // 20KHz 74
	
	// 8Mhz, prescalling 1x: (half as for 16Mhz)
	//ICR1 = 400; //20 KHz
	
	// Configure timer 1 for Fast PWM mode via ICR1, with no prescaling (1X)
	TCCR1B = (1 << WGM13) | (1<<WGM12) | (1 << CS10); //CS11 = 8x prescalling , CS10 = 1x prescalling
	// Set PB1 and PB2 as outputs
	DDRB |= _BV(1);
	TCCR1A |= (1 << COM1A1) | (1<< CS11); // set none-inverting mode and start PWM
	
	OCR1A = (int)((float)ICR1 * (float)g_nPWMDuty/1000.0); //50% g_nPWMDuty 
	
	// prepare LCD init
	_delay_ms(200);

	lcd_init();
	lcd_clrscr();
	lcd_string("Geiger Dosimeter  \npocketmagic.net    ");
	_delay_ms(1500);
	int i = 0;
	
	// program loop
	while(1){
		i++;
		
		// check voltage every two seconds, and adjust duty cycle to regulate voltage
		//if (g_nTimeSec % 2 == 0) {
			g_nTubeVoltage = readHV();
			if (g_nTubeVoltage >= g_nTubeVoltageThreshold + tolv) { // we need to decrease voltage
				if (g_nPWMDuty > 10) g_nPWMDuty --; //1%
				OCR1A = (int)((float)ICR1 * (float)g_nPWMDuty/1000.0); //
			}
			if (g_nTubeVoltage <= g_nTubeVoltageThreshold - tolv) { // we need to increase voltage
				if (g_nPWMDuty < 350) g_nPWMDuty ++;//35%
				OCR1A = (int)((float)ICR1 * (float)g_nPWMDuty/1000.0); //
			}
		//}

		float usvh = 0;
		
		// we compute the dose every 5 seconds
		if ((g_nTimeSec > 0) && g_nTimeSec % 5 == 0) {
			g_nLastCPM = (60.0 * gs_geiger_pulses) / g_nTimeSec;
			usvh = usvhfactor * g_nLastCPM;
			usvhold = usvh;
		}
		
		if (bRefresh) {
			bRefresh = false;

			// move LCD cursor back to home position: we prepare a new display content
			lcd_home();

			// 2x16 Output
			// first line: dose in microsieverts and in counts per minute
			// second line: voltage on tube, second counter, pulse counter, duty cycle
			
			//XXXXXXXXXXXXXXXX
			// 0.12uSvh 18CPM
			//400V 50s 18 245

			lcd_string_format("%2.2fuSvh %dCPM     \n%dV %ds %d %d    ",
				usvhold,g_nLastCPM,
				g_nTubeVoltage, g_nTimeSec,gs_geiger_pulses, g_nPWMDuty );
		}
		
		if (bBeep) {
			bBeep = false;
			SPKSet(1);
			LEDSet(1);
			_delay_us(100);
			SPKSet(0);
			LEDSet(0);
		}

	}
	return (0);
} 
