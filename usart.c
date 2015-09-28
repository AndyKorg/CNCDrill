/*
 */ 

#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include "usart.h"
#include "CNCDrill.h"
#include "ComandList.h"
#include "EERTOS.h"

volatile struct usartBuf usartRXbuf;
volatile struct usartBuf usartTXbuf;

/************************************************************************/
/* Прием-передача USART													*/
/************************************************************************/
//-------- Инициализация USART
void SerilalIni(void){
	UCSR1B =	(1<<RXEN1) |									//Разрешить прием
				(1<<TXEN1);										//Разрешить передачу по USART
	usartRXIEnable;												//Разрешить прерывание от приемника, при начале передачи дополнительно разрешается прерывания при пустом регистре передатчика
	UCSR1C = USART_MODE | USART_DATA_LEN | USART_STOP_BIT | USART_PARITY;
	UBRR1H = UBRRH_VALUE;
	UBRR1L = UBRRL_VALUE;
	#if USE_2X
		UCSR1A |= (1 << U2X1);
	#else
		UCSR1A &= ~(0 << U2X1);
	#endif
	usartRXbuf.len = 0;
	usartTXbuf.len = 0;
}

//------ Собственно прием байта и деление на строки
ISR(USART1_RX_vect){
	u08 rxbyte = UDR1;
	u08 *cmd;
	
	//Специальная операция - Остановить операции
	if ((usartRXbuf.buf[0] == CMD_STOP)						//Первый байт команда остановки
		&& (usartRXbuf.len == 1)							//В буфере 1 байт
		&& ((rxbyte == 0xd) || (rxbyte == 0xa))				//Следующий конец строки
		)
		{		
		RaiseAlarm();
		usartRXbuf.len = 0;
		while(FIFO_COUNT(CmdBuf)){							//Очистить очередь
			cmd = FIFO_FRONT(CmdBuf);
			free((void*) cmd);
			FIFO_POP(CmdBuf);
		}
		FIFO_FLUSH(CmdBuf);
		return;
	}

	if (usartRXbuf.len == RX_LEN_STR){							//Буфер заполнен, сообщаем об ошибке
		CmdError();
		return;
	}
	else{
		usart_putchar(rxbyte);									//Эхо
	}
	if (((rxbyte == 0xd) || (rxbyte == 0xa)) && (usartRXbuf.len)){ //Конец строки, загнать строку в буфер
		if (!FIFO_IS_FULL(CmdBuf)){
			cmd = malloc(usartRXbuf.len);
			if (cmd != NULL){								//Удалось хапнуть память для строки
				memcpy((void*)cmd, (void*)usartRXbuf.buf, usartRXbuf.len);
				*(cmd+usartRXbuf.len) = 0;					//Завершающий 0
				FIFO_PUSH(CmdBuf, cmd);
				usartRXbuf.len = 0;
			}
		}
	}
	else{														//Пока идет строка, просто запоминаем ее
		if ((rxbyte >= 0x20) && (rxbyte <= 0x7e)){				//Только символы от 0x20 до 7e
			usartRXbuf.buf[usartRXbuf.len] = rxbyte;
			usartRXbuf.len++;
		}
	}
}

//------ Прерывание - передатчик пуст
ISR(USART1_UDRE_vect){
	static u08 CurrentChar = 0;
	
	if (usartTXbuf.len){
		UDR1 = usartTXbuf.buf[CurrentChar++];					//Передается очередной символ
		if (usartTXbuf.len == CurrentChar){						//Данных в буфере больше нет, передачу запрещаем
			usartTXIEdisable;
			CurrentChar = 0;
			usartTXbuf.len = 0;									//Все передано
		}
	}
}

//------ Помещает байт c в буфер передачи
unsigned char usart_putchar(char c){
	u08 storeInt = UCSR1B, Ret = USART_BUF_READY;				//Символ благополучно помещен в буфер
	
	while(usartTXbuf.len == RX_LEN_STR){						//Подождем пока буфер передачи освободится если он занят
		TaskManager();
	}
	usartTXIEdisable;											//Запретить прерывание от передатчика на время обработки
	usartTXbuf.buf[usartTXbuf.len++] = c;
	if TXIEisSet(storeInt)										//Если прерывания были разрешены то разрешить их снова
		usartTXIEnable;
	usartTXIEnable;												//Разрешается передача
	return Ret;
}

void usart_hex(unsigned char c){
	#define HI(b)	((b>>4) & 0xf)
	#define LO(b)	(b & 0xf)
	#define HE(b)	(((b & 0x7)-1) | 0x40)
	
	if (HI(c)>9)
		usart_putchar(HE(HI(c)));
	else
		usart_putchar(0x30 | HI(c));
	if (LO(c)>9)
		usart_putchar(HE(LO(c)));
	else
		usart_putchar(0x30 | LO(c));
}

//ВЫвод строки
void usart_putstr(char *str){
	while(*str){
		usart_putchar(*str);
		str++;
	}
}
