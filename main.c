#include <avr/io.h>

#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>


#define BAUD 1000000UL// Baudrate UL UL UL UL
#define TIMER_PRESCALE ((1 << CS01) | (1 << CS00))
#define TIMER_PRELOAD 6

#define uart_maxstrlen 50

volatile uint8_t uart_str_complete=0;
volatile uint8_t uart_str_count=0;
volatile char uart_string[uart_maxstrlen+1]="";
volatile char state_b=0;
volatile uint8_t transmit_state=0;
// Berechnungen
#define UBRR_VAL ((F_CPU+BAUD*8)/(BAUD*16)-1)   // clever runden
#define BAUD_REAL (F_CPU/(16*(UBRR_VAL+1)))     // Reale Baudrate
#define BAUD_ERROR ((BAUD_REAL*1000)/BAUD) // Fehler in Promille, 1000 = kein Fehler.
#if ((BAUD_ERROR<990) || (BAUD_ERROR>1010))
 #error Systematischer Fehler der Baudrate grösser 1% und damit zu hoch! 
#endif

uint16_t i = 0;
uint8_t j = 0;
uint8_t state = 0; 
float tempvol = 0;
volatile uint16_t worldclock = 0;
volatile uint16_t last_period = 0;
volatile uint16_t period_dur = 0;


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


/* puts ist unabhaengig vom Controllertyp */
void uart_puts (char *s)
{
    while (*s)
    {   /* so lange *s != '\0' also ungleich dem "String-Endezeichen" */
        uart_putc(*s);
        s++;
    }
}

uint8_t uart_getc(void) {
	while (!(UCSRA & (1 << RXC) )) {
	}
	return UDR;
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

ISR(USART_RXC_vect) {
        uart_gets();
        if ( uart_str_complete == 1) {
                uart_puts("bekommen: ");
                uart_puts(uart_string);
		if (0 == strcmp(uart_string, "on")) {
               		uart_puts("on!\r\n");
			state = 1;
		}
		if (0 == strcmp(uart_string, "off")) {
			state = 0;
               		uart_puts("off!\r\n");
		}
               uart_puts("\r\n");
                uart_str_complete = 0;
        }
}

ISR (TIMER0_OVF_vect) { //overflows every 0.001s == 1ms
	TCNT0 = TIMER_PRELOAD;
	worldclock++;
}

ISR (INT0_vect) {
	// 
	//period_dur = worldclock - last_period;
	//last_period  = worldclock;
	if (state == 1) {
	_delay_us(0);
	PORTC |= (1 << PC1);
	_delay_us(20);
	PORTC &= ~(1 << PC1);
	}
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
//	fallende flanke
	MCUCR = (1 << ISC11);


	worldclock = 0;
	TCCR0 |= TIMER_PRESCALE; // Timer0, Clock/64
	TCNT0 = TIMER_PRELOAD;
	TIMSK |= (1<<TOIE0); //Interrupt auf Overflow

	sei();
/* END INIT SECTION */
	uart_puts("Hallo Welt!\r\n");
	char buf[60];
	uint16_t lastprint = 0;
	while(1) {	
		_delay_us(1);
		if (worldclock >= (lastprint + 1000) ) { // each 1s
			uint16_t temp;
			temp = 	read_adc();
			float tmp2 = convert_adc_to_celsius(temp);

			sprintf(buf, "clk: %u period: %ums raw: %u conv: %.3f state: %u\r\n", worldclock,period_dur, temp, tmp2, state);
			uart_puts(buf);
			if (tmp2 > 300) {
				state = 0;
				uart_puts("shutdown!\r\n");
			}
			lastprint = worldclock;
		}

	}

	return 0;
}
