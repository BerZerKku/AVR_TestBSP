/**	Класс работы с шиной сигналов Sout
 *
 */

#ifndef TSOUTBUS_H_
#define TSOUTBUS_H_

#include <avr/io.h>
#include <stdint.h>

/**	\brief Класс работы с шиной сигналов SOut (неисправности/аварии).
 *
 *
 */
class TSoutBus
{
	
public:
	
// Светодиод аварии зажигается нулем, остальные единицей
	void setAlarm() 	{ PORTB &= ~(1 << PB7); }
	void clrAlarm() 	{ PORTB |= (1 << PB7); }
	void tglAlarm() 	{ PORTB ^= (1 << PB7); }
	
	void setWarning() 	{ PORTB |= (1 << PB6); }
	void clrWarning() 	{ PORTB &= ~(1 << PB6); }
	void tglWarning() 	{ PORTB ^= (1 << PB6); }
	
	void setAlarmPrd() 	{ PORTB |= (1 << PB5); }
	void clrAlarmPrd() 	{ PORTB &= ~(1 << PB5); }
	void tglAlarmPrd() 	{ PORTB ^= (1 << PB5); }
	
	void setAlarmPrm() 	{ PORTB |= (1 << PB4); }
	void clrAlarmPrm() 	{ PORTB &= ~(1 << PB4); }
	void tglAlarmPrm() 	{ PORTB ^= (1 << PB4); }
	
	void setAlarmDef() 	{ PORTF |= (1 << PF3); }
	void clrAlarmDef() 	{ PORTF &= ~(1 << PF3); }
	void tglAlarmDef() 	{ PORTF ^= (1 << PF3); }
	
	void setComPrd()	{ PORTF |= (1 << PF2); }
	void clrComPrd()	{ PORTF &= ~(1 << PF2); }
	void tglComPrd() 	{ PORTF ^= (1 << PF2); }
	
	void setComPrm()	{ PORTF |= (1 << PF1); }
	void clrComPrm()	{ PORTF &= ~(1 << PF1); }
	void tglComPrm() 	{ PORTF ^= (1 << PF1); }
	
	void setCf()		{ PORTF |= (1 << PF0); }
	void clrCf()		{ PORTF |= (1 << PF0); }
	void tglCf()	 	{ PORTF ^= (1 << PF0); }
	
// Работа с сигналами по маске, применяется для бит установленных в единицу 
	void setMask(uint8_t mask) 		
	{ 
		PORTB |= (mask & 0x70); 
		PORTF |= mask & 0x0F; 
		
		if (mask & 0x80)
			setAlarm();
	}
	void clrMask(uint8_t mask)
	{
		PORTB &= ~(mask & 0x70);
		PORTF &= ~(mask & 0x0F);
		if (mask & 0x80)
			clrAlarm();
	}
	void tglMask(uint8_t mask)
	{
		PORTB ^= (mask & 0xF0);
		PORTF ^= (mask & 0x0F);

	}
	
// 	Вывод значения на светодиоды
	void setValue(uint8_t val)
	{
		uint8_t tmp;
		
		tmp = PORTB & 0x0F;
		tmp += (val	^ 0x80) & 0xF0;
		PORTB = tmp;
		
		tmp = PORTF & 0xF0;
		tmp += val & 0x0F;
		PORTF = tmp;
	}	
	
private:
	
};

#endif /* TSOUTBUS_H_ */
