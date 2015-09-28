/*
 * CNCDrill.h
 *
 * Created: 07.07.2015 9:44:01
 *  Author: Admin
 */ 


#ifndef CNCDRILL_H_
#define CNCDRILL_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "avrlibtypes.h"
#include "bits_macros.h"
#include "FIFO.h"

#include "HAL.h"

typedef void (*FINISH_CMD)(void);			
extern FINISH_CMD XFinish, YFinish, ZFinish;

#define cmdByte(cmd, byte) (*(cmd+byte))							//�������� ���� �� ������
#define RET_OK				1
#define RET_ERR				0

void CmdError(void);												//��������� �� ������

typedef struct {
	u16 EncodeCount;												//���������� ����� ��������
	u32 Time;														//����� ��� ����� ���������� ����� ��������
} tMeasure;

FIFO(16) CmdBuf;

//----------------------------------- ������� USART (N/C Drill ���� ��������� ��������� ������) -----------------------------------
typedef u08 (*VOID_CMD)(u08 *cmd);
typedef const PROGMEM struct{
	char Code[8];													//��� �������
	VOID_CMD Func;													//������� ����������� �������
} PROGMEM pCmdElement;

#endif /* CNCDRILL_H_ */
