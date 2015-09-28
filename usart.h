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

//------- ��������� USART, ��������� � ����� ������� ��������
#define BAUD 38400

#define USART_MODE		(0<<UMSEL1)								//Async mode
#define USART_DATA_LEN	((0<<UCSZ12) | (1<<UCSZ11) | (1<<UCSZ10))	//8 Bit data length
#define USART_STOP_BIT	(0<<USBS1)								//1 Stop bit
#define USART_PARITY	((0<<UPM11) | (0<<UPM10))				//Parity disabled
#define usartRXIEnable	SetBit(UCSR1B, RXCIE1)					//��������� ���������� �� ���������
#define usartRXIEdsable	ClearBit(UCSR1B, RXCIE1)				//��������� ���������� �� ���������
#define usartTXIEnable	SetBit(UCSR1B, UDRIE1)					//��������� ���������� �� ��������� ��������
#define usartTXIEdisable	ClearBit(UCSR1B, UDRIE1)			//��������� ���������� �� ��������� ��������
#define TXIEisSet(flag)	BitIsSet(flag, UDRIE1)					//���������� �� ��������� ���������?

#include <util/setbaud.h>										//������ ������������� ���������

//������ ������-��������
#define USART_BUF_BUSY	0x80									//����� ���� ��� �����
#define USART_BUF_READY	0										//������ � ������ ������

#define RX_LEN_STR		128

#if (RX_LEN_STR>255)
	#warning "������� ������� �����"
#endif

struct usartBuf{
	u08 len;													//���������� �������� � ������
	u08 buf[RX_LEN_STR];										//���������� ��� �����
};

extern volatile struct usartBuf usartRXbuf;
extern volatile struct usartBuf usartTXbuf;

void SerilalIni(void);
unsigned char usart_putchar(char c);
void usart_hex(unsigned char c);
void usart_putstr(char *str);									//����� ������


#endif /* USART_H_ */