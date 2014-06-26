/*
 * TTests.cpp
 *
 *  Created on: 26.06.2014
 *      Author: Shcheblykin
 */
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "../inc/TTests.h"

/**	Структура FSM для тестов.
 *
 *
 */
const TTests::SStateFSM TTests::FSM[TEST_MAX] = { 					//
		{ &TTests::testError, 	{TEST_SOUT_BUS, TEST_SOUT_BUS} },	//
		{ &TTests::testSoutBus, {TEST_PLIS_REG, TEST_SOUT_BUS} },	//
		{ &TTests::testRegPlis, {TEST_DATA_BUS, TEST_PLIS_REG} }, 	//
		{ &TTests::testDataBus, {TEST_FRAM,  	TEST_DATA_BUS} }, 	//
		{ &TTests::testFram, 	{TEST_EXT_BUS, 	TEST_FRAM    } }, 	//
		{ &TTests::testExtBus,  {TEST_EXT_BUS,  TEST_EXT_BUS } } 	//
};

// Тело класса
void TTests::main() {
		uint8_t next = 0;

		next = (this->*FSM[curTest].test)(next % FSM_NEXT_MAX);
		curTest = FSM[curTest].next[next];
}


/**	Тест шины SOut
 *
 *	Визуальная проверка шины внешних сигналов (авария, предупреждение и т.д).
 * 	Поочередно устаналиваются сигналы на каждом из выходов.
 *	Повторяется дважды.
 *
 *	@param value Не используется.
 *	@return Всегда 0.
 */
uint8_t TTests::testSoutBus(uint8_t value) {
	uint8_t step = 16;

	while (step) {
		rstExtWdt();

		if (flag) {
			flag = false;
			step--;
			SOut.setValue(1 << (step % 8));
		}
	}

	return FSM_NEXT_NO_ERROR;
}

/**	Вывод сообщения об ошибке в тесте.
 *
 *	В течении нескольких секунд на шину SOut выводится код ошибки.
 *
 *	@param value Код ошибки.
 *	@return Всегда 0.
 */
uint8_t TTests::testError(uint8_t value=0) {
	uint8_t step = 5;

	SOut.setValue(value);

	while(step) {
		rstExtWdt();

		if (flag) {
			flag = false;
			step--;
		}
	}

	return FSM_NEXT_NO_ERROR;
}

/** Тестирование ПЛИС.
 *
 *	Производится проверка регистров ПЛИС доступных для записи и(или) чтения:
 *	- проверка версии прошивки Vers(чтение);
 *	- проверка регистра ExtSet (запись/чтение);
 *	- проверка регистра Init (запись/чтение);
 *	- проверка корректности адресов, в регистры доступные на чтение и запись
 * 	последовательно записываются чилсла, а после этого так же последовательно
 *	происходит проверка их содержимого.
 *	Повторяется 4 раза.
 *
 * 	@param value Не используется.
 *	@return Код ошибки. Каждый установленный бит отвечает за отдельную ошибку.
 *	@retval 0-бит Версия прошивки Vers.
 *	@retval 1-бит Регистр ExtSet.
 *	@retval 2-бит Регистр Init.
 *	@retval 4-бит Последовательная запись/чтение.
 */
uint8_t TTests::testRegPlis(uint8_t value=0) {
	uint8_t step = 4;
	uint8_t error = 0;
	volatile uint8_t tmp = 0;

	SOut.setValue(TEST_PLIS_REG);

	while (step) {
		rstExtWdt();

		if (flag) {
			flag = false;
			step--;
			SOut.tglMask(TEST_PLIS_REG);

			// проверка версии прошивки Vers
			for (uint_fast8_t i = 0; i < 8; i++) {
				tmp = plis->vers;
				if (tmp != 255)
					error |= 1;
			}

			// проверка регистра ExtSet
			for (uint8_t i = 1; i > 0; i <<= 1) {
				plis->extSet = i;
				tmp = plis->extSet;
				if (tmp != i)
					error |= 2;
			}

			// проверка регистра Init
			for (uint8_t i = 1; i > 0; i <<= 1) {
				plis->init = static_cast<REG_INIT> (i);
				tmp = plis->init;
				if (tmp != i)
					error |= 8;
			}

			// проверка корректности адресов
			plis->init = static_cast<REG_INIT> (11);
			plis->curAdr = 22;
			plis->extSet = 44;
			plis->busW = 88;
			plis->bankFl = 255;

			if (plis->init != 11)
				error |= 16;

			if (plis->curAdr != 22)
				error |= 16;
			plis->curAdr = 0;

			if (plis->extSet != 44)
				error |= 16;
			plis->extSet = 0;

			if (plis->bankFl != 255)
				error |= 16;
			plis->bankFl = 0;
		}
	}

	if (error) {
		printError(error);
	}

	return (error > 0 ? FSM_NEXT_ERROR : FSM_NEXT_NO_ERROR);
}

/**	Тестирование шин BUSW и BUSR.
 *
 *	На шину BUSW выставляется значение и сравнивается со считанным с BUSR.
 *
 *	@param value Не используется.
 *	@return Код ошибки. Каждый установленный бит отвечает за отдельную ошибку.
 *	@retval 0-бит Ошибка записи/чтения регистра BusW.
 *	@retval 1-бит Значение регистра BusR не совпало с установленным в BusW.
 */
uint8_t TTests::testDataBus(uint8_t value=0) {
	uint8_t step = 4;
	uint8_t error = 0;
	volatile uint8_t tmp = 0;

	SOut.setValue(TEST_DATA_BUS);

	while (step) {
		rstExtWdt();

		// разрешим работу с внешними устройствами
		plis->extSet = (0 << 3) | (1 << 2);		// BL -> 0

		if (flag) {
			flag = false;
			step--;
			SOut.tglMask(TEST_DATA_BUS);


			for (uint8_t i = 0; i < 255; i++) {
				plis->busW = i;

				// проерка значения записанного в регистр BusW ПЛИС
				tmp = plis->busW;
				if (tmp != i)
					error |= 1;
				_delay_us(10);

				// проверка значения на шине BusR
				tmp = plis->busR;
				// на проверочной плате BUSW0 -> BUSR3, .., BUSW3 -> BUSR0
				uint8_t t = (tmp & 0xF0); // старшие разряды остаются как были
				t += ((tmp & 1) << 3); // BUSW0 -> BUSR3
				t += ((tmp & 2) << 1); // BUSW1 -> BUSR2
				t += ((tmp & 4) >> 1); // BUSW2 -> BUSR1
				t += ((tmp & 8) >> 3); // BUSW3 -> BUSR0
				if (t != i)
					error |= 2;
				_delay_us(40);
			}
		}

		// запрет работы с внешними устройствами
		plis->extSet &= ~((0 << 3) | (1 << 2));		// BL -> 1
	}

	if (error) {
		printError(error);
	}

	return (error > 0 ? FSM_NEXT_ERROR : FSM_NEXT_NO_ERROR);
}

/**	Тестирование чтения и записи памяти FRAM.
 *
 *	Производится проверка всей памяти FRAM:
 *	- значение записанное в FRAM сразу же проверяется;
 *	- записывается вся память FRAM и затем проверяется.
 *	Проверка производится 4 раза.
 *
 * 	@param value Не используется.
 *	@return Код ошибки. Каждый установленный бит отвечает за отдельную ошибку.
 *	@retval 0-бит Значение не совпало при считывании сразу после записи.
 *	@retval 1-бит Значение не совпало при считывании после записи всей FRAM.
 */
uint8_t TTests::testFram(uint8_t value=0) {
	uint8_t step = 5;
	uint8_t error = 0;
	volatile uint8_t* ptr = (uint8_t*) (FLASH_ADR);

	SOut.setValue(TEST_FRAM);
	plis->init = REG_INIT_FRAM_ENABLE;

	while(step) {
		rstExtWdt();

		if (flag) {
			flag = false;
			step--;
			SOut.tglMask(TEST_FRAM);


			// проверка чтение/запись одного байта данных
			ptr = (uint8_t*) (FLASH_ADR);
			for(uint16_t i = 0; i < FLASH_SIZE; i++) {
				uint8_t tmp = static_cast<uint8_t>  (i + value);
				*ptr = tmp;
				if (*ptr != tmp) {
					error |= 1;
				}
				ptr++;
				rstExtWdt();
			}

			// проверка чтения всей памяти
			ptr = (uint8_t*) (FLASH_ADR);
			for(uint16_t i = 0; i < FLASH_SIZE; i++) {
				uint8_t tmp = static_cast<uint8_t>  (i + value);
				if (*ptr != tmp) {
					error |= 2;
				}
				ptr++;
				rstExtWdt();
			}
		}
	}

	plis->init = REG_INIT_FRAM_DISABLE;

	if (error) {
		printError(error);
	}

	return (error > 0 ? FSM_NEXT_ERROR : FSM_NEXT_NO_ERROR);
}

/**	Тестирование внешней шины связи.
 *
 *	На шину поочередно устанавливаются разряды (D0-D15) и (А0-А3, CS0-CS3).
 *
 *	Проверка - визуально. Считывание не проверяется.
 *
 *	@param value Не используется.
 *	@return Всегда 0.
 */
uint8_t TTests::testExtBus(uint8_t value=0) {
	uint8_t step = 16;

	SOut.setValue(TEST_EXT_BUS);

	while(step) {
		rstExtWdt();

		if (flag) {
			flag = false;
			step--;
			SOut.tglMask(TEST_EXT_BUS);

			// разрешим работу с внешними устройствами  и установим шину на записи
			plis->extSet = (0 << 3) | (1 << 2);		// BL -> 0
			plis->extSet |= (1 << 4) | (1 << 0);	// Ext_RD -> 0
			if (step & 0x01) {
				plis->extSet |= (1 << 1);
			}
			plis->curAdr = 1 << (step % 8);
			plis->dd = 1 << (step % 16);
		}
	}

	return FSM_NEXT_NO_ERROR;
}

/**	Вывод кода ошибки на шинц SOut.
 *
 *	Код ошибки выводится на несколько секунд.
 *
 *	Во время вывода ошибки, программа крутится в этой функции.
 *
 *	@param value Код ошибки.
 *	@return Всегда 0.
 */
uint8_t TTests::printError(uint8_t value) {
	uint8_t step = 5;

	SOut.setValue(value);

	while(step) {
		rstExtWdt();

		if (flag) {
			flag = false;
			step--;
		}
	}

	return 0;
}
