#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

#include "inc\TSoutBus.h"
#include "inc\TTests.h"

#define INITSECTION __attribute__((section(".init3")))
#define NOINIT	    __attribute__((section (".noinit")))
#define NORETURN	__attribute__((__naked__))

// инициализация периферии
INITSECTION NORETURN void low_level_init();

// тесты
TTests tests;

/**	Main
 *
 */
__attribute__ ((OS_main)) int main() {

	sei();
	
	while(1) {
		tests.main();
	}
}

ISR(TIMER1_COMPA_vect) {
	tests.setFlag();
}


void low_level_init() {
   // без предделителя, clk I/O = 16000
    XDIV  = 0x00;

	//Настройка портов
	//все порты настроены на вход и находятся в Z-состоянии

    // Порт РA работает в режиме "шина данных-адреса"
    PORTA = 0xFF;
    DDRA  = 0x00;       // 0-вход, 1-выход

	DDRB=0xF0;    //сигнализация
	PORTB=0xF0;

    // Порт РC работает в режиме "шина адреса"
    PORTC = 0xFF;
    DDRC  = 0x00;

	DDRF = 0x0F;    //сигнализация
	PORTF = 0x0F;

	// Порт РG работает в режиме "шина управления"
	/*  Назначение разрядов порта PG:
	PG.0 : #WR
	PG.1 : #RD
	PG.2 : ALE
	PG.3 : TOSC2
	PG.4 : TOSC1
	*/
    PORTG = 0x1F;
    DDRG  = 0x00;

		//********** Внешняя память **********

    XMCRA = 0x00;         // Один сектор без тактов ожидания
    XMCRB = 0x80;         // 64К c запоминанием состояния шины
    MCUCR |= 0x80;        // Разрешение внешней памяти

	// CTC по OCR1A
	// предделитель 1024
	// счет 15625 цилков
	// получаем 1 секунду
	TCCR1A = (0 << WGM11) | (0 << WGM10);
	TCCR1B = (0 << WGM13) | (1 << WGM12);
	OCR1A = 15625 - 1;
	TIMSK |= (1 << OCIE1A);
	TCCR1B |=  (1 << CS12) | (0 << CS11) | (1 << CS10);
}



