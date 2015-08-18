/*
 * TTests.cpp
 *
 *  Created on: 26.06.2014
 *      Author: Shcheblykin
 */
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "TTests.h"
#include "TM29f8000b.h"


/**	Структура FSM для тестов.
 *
 *
 */
const TTests::SStateFSM TTests::FSM[TEST_MAX] = { 					//
		{ &TTests::testError, 	{TEST_SOUT_BUS, TEST_SOUT_BUS} },	//
		{ &TTests::testSoutBus, {TEST_PLIS_REG, TEST_SOUT_BUS} },	//
		{ &TTests::testRegPlis, {TEST_DATA_BUS,	TEST_PLIS_REG} }, 	//
		{ &TTests::testDataBus, {TEST_FRAM,	    TEST_DATA_BUS} }, 	// нет у ГВ
		{ &TTests::testFram, 	{TEST_2RAM, 	TEST_FRAM	 } }, 	//
		{ &TTests::test2Ram,	{TEST_EXT_BUS,	TEST_2RAM 	 } },	//
		{ &TTests::testExtBus,  {TEST_EXT_BUS,  TEST_EXT_BUS } } 	//
};


const uint8_t TTests::crc8[256] PROGMEM = {
    0x00, 0x5E, 0xBC, 0xE2, 0x61, 0x3F, 0xDD, 0x83,
    0xC2, 0x9C, 0x7E, 0x20, 0xA3, 0xFD, 0x1F, 0x41,
    0x9D, 0xC3, 0x21, 0x7F, 0xFC, 0xA2, 0x40, 0x1E,
    0x5F, 0x01, 0xE3, 0xBD, 0x3E, 0x60, 0x82, 0xDC,
    0x23, 0x7D, 0x9F, 0xC1, 0x42, 0x1C, 0xFE, 0xA0,
    0xE1, 0xBF, 0x5D, 0x03, 0x80, 0xDE, 0x3C, 0x62,
    0xBE, 0xE0, 0x02, 0x5C, 0xDF, 0x81, 0x63, 0x3D,
    0x7C, 0x22, 0xC0, 0x9E, 0x1D, 0x43, 0xA1, 0xFF,
    0x46, 0x18, 0xFA, 0xA4, 0x27, 0x79, 0x9B, 0xC5,
    0x84, 0xDA, 0x38, 0x66, 0xE5, 0xBB, 0x59, 0x07,
    0xDB, 0x85, 0x67, 0x39, 0xBA, 0xE4, 0x06, 0x58,
    0x19, 0x47, 0xA5, 0xFB, 0x78, 0x26, 0xC4, 0x9A,
    0x65, 0x3B, 0xD9, 0x87, 0x04, 0x5A, 0xB8, 0xE6,
    0xA7, 0xF9, 0x1B, 0x45, 0xC6, 0x98, 0x7A, 0x24,
    0xF8, 0xA6, 0x44, 0x1A, 0x99, 0xC7, 0x25, 0x7B,
    0x3A, 0x64, 0x86, 0xD8, 0x5B, 0x05, 0xE7, 0xB9,
    0x8C, 0xD2, 0x30, 0x6E, 0xED, 0xB3, 0x51, 0x0F,
    0x4E, 0x10, 0xF2, 0xAC, 0x2F, 0x71, 0x93, 0xCD,
    0x11, 0x4F, 0xAD, 0xF3, 0x70, 0x2E, 0xCC, 0x92,
    0xD3, 0x8D, 0x6F, 0x31, 0xB2, 0xEC, 0x0E, 0x50,
    0xAF, 0xF1, 0x13, 0x4D, 0xCE, 0x90, 0x72, 0x2C,
    0x6D, 0x33, 0xD1, 0x8F, 0x0C, 0x52, 0xB0, 0xEE,
    0x32, 0x6C, 0x8E, 0xD0, 0x53, 0x0D, 0xEF, 0xB1,
    0xF0, 0xAE, 0x4C, 0x12, 0x91, 0xCF, 0x2D, 0x73,
    0xCA, 0x94, 0x76, 0x28, 0xAB, 0xF5, 0x17, 0x49,
    0x08, 0x56, 0xB4, 0xEA, 0x69, 0x37, 0xD5, 0x8B,
    0x57, 0x09, 0xEB, 0xB5, 0x36, 0x68, 0x8A, 0xD4,
    0x95, 0xCB, 0x29, 0x77, 0xF4, 0xAA, 0x48, 0x16,
    0xE9, 0xB7, 0x55, 0x0B, 0x88, 0xD6, 0x34, 0x6A,
    0x2B, 0x75, 0x97, 0xC9, 0x4A, 0x14, 0xF6, 0xA8,
    0x74, 0x2A, 0xC8, 0x96, 0x15, 0x4B, 0xA9, 0xF7,
    0xB6, 0xE8, 0x0A, 0x54, 0xD7, 0x89, 0x6B, 0x35
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

	while (1) {
		rstExtWdt();

		if (flag) {
			if (step == 0) break;

			flag = false;
			SOut.setValue(1 << ((step-1) % 8));
			step--;
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

	while(1) {
		rstExtWdt();

		if (flag) {
			if (step == 0) break;

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
 *	@retval 3-бит Последовательная запись/чтение.
 */
uint8_t TTests::testRegPlis(uint8_t value=0) {
	uint8_t step = 4;
	uint8_t error = 0;
	volatile uint8_t tmp = 0;

	SOut.setValue(TEST_PLIS_REG);

	while (1) {
		rstExtWdt();

		if (flag) {
			if (step == 0) break;

			flag = false;
			step--;
			SOut.tglMask(TEST_PLIS_REG);

			// проверка версии прошивки Vers
			for (uint_fast8_t i = 0; i < 8; i++) {
				tmp = plis->vers;
				if (tmp != 255)
					error |= (1 << 0);
			}

			// проверка регистра ExtSet
			for (uint8_t i = 1; i > 0; i <<= 1) {
				plis->extSet = i;
				tmp = plis->extSet;
				if (tmp != i)
					error |= (1 << 1);
			}

			// проверка регистра Init
			for (uint8_t i = 1; i > 0; i <<= 1) {
				plis->init = static_cast<REG_INIT> (i);
				tmp = plis->init;
				if (tmp != i)
					error |= (1 << 2);
			}

			// проверка корректности адресов
			plis->init = static_cast<REG_INIT> (11);
			plis->curAdr = 22;
			plis->extSet = 44;
			plis->busW = 88;
			plis->bankFl = 255;

			if (plis->init != 11)
				error |= (1 << 3);

			if (plis->curAdr != 22)
				error |= (1 << 3);
			plis->curAdr = 0;

			if (plis->extSet != 44)
				error |= (1 << 3);
			plis->extSet = 0;

			if (plis->bankFl != 255)
				error |= (1 << 3);
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

	// разрешение работы с внешними устройствами
	plis->extSet = (0 << 3) | (1 << 2);		// BL -> 0

	while (1) {
		rstExtWdt();

		if (flag) {
			if (step == 0) break;

			flag = false;
			step--;
			SOut.tglMask(TEST_DATA_BUS);


//			for (uint8_t i = 0; i < 255; i++) {
//				plis->busW = i;
//
//				// проерка значения записанного в регистр BusW ПЛИС
//				tmp = plis->busW;
//				if (tmp != i)
//					error |= (1 << 0);
//				_delay_us(10);
//
//				// проверка значения на шине BusR
//				tmp = plis->busR;
//				// на проверочной плате BUSW0 -> BUSR3, .., BUSW3 -> BUSR0
//				uint8_t t = (tmp & 0xF0); // старшие разряды остаются как были
//				t += ((tmp & 1) << 3); // BUSW0 -> BUSR3
//				t += ((tmp & 2) << 1); // BUSW1 -> BUSR2
//				t += ((tmp & 4) >> 1); // BUSW2 -> BUSR1
//				t += ((tmp & 8) >> 3); // BUSW3 -> BUSR0
//				if (t != i)
//					error |= (1 << 1);
//				_delay_us(40);
//			}
		}
	}

	// запрет работы с внешними устройствами
	plis->extSet &= ~((0 << 3) | (1 << 2));		// BL -> 1

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

static uint8_t buf[16];
uint8_t TTests::testFram(uint8_t value=1) {

	typedef enum {
		L_ERROR_NO 					= 0x00,	///< Ошибок нет.
		L_ERROR_READ_IMMEDIATELY 	= 0x01,	///< Ошибка чтения байта сразу после записи.
		L_ERROR_READ_LATER 			= 0x02,	///< Ошибка чтения байта после записи всех блоков.
		L_ERROR_WRITE_BYTE			= 0x04,	///< Ошибка записи байта.
		L_ERROR_BLOCK_ERASE			= 0x08	///< Ошибка очистки блока памяти.
	} ELocalError;

	uint8_t val = 0;
	uint8_t step = 0;
	uint8_t error = L_ERROR_NO;

	PORTD |= 0x20;
	SOut.setValue(TEST_FRAM);

	setBlock(0);
	plis->init = REG_INIT_FRAM_ENABLE;

	while(1) {
		flashRead(0);

		if (flag) {
			if (error)
				break;

			flag = false;

			SOut.tglMask(TEST_FRAM);

			if (step < NUM_BLOCKS) {
				// очистка блоков памяти
				if (eraseBlock(step) != ERROR_NO) {
					error |= L_ERROR_BLOCK_ERASE;
				}
			} else if (step < NUM_BLOCKS*2) {
				// запись всех блоков с првоеркой каждого записанного байте
				setBlock(step%NUM_BLOCKS);
				val = step%NUM_BLOCKS;

				for(uint16_t i = 0; i < BLOCK_SIZE; i++) {
					val += crc8[val];
					if (flashProgramByte((uint16_t) i, val)) {
						error |= L_ERROR_WRITE_BYTE;
						break;
					}
					if (flashRead((uint16_t) i) !=  val) {
						error |= L_ERROR_READ_IMMEDIATELY;
						break;
					}
				}
			} else if (step < NUM_BLOCKS*3) {
				// проверка всех записанных блоков
				setBlock(step%NUM_BLOCKS);
				val = step%NUM_BLOCKS;

				for(uint16_t i = 0; i < BLOCK_SIZE; i++) {
					val += crc8[val];
					if (flashRead((uint16_t) i) !=  val) {
						error |= L_ERROR_READ_LATER;
						break;
					}
				}
			} else {
				break;
			}
			step++;
		}

	}

	setBlock(0);
	plis->init = REG_INIT_FRAM_DISABLE;

	if (error) {
		printError(error);
	}

	return (error > 0 ? FSM_NEXT_ERROR : FSM_NEXT_NO_ERROR);
}

/**	Тестирование чтения и записи памяти 2RAM
 *
 *	Производится проверка 1к памяти 2RAM:
 *	- значение записанное в 2RAM сразу же проверяется;
 *	- записывается вся память 2RAM и затем проверяется.
 *	Проверка производится 4 раза.
 *
 * 	@param value Не используется.
 *	@return Код ошибки. Каждый установленный бит отвечает за отдельную ошибку.
 *	@retval 0-бит Значение не совпало при считывании сразу после записи.
 *	@retval 1-бит Значение не совпало при считывании после записи всей 2RAM.
 */
uint8_t TTests::test2Ram(uint8_t value=1) {
	uint8_t val = 0;
	uint8_t step = 5;
	uint8_t error = 0;
	volatile uint8_t * volatile const ptr = (uint8_t*) (RAM_ADR);

	SOut.setValue(TEST_2RAM);

	while(1) {
		rstExtWdt();

		if (flag) {
			if (step == 0) break;

			flag = false;
			step--;
			SOut.tglMask(TEST_2RAM);

			// проверка чтение/запись одного байта данных
			// ^ (i >> 8) - надо для того, чтобы в память не писались
			// повторяющиеся куски кода
			val = step;
			for(uint16_t i = 0; i < RAM_SIZE; i++) {
				uint8_t tmp = val ^ i;
				val = pgm_read_byte(&crc8[tmp]);
				ptr[i] = val;
				if (val != ptr[i]) {
					error |= 1;
				}
				rstExtWdt();
			}

			val = step;
			// проверка чтения всей памяти
			for(uint16_t i = 0; i < RAM_SIZE; i++) {
				uint8_t tmp = val ^ i;
				val = pgm_read_byte(&crc8[tmp]);
				if (val != ptr[i]) {
					error |= 2;
				}
				rstExtWdt();
			}
		}
	}

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

	while(1) {
		rstExtWdt();

		if (flag) {
			if (step == 0) break;

			flag = false;
			step--;
			SOut.tglMask(TEST_EXT_BUS);

			// разрешим работу с внешними устройствами  и установим шину на записи
			plis->extSet = (0 << 3) | (1 << 2);		// BL -> 0
			plis->extSet |= (1 << 4) | (1 << 0);	// Ext_RD -> 1
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

	while(1) {
		rstExtWdt();

		if (flag) {
			if (step == 0) break;

			flag = false;
			step--;
		}
	}

	return 0;
}
