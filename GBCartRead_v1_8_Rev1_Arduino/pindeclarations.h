#ifndef PIN_DECLARATIONS_H_
#define PIN_DECLARATIONS_H_

#define PA0 0
#define PA1 1
#define PA2 2
#define PA3 3
#define PA4 4
#define PA5 5
#define PA6 6
#define PA7 7

#define PB0 0
#define PB1 1
#define PB2 2
#define PB3 3
#define PB4 4
#define PB5 5
#define PB6 6
#define PB7 7

#define PC0 0
#define PC1 1
#define PC2 2
#define PC3 3
#define PC4 4
#define PC5 5
#define PC6 6
#define PC7 7

#define PD0 0
#define PD1 1
#define PD2 2
#define PD3 3
#define PD4 4
#define PD5 5
#define PD6 6
#define PD7 7

#define wrPin_high    PORTC |= (1<<PC3);
#define wrPin_low     PORTC &= ~(1<<PC3);
#define mreqPin_high  PORTC |= (1<<PC4);
#define mreqPin_low   PORTC &= ~(1<<PC4);
#define rdPin_high    PORTC |= (1<<PC5);
#define rdPin_low     PORTC &= ~(1<<PC5);
#define latchPin_high PORTB |= (1<<PB2);
#define latchPin_low  PORTB &= ~(1<<PB2);
#define dataPin_high  PORTB |= (1<<PB3);
#define dataPin_low   PORTB &= ~(1<<PB3);
#define clockPin_high PORTB |= (1<<PB5);
#define clockPin_low  PORTB &= ~(1<<PB5);

#endif /* PIN_DECLARATIONS_H_ */
