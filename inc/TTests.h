/*
 * TTests.h
 *
 *  Created on: 26.06.2014
 *      Author: Shcheblykin
 */

#ifndef TTESTS_H_
#define TTESTS_H_

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include "TSoutBus.h"

/**	\brief Класс тестов блока БСП.
 *
 *	Тело класса организовано при помощи автомата конечных состояний (FSM).
 *	В каждом из состоянии тест находится до тех пор, пока не появится
 *	необходимость перейти к следующемй тесту (т.е. бесконечный цикл).
 *
 *	Тестирование начинается с шины SOut. Т.к. на нее идет выход сигналов МК
 *	напрямую.
 *
 *	При обнаружении ошибок в тесте, код ошибки на несколько секунд будет
 *	выведен на шину SOut. После этого тест начнется сначала.
 *
 *	Флаг цикла \a flag используется для определения временных интервалов.
 *	Например, при мигании светодиодами. Времени отводимом на один цикл и т.д.
 *	*/
class TTests {

public:
	/**	Конструктор.
	 *
	 */
	TTests() {
		plis = (SPlisRegister*) (PLIS_ADR);
		ram = (S2RamRegister*) (RAM_ADR);

		curTest = TEST_SOUT_BUS;
		error = 0;
		flag = false;
	}

	/**	Установка флага цикла.
	 *
	 * 	Флаг сбрасывается после обработки в теле класса.
	 *
	 */
	void setFlag() {
		flag = true;
	}

	/**	Тело класса.
	 *
	 */
	void main();

	// Класс работы с шиной SOut
		TSoutBus SOut;

private:

	/// Указатель на функцию теста класса TTests
	typedef uint8_t (TTests::*pTest) (uint8_t time);

	/// Массив значений для вычисления CRC-8.
	static const uint8_t crc8[256];

	/// Номера тестов блока БСП
	enum TESTS {
		TEST_ERROR 		= 0,	///< Вывод сообщения ошибки теста.
		TEST_SOUT_BUS	= 1,	///< Проверка шины SOut.
		TEST_PLIS_REG	= 2,	///< Проверка регистров ПЛИС.
		TEST_DATA_BUS	= 3,	///< Проверка шин данных BusW/BusR.
		TEST_FRAM		= 4,	///< Проверка чтения/записи FRAM.
		TEST_2RAM		= 5,	///< Проверка чтения/записи 2RAM.
		TEST_EXT_BUS	= 6,	///< Проверка внешней шины данных/адреса.
		TEST_MAX				///< Максимальное кол-во тестов.
	};

	/// Возможные переходы в FSM
	enum FSM_NEXT {
		FSM_NEXT_NO_ERROR 	= 0,///< Тест закончился без ошибок.
		FSM_NEXT_ERROR		= 1,///< Тест закончился с ошибой.
		FSM_NEXT_MAX			///< Максимальное кол-во переходов.
	};

	/// Используемые значения для регистра init ПЛИС
	enum REG_INIT {
		REG_INIT_FRAM_ENABLE  = 0x55,	///< Разрешение работы с FRAM
		REG_INIT_FRAM_DISABLE = 0x00 	///< Запрет работы с FRAM
	};

	/// структура регистров расположенных в ПЛИС
	struct SPlisRegister{
		REG_INIT init;			///<
		const uint8_t vers;		///< версия прошивки ПЛИС
		uint16_t dd;			///<
		uint8_t curAdr;			///<
		uint8_t extSet;			///<
		const uint8_t busR;		///< регистр чтения шины BusR
		uint8_t busW;			///< регистр записи шины BusW
		uint8_t bankFl;			///<
		uint8_t null;			///<
	};

	/// структура параметров расположенных в 2RAM
	struct S2RamRegister{
		uint8_t line;			///<
	};

	/// структура FSM тестов
	struct SStateFSM{
		pTest test;					///< текущий тест
		TESTS next[FSM_NEXT_MAX];	///< таблица переходов состояний
	};

	// АДРЕСА РЕГИСТРОВ И ПЕРЕМЕННЫХ ВО ВНЕШНЕЙ ПАМЯТИ
	static const uint16_t RAM_ADR	 =	0x3000;		///< Начальный адрес 2RAM.
	static const uint16_t RAM_SIZE	 =	0x0800;		///< Размер памяти 2RAM.
	static const uint16_t PLIS_ADR   =	0x7000;		///< Начальный адрес ПЛИС.
	static const uint16_t FLASH_ADR  =	0x8000;		///< Начальный адрес FLASH.
	static const uint16_t FLASH_SIZE =	0x8000;		///< Размер памяти FLASH.

	// СТРУКТУРЫ РЕГИСТРОВ И ПЕРЕМЕННЫХ ВО ВНЕШНЕЙ ПАМЯТИ
	volatile SPlisRegister *plis;					///< Регистры ПЛИС.
	volatile S2RamRegister *ram;					///< Параметры 2RAM.

	static const SStateFSM FSM[TEST_MAX];			///< FSM.

	volatile bool flag;								///< Флаг цикла.
	TESTS curTest;									///< Текущий тест.
	uint8_t error;									///< Ошибки теста.


	// ТЕСТЫ
	uint8_t testSoutBus(uint8_t value);				// Тест шины SOut.
	uint8_t testRegPlis(uint8_t value);				// Тест ПЛИС.
	uint8_t testDataBus(uint8_t value);				// Тест шин BusR, BusW.
	uint8_t testFram(uint8_t value);				// Тест чтения\записи FRAM.
	uint8_t test2Ram(uint8_t value);				// Тест чтения\записи 2RAM.
	uint8_t testExtBus(uint8_t value);				// Тест внешней шины.
	uint8_t testError(uint8_t value);				// Вывод сообщения ошибки.


	// Вывод кода ошибки на шину SOut.
	uint8_t printError(uint8_t value);

	/**	Сброс внешнего сторожевого таймера, записью в 2RAM.
	 *
	 */
	void rstExtWdt() {
		volatile uint8_t tmp = ram->line;
	}
};


#endif /* TTESTS_H_ */
