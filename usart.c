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
/* �����-�������� USART													*/
/************************************************************************/
//-------- ������������� USART
void SerilalIni(void){
	UCSR1B =	(1<<RXEN1) |									//��������� �����
				(1<<TXEN1);										//��������� �������� �� USART
	usartRXIEnable;												//��������� ���������� �� ���������, ��� ������ �������� ������������� ����������� ���������� ��� ������ �������� �����������
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

//------ ���������� ����� ����� � ������� �� ������
ISR(USART1_RX_vect){
	u08 rxbyte = UDR1;
	u08 *cmd;
	
	//����������� �������� - ���������� ��������
	if ((usartRXbuf.buf[0] == CMD_STOP)						//������ ���� ������� ���������
		&& (usartRXbuf.len == 1)							//� ������ 1 ����
		&& ((rxbyte == 0xd) || (rxbyte == 0xa))				//��������� ����� ������
		)
		{		
		RaiseAlarm();
		usartRXbuf.len = 0;
		while(FIFO_COUNT(CmdBuf)){							//�������� �������
			cmd = FIFO_FRONT(CmdBuf);
			free((void*) cmd);
			FIFO_POP(CmdBuf);
		}
		FIFO_FLUSH(CmdBuf);
		return;
	}

	if (usartRXbuf.len == RX_LEN_STR){							//����� ��������, �������� �� ������
		CmdError();
		return;
	}
	else{
		usart_putchar(rxbyte);									//���
	}
	if (((rxbyte == 0xd) || (rxbyte == 0xa)) && (usartRXbuf.len)){ //����� ������, ������� ������ � �����
		if (!FIFO_IS_FULL(CmdBuf)){
			cmd = malloc(usartRXbuf.len);
			if (cmd != NULL){								//������� ������� ������ ��� ������
				memcpy((void*)cmd, (void*)usartRXbuf.buf, usartRXbuf.len);
				*(cmd+usartRXbuf.len) = 0;					//����������� 0
				FIFO_PUSH(CmdBuf, cmd);
				usartRXbuf.len = 0;
			}
		}
	}
	else{														//���� ���� ������, ������ ���������� ��
		if ((rxbyte >= 0x20) && (rxbyte <= 0x7e)){				//������ ������� �� 0x20 �� 7e
			usartRXbuf.buf[usartRXbuf.len] = rxbyte;
			usartRXbuf.len++;
		}
	}
}

//------ ���������� - ���������� ����
ISR(USART1_UDRE_vect){
	static u08 CurrentChar = 0;
	
	if (usartTXbuf.len){
		UDR1 = usartTXbuf.buf[CurrentChar++];					//���������� ��������� ������
		if (usartTXbuf.len == CurrentChar){						//������ � ������ ������ ���, �������� ���������
			usartTXIEdisable;
			CurrentChar = 0;
			usartTXbuf.len = 0;									//��� ��������
		}
	}
}

//------ �������� ���� c � ����� ��������
unsigned char usart_putchar(char c){
	u08 storeInt = UCSR1B, Ret = USART_BUF_READY;				//������ ������������ ������� � �����
	
	while(usartTXbuf.len == RX_LEN_STR){						//�������� ���� ����� �������� ����������� ���� �� �����
		TaskManager();
	}
	usartTXIEdisable;											//��������� ���������� �� ����������� �� ����� ���������
	usartTXbuf.buf[usartTXbuf.len++] = c;
	if TXIEisSet(storeInt)										//���� ���������� ���� ��������� �� ��������� �� �����
		usartTXIEnable;
	usartTXIEnable;												//����������� ��������
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

//����� ������
void usart_putstr(char *str){
	while(*str){
		usart_putchar(*str);
		str++;
	}
}
