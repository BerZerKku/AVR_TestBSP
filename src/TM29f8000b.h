// @file TM29f8000b.h
/**
 *	@mainpage
 *	@version 1.0
 *	@author Щеблыкин М.В.
 * 	@date 18 Августа 2015
 *
 *	@brief Набор функций для работы с микросхемой памяти M29F800FT MICRON.
 *
 *	Начало работы с памятью:
 *	-# Записать в регистр ПЛИС init значение 0x55;
 *	-# Установить 0 номер блока памяти \a setBlock();
 *	-# Проверить что установлена нужная память \a checkFlash();
 *	-# Стереть все используемые блоки, данные в которых не нужны \a eraseBlock().
 *	-# Установить нужный блок памяти  \a setBlock();
 *	-# Записать данные по одному байту \a flashProgramByte() или сразу массив
 *	\a flashProgramData().
 *
 *	Блок памяти используется не полностью, а начиная с #BASE_ADDR. Если оставить
 *	адрес равным 0х8000, то не потребуется каких-либо изменений в железе и
 *	прошивке ПЛИС. При этом в каждом блоке будет использоваться только половина
 *	памяти.
 *
 *	Для перезаписи параметров можно сначала считать их все, очистить блок памяти
 *	и затем записать все параметры в память.
 *
 *	Написано и проверено в WinAVR-2010010.
 *
 *	@note Время записи одного байта данных порядка 15 мкс, стирания блока - 0,5 сек.
 *
 *	@todo Сделать возможность использования блоков памяти в старших адресах.
 *
 *	@warning Все циклы ожидания необходимо заменить на опрос или паузу по таймеру.
 *
 *	@bug Не работает если в старшем разряде адреса #BASE_ADDR стоит 0.
 */
#ifndef TM29F8000B_H_
#define TM29F8000B_H_

#include <stdint.h>

/// \defgroup define Макросы и перечисления
///@{

/// Начальный адрес (в каждом блоке).
#define BASE_ADDR ((volatile uint8_t*) 0x8000UL)
/// Код производителя (MICRON = 0x01).
#define MANUFACTURER (0x01)
/// Код микросхемы (M29F800FT MICRON = 0xD6).
#define DEVICE (0xD6)
/// Количество используемых блоков памяти.
#define NUM_BLOCKS (15)
///< Размер блока памяти. Учитывается базовый адрес.
#define BLOCK_SIZE (0xFFFF - (uint16_t) BASE_ADDR)

/// Коды ошибок.
typedef enum __attribute__ ((__packed__)) {
	ERROR_NO 			= 0,	///< Ошибок нет.
	ERROR_MANUFACTURER	= 1,	///< Ошибочный код производителя микросхемы.
	ERROR_DEVICE		= 2,	///< Ошибочный код микросхемы памяти.
	ERROR_BLOCK			= 3,	///< Ошибочный номер блока памяти микросхемы.
	ERROR_POLL			= 4,	///< Ошибка опроса микросхемы.
	ERROR_PROGRAM_FAIL	= 5,	///< Ошибка записи.
	ERROR_BLOCK_INVALID	= 6,	///< Ошибочный номер блока памяти.
	ERROR_ERASE			= 7,	///< Ошибка очистки микросхемы.
	ERROR_BUFF_OVF		= 8		///< Ошибка переполение буфера.
} EError;

typedef enum __attribute__ ((__packed__)) {
	STATE_CHECK_INIT	= 0,	///< Идет проверка микросхемы.
	STATE_READ			= 1,	///< Чтение.
	STATE_WRITE_PARAM	= 2,	///< Идет запись параметров.
	STATE_WRITE_LOG		= 3,	///< Идет запись журнала.
	STATE_ERASE_BLOCK	= 4		///< Идет очистка блока памяти.
} EState;

///@}

/// \defgroup global_function Глобальные функции
///@{

extern EError checkFlash(void);
extern EError eraseBlock(uint8_t num);
extern uint8_t flashRead(uint16_t adr);
extern EError flashProgramByte(uint16_t adr, uint8_t byte);
extern EError flashProgramData(uint16_t adr, uint8_t buf[], uint8_t num);
extern EError setBlock(uint8_t num);

extern EError push(uint8_t byte);
extern EError poll(void);
extern EState getState(void);
///@}

#endif /* TM29F8000B_H_ */
