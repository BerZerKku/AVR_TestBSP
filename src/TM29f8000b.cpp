/*
 * TM29f8000b.cpp
 *
 *  Created on: 12.08.2015
 *      Author: Shcheblykin
 */

#include <avr/io.h>
#include <util/delay.h>
#include "TM29f8000b.h"

/// \defgroup static_function Локальные функции
///@{

static void flashWrite(uint16_t adr, uint8_t byte);
static EError flashPoll(uint16_t adr, uint8_t val);
static void flashReadReset(void);

///@}

/**	Проверка микросхемы памяти.
 *
 *	Достаточно вызвать один раз, перед началом работы с памятью.
 *
 * 	-# Считывается производитель микросхемы и сравнивается с необходимым
 * 	\a #MANUFACTURER. В случае ошибки возвращается код \a #ERROR_MANUFACTURER.
 * 	-# Считывается код микросхемы и сравнивается с необходимым \a #DEVICE. В
 * 	случае ошибки возвращается код \a #ERROR_DEVICE.
 *	-# Если все совпало, возращается код \a #ERROR_NO.
 *
 *	@return Код ошибки.
 *	@retval #ERROR_NO
 *	@retval #ERROR_MANUFACTURER
 *	@retval #ERROR_DEVICE
 */
EError checkFlash(void) {
	EError err= ERROR_NO;

	// Шаг 1. Отправка инструкции "Read Electronic Signature".
	flashWrite(0x0AAAL, 0xAA);	/* 1st Cycle */
	flashWrite(0x0555L, 0x55);	/* 2nd Cycle */
	flashWrite(0x0AAAL, 0x90);	/* 3rd Cycle */

	// Шаг 2. Считывание кодов производителя и микросхемы.
	if (flashRead(0x0000) != MANUFACTURER) {
		// A0 = A1 = A6 = 0
		err = ERROR_MANUFACTURER;
	} else if (flashRead(0x0002L) != DEVICE) {
		// A0 = 1, A1 = A6 = 0, remember A-1
		err = ERROR_DEVICE;
	}

	// Шаг 3. Возврат в режим чтения.
	flashWrite(0x0000L, 0xF0);
	return err;
}

/**	Очистка блока памяти.
 *
 *	Во время стирания другие операции невозможны.
 *
 *	-# Отправка команд очистки блока памяти.
 *	-# Ожидание переключения бита DQ2.
 *	-# Ожидание установки бита очистки DQ7.
 *	-# Ожидание окончания очистки блока памяти.
 *	-# Возврат в режим чтения.
 *
 *	@param[in] num Номер очищаемого блока памяти [0..#NUM_BLOCKS).
 * 	@return Код ошибки.
 * 	@retval #ERROR_NO
 * 	@retval #ERROR_BLOCK
 * 	@retval #ERROR_ERASE
 *
 */
EError eraseBlock(uint8_t num) {
	uint8_t first, second;
	EError err = setBlock(num);

	if (err != ERROR_NO)
		return err;

	// - Запись команд сброса.
	flashWrite(0x0AAAUL, 0xAA);
	flashWrite(0x0555UL, 0x55);
	flashWrite(0x0AAAUL, 0x80);
	flashWrite(0x0AAAUL, 0xAA);
	flashWrite(0x0555UL, 0x55);
	flashWrite(0x0000UL, 0x30);


	//	Ожидание времени блокировки (смена значения DQ2).
	/// @todo Сделать проверку ожидания времени блокировки по таймеру.
	while(1) {
		first  = flashRead(0x0000UL) & 0x04;
		second = flashRead(0x0000UL) & 0x04;

		if (first != second)
			break;
	}

	//	Ожидание установки бита очистки.
	/// @todo Сделать проверку ожидания установки бита очистки по таймеру.
	while(1) {
		if ((flashRead(0x0000UL) & 0x08) == 0x08)
			break;
	}

	//	Ожидание окончания очистки блока (порядка 500мс).
	/// @todo Сделать ожидание окончания очистки блока по таймеру.
	if (flashPoll(0x0000UL, 0xFF) != ERROR_NO) {
		err = ERROR_ERASE;
	}

	// Шаг 5. Возврат в режим чтения.
	flashWrite(0x0000UL, 0xF0);

	return err;
}

/**	Запись одного байта данных.
 *
 *	-# Отправка команд перехода в режим записи.
 *	-# Запись байта данных.
 *	-# Проверка записи байта данных.
 *	-# Возврат в режим чтения.
 *
 *	@param[in] adr Адрес памяти в текущем блоке памяти.
 *	@param[in] byte Байт данных.
 * 	@return Код ошибки.
 * 	@retval #ERROR_NO
 * 	@retval #ERROR_PROGRAM_FAIL
 */
EError flashProgramByte(uint16_t adr, uint8_t byte) {
	// Шаг 1. Переход в режим записи.
	flashWrite(0x0AAAUL, 0xAA);
	flashWrite(0x0555UL, 0x55);
	flashWrite(0x0AAAUL, 0xA0);

	// Шаг 2. Запись байта данных.
	flashWrite(adr, byte);

	// Шаг 3. Проверка записи.
	if (flashPoll(adr, byte) != ERROR_NO) {
		flashReadReset();
		return ERROR_PROGRAM_FAIL;
	}

	// Шаг4. Возврат в режим чтения.
	flashWrite(0x0000L, 0xF0);
	return ERROR_NO;
}

/** Запись массива данных.
 *
 *	-# Цикл записи для каждого байта массива:
 *		- Переход в режим записи одного байта;
 *		- Запись байта данных;
 *		- Проверка записи байта данных.
 *	-# Возврат в режим чтения.
 *
 *	@param[in] adr Адрес в текущем блоке памяти.
 *	@param[in] buf[] Массив данных.
 *	@param[in] num Кол-во байт данных для записи.
 * 	@return Код ошибки.
 * 	@retval #ERROR_NO
 * 	@retval #ERROR_PROGRAM_FAIL
 */
EError flashProgramData(uint16_t adr, uint8_t buf[], uint8_t num) {

	for(uint8_t i = 0; i < num; i++, adr++) {
		// Шаг 1. Переход в режим записи.
		flashWrite(0x0AAAUL, 0xAA);
		flashWrite(0x0555UL, 0x55);
		flashWrite(0x0AAAUL, 0xA0);

		// Шаг 2. Запись байта данных.
		flashWrite(adr, buf[i]);

		// Шаг 3. Проверка записи байта данных.
		if (flashPoll(adr, buf[i]) != ERROR_NO) {
			flashReadReset();
			return ERROR_PROGRAM_FAIL;
		}
	}

	// Шаг4. Возврат в режим чтения.
	flashWrite(0x0000L, 0xF0);
	return ERROR_NO;
}

/**	Выбор блока для записи.
 *
 *	@param[in] num Номер блока памяти [0..#NUM_BLOCKS).
 *	@return Код ошибки.
 *	@retval #ERROR_NO
 * 	@retval #ERROR_PROGRAM_FAIL
 */
EError setBlock(uint8_t num) {
	if (num >= NUM_BLOCKS)
		return ERROR_BLOCK_INVALID;

	PORTF = num & 0x0F;

	return ERROR_NO;
}

/**	Чтение одного байта данных.
 *
 *	Чтение байта данных по адресу \a adr.
 *
 *	@param[in] adr Адрес в текущем блоке памяти.
 *	@return Байт данных.
 */
uint8_t flashRead(uint16_t adr) {
	return BASE_ADDR[adr];
}

/**	Запись байта данных.
 *
 * 	По адресу \a adr записывается байт данных \a byte.
 *
 * 	@param[in] adr Адрес в текущем блоке памяти.
 * 	@param[in] byte Данные.
 */
void flashWrite(uint16_t adr, uint8_t byte) {
	BASE_ADDR[adr] = byte;
}

/**	Опрос флэш.
 *
 *	-# Чтение DQ5 и DQ7.
 *	-# Если значение DQ7 не совпало с \a val (бит 7) то операция записи/очистки
 *	успешна.
 *	-# Ожидание установки бита DQ5.
 *	-# Проверка значения DQ7 на совпадение с \a val (бит 7), если совпали то
 *	операция записи/очистки успешна.
 *
 * 	@param[in] adr Адрес в текущем блоке памяти.
 * 	@param[in] val Байт данных.
 * 	@return Код ошибки.
 * 	@retval #ERROR_NO
 * 	@retval #ERROR_POLL
 *
 */
EError flashPoll(uint16_t adr, uint8_t val) {
	uint8_t uc; /* holds value read from valid offset */

	/// @todo Проверка ведется в бесконечном  цикле до появления успешного или
	/// ошибочного результата. Надо сделать по таймеру!
	while (1) {
		// Шаг 1. Чтение DQ5 и DQ7.
		uc = flashRead(adr);

		// Шаг 2. Если DQ7 совпало с val (бит 7) то операция успешна.
		if ((uc & 0x80) == (val & 0x80)) /* DQ7 == DATA */
			return ERROR_NO;

		// Шаг 3. Если DQ5 не 0, то операция еще не завершена.
		if ((uc & 0x20) == 0x00)
			continue;

		// Шаг 4. Снова проверяем DQ7 addr.
		uc = flashRead(adr);
		if ((uc & 0x80) == (val & 0x80))
			return ERROR_NO;
		else
			return ERROR_POLL;
	}

	return ERROR_POLL;
}

/**	Возврат в режим чтения.
 *
 *	-# Отправка команд перехода в режим чтения.
 *	-# Пауза минимум в 10 мкс.
 */
void flashReadReset(void) {
	// Шаг 1. Переход в режим чтения.
	flashWrite(0x0AAAL, 0xAA); /* 1st Cycle */
	flashWrite(0x0555L, 0x55); /* 2nd Cycle */
	flashWrite(0x0AAAL, 0xF0); /* 3rd Cycle */

	// Шаг 2. Пауза в 10 мкс.
	/// @todo Паузу надо сделать через таймер.
	_delay_us(10);
}

