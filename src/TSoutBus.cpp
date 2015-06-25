/*
 * TSoutBus.cpp
 *
 *  Created on: 25.06.2015
 *      Author: Shcheblykin
 */

#include "TSoutBus.h"

const SPort TSoutBus::s_stPorts[s_u8NumPorts] = {
		{&PORTB, PB7, false},	// SOUT0 - CF
		{&PORTB, PB6, false},	// SOUT1 - Com PRM
		{&PORTB, PB5, false},	// SOUT2 - Com PRD
		{&PORTB, PB4, false},	// SOUT3 - Alarm DEF
		{&PORTE, PE5, false},	// SOUT4 - Alarm PRM
		{&PORTE, PE4, false},	// SOUT5 - Alarm PRD
		{&PORTE, PE3, false},	// SOUT6 - Warning
		{&PORTD, PD0, false}	// SOUT7 - Alam
};

// Конструктор.
TSoutBus::TSoutBus() {
	// адрес(PORTX - 1) = адрес(DDRx)
	for(uint8_t i = 0; i < 8; i++) {
		*(s_stPorts[i].port - 1) |= (1 << s_stPorts[i].pin);	// DDRx = 1
		*s_stPorts[i].port |= (1 << s_stPorts[i].pin);			// PORTx = 0
	}
}

// Установка сигналов по маске.
void TSoutBus::setMask(uint8_t mask) {
	for(uint8_t i = 0; i < 8; i++) {
		if (mask & (1 << i)) {
			ledOn(i);
		}
	}
}

// Сброс сигналов по маске.
void TSoutBus::clrMask(uint8_t mask) {
	for(uint8_t i = 0; i < 8; i++) {
		if (mask & (1 << i)) {
			ledOff(i);
		}
	}
}

// Переключение сигналов по маске.
void TSoutBus::tglMask(uint8_t mask) {
	for(uint8_t i = 0; i < 8; i++) {
		if (mask & (1 << i)) {
			ledSwitch(i);
		}
	}
}

// Установка сигналов.
void TSoutBus::setValue(uint8_t val) {
	for(uint8_t i = 0; i < 8; i++) {
		// установка значения на выводе с проверкой на инверсию
		if (val & (1 << i)) {
			ledOn(i);
		} else {
			ledOff(i);
		}
	}
}

// Включение светодиода.
void TSoutBus::ledOn(uint8_t num) {
	if (s_stPorts[num].inv) {
		*s_stPorts[num].port &= ~((1 << s_stPorts[num].pin));
	} else {
		*s_stPorts[num].port |= (1 << s_stPorts[num].pin);
	}
}

// Выключение светодиода.
void TSoutBus::ledOff(uint8_t num) {
	if (s_stPorts[num].inv) {
		*s_stPorts[num].port |= (1 << s_stPorts[num].pin);
	} else {
		*s_stPorts[num].port &= ~((1 << s_stPorts[num].pin));
	}
}

// Переключение состояние светодиода.
void TSoutBus::ledSwitch(uint8_t num) {
	uint8_t val = *s_stPorts[num].port;
	*s_stPorts[num].port = val ^ (1 << s_stPorts[num].pin);
}

