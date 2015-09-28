/***********************************************************
	ЯДОРО RTOS
	Автор DI-HALT http://easyelectronics.ru/
***********************************************************/
#ifndef EERTOS_H
#define EERTOS_H

#include "EERTOSHAL.h"

extern void InitRTOS(void);
extern void Idle(void);

typedef void (*TPTR)(void);

extern void SetTask(TPTR TS);
extern void SetTimerTask(TPTR TS, u16 NewTime);

extern void TaskManager(void);
extern void TimerService(void);

extern TPTR TimerRTOSFunc;							//Функция вызываемая при каждом срабатывании таймера задач, должна быть как можно менее длительной
extern long unsigned int TiksCount;

#endif
