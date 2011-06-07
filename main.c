#include <avr/io.h>

#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>


#define BAUD 1000000UL// Baudrate UL UL UL UL

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

double convert_adc_to_celsius(uint16_t wert) {
	double volt = wert * 2.461 / (1024.0);
	return volt;
}

uint16_t read_adc(void) {
	uint8_t i = 0;
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

ISR (TIMER0_OVF_vect) {
	cli();
	i++;
	if (i % 30 == 0) {
		uint16_t temp;
		temp = 	read_adc();
		double tmp2 = convert_adc_to_celsius(temp);
		char buf[60];
		sprintf(buf, " %.3f\r\n", tmp2);
		uart_puts(buf);
		sprintf(buf, "%u\r\n", temp);
		uart_puts(buf);
		if (tmp2 > 2.1) {
			state = 0;
			uart_puts("shutdown!\r\n");
		}
	}
	sei();
}

ISR (INT0_vect) {
	if (state == 1) {
	_delay_us(0);
	PORTC |= (1 << PC1);
	_delay_us(10);
	PORTC &= ~(1 << PC1);
	}
}


int main(void) {

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
//	ADMUX &= ~(1 << REFS0);
//	ADMUX &= ~(1 << REFS1);
//	ADMUX &= ~(1 << ADLAR);
	ADMUX = 0x00;
	ADCSRA &= ~(1 << ADFR);
	ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

	TCCR0 |= (1 << CS02) | (1 << CS00); // Timer0, Clock/1024
	TIMSK |= (1<<TOIE0); //Interrupt auf Overflow
//	external interrupt 0 enable
	GICR = (1 << INT0);
//	steigende und fallende flanke
	MCUCR = (1 << ISC00);
	sei();

	uart_puts("Hallo Welt!\r\n");
	while(1) {
		_delay_ms(100);
	}

	return 0;
}
