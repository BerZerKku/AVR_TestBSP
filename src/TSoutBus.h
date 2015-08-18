/**	Класс работы с шиной сигналов Sout
 *
 */

#ifndef TSOUTBUS_H_
#define TSOUTBUS_H_

#include <avr/io.h>
#include <stdint.h>


/// Соответствие ножек МК и сигнализации.
typedef struct {
	volatile uint8_t *port;	///< Порт МК.
	uint8_t pin;			///< Номер вывода порта.
	bool inv;				///< Инверсия сигнала (true - есть, false - Нет)
} SPort;

/**	\brief Класс работы с шиной сигналов SOut (неисправности/аварии).
 *
 *
 */
class TSoutBus
{
	/// Количество задейстованных ножек сигнализации
	static const uint8_t s_u8NumPorts = 8;
	/// Расположение ножек сигнализации
	static const SPort s_stPorts[s_u8NumPorts];
public:
	
	/** Конструктор
	 *
	 * 	Производится настройка необходимых портов на выход и установка низкого
	 * 	логического уровня.
	 */
	TSoutBus();
	
	/**	Установка сигналов по маске.
	 *
	 * 	Сигнал устанавливается в 1, для соответствующей 1 в маске.
	 *
	 * 	@param mask Маска сигналов.
	 */
	void setMask(uint8_t mask);

	/**	Сброс сигналов по маске.
	 *
	 * 	Сигнал устанавливается в 0, для соответствующей 1 в маске.
	 *
	 * 	@param mask Маска сигналов.
	 */
	void clrMask(uint8_t mask);

	/**	Переключение сигналов по маске.
	 *
	 * 	Сигнал переключается, для соответствующей 1 в маске.
	 *
	 * 	@param mask Маска сигналов.
	 */
	void tglMask(uint8_t mask);
	
	/**	Установка сигналов.
	 *
	 * 	Сигналы устанавливаются в соответсвтии со значением \a val.
	 *
	 *	@param val Значение сигналов.
	 */
	void setValue(uint8_t val);
	
private:
	/**	Включение светодиода.
	 *
	 * 	@param num Номер светодиода.
	 */
	void ledOn(uint8_t num);

	/**	Выключение светодиода.
	 *
	 * 	@param num Номер светодиода.
	 */
	void ledOff(uint8_t num);
	
	/**	Переключение состояние светодиода.
	 *
	 * 	@param num Номер светодиода.
	 */
	void ledSwitch(uint8_t num);

};

#endif /* TSOUTBUS_H_ */
