/*
 */ 


#ifndef USART_H_
#define USART_H_

#ifndef F_CPU
	#warning "F_CPU not define"
#endif

#include <avr/io.h>
#include <avr/interrupt.h>

#include "avrlibtypes.h"
#include "bits_macros.h"

//------- Настройка USART, перенести в общий каталог солюшена
#define BAUD 38400

#define USART_MODE		(0<<UMSEL1)								//Async mode
#define USART_DATA_LEN	((0<<UCSZ12) | (1<<UCSZ11) | (1<<UCSZ10))	//8 Bit data length
#define USART_STOP_BIT	(0<<USBS1)								//1 Stop bit
#define USART_PARITY	((0<<UPM11) | (0<<UPM10))				//Parity disabled
#define usartRXIEnable	SetBit(UCSR1B, RXCIE1)					//Разрешить прерывание от приемника
#define usartRXIEdsable	ClearBit(UCSR1B, RXCIE1)				//Запретить прерывание от приемника
#define usartTXIEnable	SetBit(UCSR1B, UDRIE1)					//Разрешить прерывание по окончанию передачи
#define usartTXIEdisable	ClearBit(UCSR1B, UDRIE1)			//Запретить прерывание по окончанию передачи
#define TXIEisSet(flag)	BitIsSet(flag, UDRIE1)					//Прерывания по окончании разрешены?

#include <util/setbaud.h>										//Расчет коэффициентов делителей

//Буфера приема-передачи
#define USART_BUF_BUSY	0x80									//Буфер пуст или занят
#define USART_BUF_READY	0										//Данные в буфере готовы

#define RX_LEN_STR		128

#if (RX_LEN_STR>255)
	#warning "Слишком большой буфер"
#endif

struct usartBuf{
	u08 len;													//Количество символов в буфере
	u08 buf[RX_LEN_STR];										//Собственно сам буфер
};

extern volatile struct usartBuf usartRXbuf;
extern volatile struct usartBuf usartTXbuf;

void SerilalIni(void);
unsigned char usart_putchar(char c);
void usart_hex(unsigned char c);
void usart_putstr(char *str);									//ВЫвод строки


#endif /* USART_H_ */