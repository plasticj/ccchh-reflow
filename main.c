#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>

#define BAUD 1000000UL// Baudrate UL UL UL UL

#define TIMER_TICKS_PER_HP 624
#define uart_maxstrlen 30

volatile uint8_t uart_str_complete=0;
volatile uint8_t uart_str_count=0;
volatile char uart_string[uart_maxstrlen+1]="";
// Berechnungen
#define UBRR_VAL ((F_CPU+BAUD*8)/(BAUD*16)-1)   // clever runden
#define BAUD_REAL (F_CPU/(16*(UBRR_VAL+1)))     // Reale Baudrate
#define BAUD_ERROR ((BAUD_REAL*1000)/BAUD) // Fehler in Promille, 1000 = kein Fehler.
#if ((BAUD_ERROR<990) || (BAUD_ERROR>1010))
 #error Systematischer Fehler der Baudrate grösser 1% und damit zu hoch! 
#endif


uint8_t state = 0; 

// PT1000 values for 0,100,200,300,400 deg C
const float tempcal[5] = {1000, 1385.055, 1758.56, 2120.515, 2470.92};

void uart_init(unsigned int ubrr) {
	UCSRB |= (1 << RXEN) | (1 << RXCIE) | (1 << TXEN); // reciever/transmitter ein, reciever interrupt ein
        UCSRC |= (1 << URSEL) | ( 1 << UCSZ1) | (3 << UCSZ0); // Asy 8 N 1

        UBRRH = (unsigned char) (ubrr >> 8);
        UBRRL = (unsigned char) ubrr;
}

/* ATmega8 */
inline void uart_putc(unsigned char c)
{
    while (!(UCSRA & (1<<UDRE)))  /* warten bis Senden moeglich */
    {
    }

    UDR = c;                      /* sende Zeichen */
}

inline int strcmp (const char * s1, const char * s2) {
	for(; *s1 == *s2; ++s1, ++s2) {
		if(*s1 == 0)
			return 0;
	}
	return *(unsigned char *)s1 < *(unsigned char *)s2 ? -1 : 1;
}

/* puts ist unabhaengig vom Controllertyp */
inline void uart_puts (char *s)
{
    while (*s)
    {   /* so lange *s != '\0' also ungleich dem "String-Endezeichen" */
        uart_putc(*s);
        s++;
    }
}

inline uint8_t uart_getc(void) {
	while (!(UCSRA & (1 << RXC) )) {
	}
	return UDR;
}



static inline void uart_puti(uint16_t data) { /* send integer per UART */
	uint8_t buffer[7];
	uart_puts(utoa(data, buffer, 10));
}

void uart_gets() {
        unsigned char buffer;

        // Daten aus dem Puffer lesen
        buffer = UDR;
        if (uart_str_complete == 0) {    // wenn uart_string gerade in Verwendung, neues Zeichen verwerfen
                // Daten werden erst in string geschrieben, wenn nicht String-Ende/max Zeichenlänge erreicht ist/string gerade verarbeitet wird
                if (buffer!='\n' && buffer!='\r' && uart_str_count<uart_maxstrlen-1){
                        uart_string[uart_str_count]=buffer;
                        uart_str_count++;
                } else {
                        uart_string[uart_str_count]='\0';
                        uart_str_count=0;
                        uart_str_complete=1;
                }
        }
}

float convert_adc_to_celsius(uint16_t wert) {
	float R = 1000 * (wert * 2.5 / (1024.0));
	uint8_t i = 0;

	// t < 0 deg C
	if (R < tempcal[0]) { 
		return -1;
	}
	
	for (i = 0; i< 5; i++) {
		// i*100 < t <= (i+1)*100 deg C
		if ( (R > tempcal[i]) && (R <= tempcal[i+1]) ) {
			return i*100 + (100-0)/(tempcal[i+1]-tempcal[i])*(R-tempcal[i]);
		}
	}

	// t > 400 deg C
	if (R > tempcal[4]) {
		return 0 + (100-0)/(tempcal[4]-tempcal[3])*(R-tempcal[4]);
	}

	return 1234;
}

uint16_t read_adc(void) {
	//uint8_t i = 0;
	ADCSRA |= ( 1 << ADSC);
	while ( ADCSRA & (1 << ADSC) );
	uint16_t x;
	x = ADCL;       // mit uint16_t x
	x += (ADCH<<8); // in zwei Zeilen (LSB/MSB-Reihenfolge und
        	        // C-Operatorpriorität sichergestellt) 
	return x;
}

// uint16_t test_period(void) {
// 	// ext interrupt aus
// //	GICR &= ~(1 << INT0);
// 
// 	uint16_t test[4];
// 	uint8_t i = 0;
// 	for (; i < 4; i++) {
// 		// warten bis PD2 high geht
// 		while(! (PINB & (1 << PB2)));
// 		// warten bis PD2 low geht
// 		while(PINB & (1 << PB2));
// 		//	jetzt ZC
// 		//	16btimer1 = 0, starten
// 		TCNT1 = 0;
// 		TCCR1B |= (1 << CS12); //clk/256
// 		//	warten bis ZC-signal wieder high
// 		while(! (PINB & (1 << PB2)));
// 		//	warten bis ZC-signal wieder low
// 		while(PINB & (1 << PB2));
// 
// 		//	timer ablesen
// 		test[i] = TCNT1;
// 	}
// 	// ext interrupt ein
// //	GICR |= (1 << INT0);
// 
// 	return (test[3]+test[2]+test[1]+test[0])/8; //return half-period
// }

ISR(USART_RXC_vect) {
        uart_gets();
        if ( uart_str_complete == 1) {
                uart_puts("got: ");
                uart_puts(uart_string);
		if (uart_string[0] == 'l') {
			OCR1B = atoi(&uart_string[1]);
		}

		if (0 == strcmp(uart_string, "on")) {
               		uart_puts("on!\r\n");
			state = 1;
		}
		if (0 == strcmp(uart_string, "off")) {
			state = 0;
               		uart_puts("off!\r\n");
		}
//		if (0 == strcmp(uart_string, "t")) {
//			uint16_t dur = test_period();
//			uart_puts("p: ");
//			uart_puti(dur);
//			uart_puts("samp.\r\n");
//		}

		uart_puts("\r\n");
		uart_str_complete = 0;
        }
}

ISR(TIMER1_COMPB_vect) {
	_delay_us(0);
	PORTC |= (1 << PC1);
	_delay_us(5);
	PORTC &= ~(1 << PC1);
}

ISR(INT0_vect) {
	// setze timer zurück
	TCNT1 = 0;
	// OCR1B wert erreicht -> compb_vect
	// OCR1A -> dauer der periode
}

int main(void) {
/* INIT SECTION */
	cli();
	uart_init(UBRR_VAL);

	// eingang AD
	DDRC &= ~(1 << PC0);
	PORTC &= ~(1<< PC0);

	// eingang ZC 
	DDRD &= ~(1 << PD2);
	PORTD &= ~(1 << PD2);

	// ausgang dimmer
	DDRC |= (1 << PC1);

	// avcc
	ADMUX = 0x00;
	ADCSRA &= ~(1 << ADFR);
	ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

//	external interrupt 0 enable
	GICR = (1 << INT0);
//	steigende und fallende flanke
	MCUCR = (1 << ISC10);

	// 16bit timer1 : CTC mode
	// TCNT1 == OCR1A or TCNT1 == ICR1
	// OCF1A or ICF1 flag -> INT oVF
	TCCR1B |= (1<<WGM12) | ( 1 << CS12); // CTC mode; clk = clkio/256
	TCCR1B &= ~( (1 <<WGM13));
	TCCR1A &= ~( (1 << WGM11) | (1 << WGM10) ); 
	// ttop: OCR1A: ocr1x immedia, TOV1 set on MAX
	OCR1A = TIMER_TICKS_PER_HP;

	OCR1B = 255;
	TIMSK |= (1 << OCIE1B);

	sei();
/* END INIT SECTION */

	uart_puts("Init!\r\n");
	char buf[30];

	while(1) {	
		_delay_ms(2000);
		float tmp2 = convert_adc_to_celsius(read_adc() );
		dtostrf(tmp2, 6, 1, buf);
		uart_puts(buf);
		uart_puts(" ");
		uart_puti(OCR1B);
		uart_puts("\r\n");

		if (tmp2 > 300) {
			OCR1B = TIMER_TICKS_PER_HP+100;
			uart_puts("off!\r\n");
		}


	}

	return 0;
}
