/*
 * Описание железа
 *
 * Created: 08.08.2015 8:39:25
 *  Author: Kirill
 */ 


#ifndef HAL_H_
#define HAL_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "avrlibtypes.h"
#include "bits_macros.h"

//Ось Х ---------------------------------------------
//Временное описание шагового двигателя на оси X для мезонинной платы а9377
#define X_DIR_STEP_DRIVE_DDR	DDRB
#define X_DIR_STEP_DRIVE_PORT	PORTB
#define X_DIR_STEP_DRIVE_OUT	PINB5
#define X_STEP_STEP_DRIVE_DDR	DDRF
#define X_STEP_STEP_DRIVE_PORT	PORTF
#define X_STEP_STEP_DRIVE_OUT	PINF3
#define X_SIZE1_STEP_DRIVE_DDR	DDRA
#define X_SIZE1_STEP_DRIVE_PORT	PORTA
#define X_SIZE1_STEP_DRIVE_OUT	PINA0
#define X_SIZE2_STEP_DRIVE_DDR	DDRA
#define X_SIZE2_STEP_DRIVE_PORT	PORTA
#define X_SIZE2_STEP_DRIVE_OUT	PINA1

#define X_MONITOR_SENS_DDR		DDRF					//Монитор шагового двигателя
#define X_MONITOR_SENS_OUT		PORTF
#define X_MONITOR_SENS_IN		PINF
#define X_MONITOR_SENS_PIN		PINF2
//Датчики
#define X_MAX_SENS_DDR			DDRC
#define X_MAX_SENS_OUT			PORTC
#define X_MAX_SENS_IN			PINC
#define X_MAX_SENS_PIN			PINC1
#define X_MIN_SENS_DDR			DDRC
#define X_MIN_SENS_OUT			PORTC
#define X_MIN_SENS_IN			PINC
#define X_MIN_SENS_PIN			PINC0
//Вспомогательный энкодер
#define X_ENCOD1_SENS_DDR		DDRD
#define X_ENCOD1_SENS_OUT		PORTD
#define X_ENCOD1_SENS_IN		PIND
#define X_ENCOD1_SENS_PIN		PIND1
#define X_ENCOD1_SENS_INT_REG	EICRA
#define X_ENCOD1_SENS_INT		INT1
#define X_ENCOD1_SENS_INT_MASK	EIMSK
#define X_ENCOD1_SENS_ISC0		ISC10
#define X_ENCOD1_SENS_ISC1		ISC11
#define X_ENCOD1_SENS_INT_vect	INT1_vect
//Основной энкодер
#define X_ENCOD2_SENS_DDR		DDRD
#define X_ENCOD2_SENS_OUT		PORTD
#define X_ENCOD2_SENS_IN		PIND
#define X_ENCOD2_SENS_PIN		PIND0
#define X_ENCOD2_SENS_INT_REG	EICRA
#define X_ENCOD2_SENS_INT		INT0
#define X_ENCOD2_SENS_INT_MASK	EIMSK
#define X_ENCOD2_SENS_ISC0		ISC00
#define X_ENCOD2_SENS_ISC1		ISC01
#define X_ENCOD2_SENS_INT_vect	INT0_vect

//Ось Y ---------------------------------------------
//Шаговый двигатель
#define Y_DIR_STEP_DRIVE_DDR	DDRG
#define Y_DIR_STEP_DRIVE_PORT	PORTG
#define Y_DIR_STEP_DRIVE_OUT	PORTG0
#define Y_STEP_STEP_DRIVE_DDR	DDRG
#define Y_STEP_STEP_DRIVE_PORT	PORTG
#define Y_STEP_STEP_DRIVE_OUT	PORTG1
#define Y_SIZE1_STEP_DRIVE_DDR	DDRG
#define Y_SIZE1_STEP_DRIVE_PORT	PORTG
#define Y_SIZE1_STEP_DRIVE_OUT	PORTG2
#define Y_SIZE2_STEP_DRIVE_DDR	DDRG
#define Y_SIZE2_STEP_DRIVE_PORT	PORTG
#define Y_SIZE2_STEP_DRIVE_OUT	PORTG3
#define Y_ENABL_STEP_DRIVE_DDR	DDRG
#define Y_ENABL_STEP_DRIVE_PORT	PORTG
#define Y_ENABL_STEP_DRIVE_OUT	PORTG4

#define Y_MONITOR_SENS_DDR		DDRE					//Монитор шагового двигателя
#define Y_MONITOR_SENS_OUT		PORTE
#define Y_MONITOR_SENS_IN		PINE
#define Y_MONITOR_SENS_PIN		PINE5

#define _YAxisOn()				ClearBit(Y_ENABL_STEP_DRIVE_PORT, Y_ENABL_STEP_DRIVE_OUT)
#define _YAxisOff()				SetBit(Y_ENABL_STEP_DRIVE_PORT, Y_ENABL_STEP_DRIVE_OUT)
//Датчики
#define Y_MAX_SENS_DDR			DDRC
#define Y_MAX_SENS_OUT			PORTC
#define Y_MAX_SENS_IN			PINC
#define Y_MAX_SENS_PIN			PINC2
#define Y_MIN_SENS_DDR			DDRC
#define Y_MIN_SENS_OUT			PORTC
#define Y_MIN_SENS_IN			PINC
#define Y_MIN_SENS_PIN			PINC3

#define Y_ENCOD1_SENS_DDR		DDRE
#define Y_ENCOD1_SENS_OUT		PORTE
#define Y_ENCOD1_SENS_IN		PINE
#define Y_ENCOD1_SENS_PIN		PINE6
#define Y_ENCOD1_SENS_INT_REG	EICRB
#define Y_ENCOD1_SENS_INT		INT6
#define Y_ENCOD1_SENS_INT_MASK	EIMSK
#define Y_ENCOD1_SENS_ISC0		ISC60
#define Y_ENCOD1_SENS_ISC1		ISC61
#define Y_ENCOD1_SENS_INT_vect	INT6_vect

#define Y_ENCOD2_SENS_DDR		DDRE
#define Y_ENCOD2_SENS_OUT		PORTE
#define Y_ENCOD2_SENS_IN		PINE
#define Y_ENCOD2_SENS_PIN		PINE7
#define Y_ENCOD2_SENS_INT_REG	EICRB
#define Y_ENCOD2_SENS_INT		INT7
#define Y_ENCOD2_SENS_INT_MASK	EIMSK
#define Y_ENCOD2_SENS_ISC0		ISC70
#define Y_ENCOD2_SENS_ISC1		ISC71
#define Y_ENCOD2_SENS_INT_vect	INT7_vect

//Ось Z ---------------------------------------------
//Двигатель
#define Z_DRIVE_PLUS_DDR		DDRA
#define Z_DRIVE_PLUS_PORT		PORTA
#define Z_DRIVE_PLUS_OUT		PORTA2
#define Z_DRIVE_MINUS_DDR		DDRA
#define Z_DRIVE_MINUS_PORT		PORTA
#define Z_DRIVE_MINUS_OUT		PORTA3

#define Z_DRIVE_PWM_DDR			DDRE					//ШИМ на ось Z
#define Z_DRIVE_PWM_OUT			PORTE3
#define Z_DRIVE_PWM_PORT		PORTE
#define Z_DRIVE_PWM_REG_A		TCCR3A
#define Z_DRIVE_PWM_REG_B		TCCR3B
#define Z_DRIVE_PWM_REG_C		TCCR3C
#define Z_DRIVE_PWM_COM_0		COM3A0
#define Z_DRIVE_PWM_COM_1		COM3A1
#define Z_DRIVE_WGM0			WGM30
#define Z_DRIVE_WGM1			WGM31
#define Z_DRIVE_WGM2			WGM32
#define Z_DRIVE_WGM3			WGM33
#define Z_DRIVE_CS0				CS30
#define Z_DRIVE_CS1				CS31
#define Z_DRIVE_CS2				CS32
#define Z_DRIVE_TCNT			TCNT3
#define Z_DRIVE_OCR				OCR3A

//Датчики
#define Z_HIGHT_SENS_DDR		DDRC
#define Z_HIGHT_SENS_OUT		PORTC
#define Z_HIGHT_SENS_IN			PINC
#define Z_HIGHT_SENS_PIN		PINC4
#define Z_LOW_SENS_DDR			DDRC
#define Z_LOW_SENS_OUT			PORTC
#define Z_LOW_SENS_IN			PINC
#define Z_LOW_SENS_PIN			PINC6
#define Z_ENCOD_SENS_DDR		DDRC
#define Z_ENCOD_SENS_OUT		PORTC
#define Z_ENCOD_SENS_IN			PINC
#define Z_ENCOD_SENS_PIN		PINC5

//Сверло ---------------------------------------------
#define DRILL_DRIVE_PLUS_DDR	DDRA
#define DRILL_DRIVE_PLUS_PORT	PORTA
#define DRILL_DRIVE_PLUS_OUT	PORTA4
#define DRILL_DRIVE_MINUS_DDR	DDRA
#define DRILL_DRIVE_MINUS_PORT	PORTA
#define DRILL_DRIVE_MINUS_OUT	PORTA5

#define DRILL_DRIVE_PWM_DDR		DDRB					//ШИМ на сверло
#define DRILL_DRIVE_PWM_OUT		PORTB4
#define DRILL_DRIVE_PWM_PORT	PORTB
#define DRILL_DRIVE_PWM_REG_A	TCCR0
#define DRILL_DRIVE_PWM_COM_0	COM00
#define DRILL_DRIVE_PWM_COM_1	COM01
#define DRILL_DRIVE_WGM0		WGM00
#define DRILL_DRIVE_WGM1		WGM01
#define DRILL_DRIVE_CS0			CS00
#define DRILL_DRIVE_CS1			CS01
#define DRILL_DRIVE_CS2			CS02
#define DRILL_DRIVE_TCNT		TCNT0
#define DRILL_DRIVE_OCR			OCR0

//Клавиатура ---------------------------------------------
//Специальная отдельная клавиша СТОП на отдельном прерывании
#define KEY_STOP_DDR			DDRE
#define KEY_STOP_PORT_OUT		PORTE
#define KEY_STOP_PORT_IN		PINE
#define KEY_STOP_PIN			PORTE4
#define KEY_STOP_INT_REG		EICRB					//Прерывание должно быть по переходу от 1 к 0
#define KEY_STOP_INT			INT4
#define KEY_STOP_INT_MASK		EIMSK
#define KEY_STOP_ISC0			ISC40
#define KEY_STOP_ISC1			ISC41
#define KEY_STOP_INT_vect		INT4_vect
//Матричная клавиатура 3х4
#define KEY_COL1_DDR			DDRE					//Колонки - ввод
#define KEY_COL1_PORT_OUT		PORTE
#define KEY_COL1_PORT_IN		PINE
#define KEY_COL1_PIN			PINE2
#define KEY_COL2_DDR			DDRA
#define KEY_COL2_PORT_OUT		PORTA
#define KEY_COL2_PORT_IN		PINA
#define KEY_COL2_PIN			PINA6
#define KEY_COL3_DDR			DDRA
#define KEY_COL3_PORT_OUT		PORTA
#define KEY_COL3_PORT_IN		PINA
#define KEY_COL3_PIN			PINA7
#define KEY_COL4_DDR			DDRC
#define KEY_COL4_PORT_OUT		PORTC
#define KEY_COL4_PORT_IN		PINC
#define KEY_COL4_PIN			PINC7

#define KEY_ROW1_DDR			DDRD					//Строки - вывод
#define KEY_ROW1_PORT_OUT		PORTD
#define KEY_ROW1_PORT_IN		PIND
#define KEY_ROW1_PIN			PORTD7
#define KEY_ROW2_DDR			DDRD
#define KEY_ROW2_PORT_OUT		PORTD
#define KEY_ROW2_PORT_IN		PIND
#define KEY_ROW2_PIN			PORTD6
#define KEY_ROW3_DDR			DDRD
#define KEY_ROW3_PORT_OUT		PORTD
#define KEY_ROW3_PORT_IN		PIND
#define KEY_ROW3_PIN			PORTD5

#define KEY_PERIOD				100						//Период опроса клавиатуры

//Светодиод на кнопке на матричной клавиатуре
#define KEY_LED_DDR				DDRD
#define KEY_LED_PORT_OUT		PORTD
#define KEY_LED_PORT_IN			PIND
#define KEY_LED_PIN				PORTD4

#define KeyLedOff()				ClearBit(KEY_LED_PORT_OUT, KEY_LED_PIN)
#define KeyLedOn()				SetBit(KEY_LED_PORT_OUT, KEY_LED_PIN)

//--------------------- Описание энкодеров
#define ENCODER_MIN			0							//Максимальные и минимальные значения энкодера
#define ENCODER_OFF			0xffff

typedef struct {										
	u16 X1;
	u16 X2;
	u16 Y1;
	u16 Y2;
	u16 Z;
} tEncoder;

extern volatile tEncoder Encoder,						//Текущие счетчики
						EncoderOffset,					//Смещение нуля для реальной платы
						EncoderNeed;					//Необходимые значения энкодера

//отключить ногу ШИМ
#define PWM_PinOff(TimerReg, TimerCom1, TimerCom0)	do {TimerReg &= ~((1<<TimerCom1) | (1<<TimerCom0));} while (0)
//включить ногу ШИМ
#define PWM_PinOn(TimerReg, TimerCom1, TimerCom0)	do {TimerReg = (TimerReg & (~((1<<TimerCom1) | (1<<TimerCom0)))) | ((1<<TimerCom1) | (0<<TimerCom0));} while (0)
//Прерывание по перепаду от 0 к 1
#define IntRising(Reg, Isc1, Isc0)	do {Reg = (Reg & (~((1<<Isc1) | (1<<Isc0)))) | ((1<<Isc1) | (1<<Isc0));} while (0)
//Прерывание по перепаду от 1 к 0
#define IntFalling(Reg, Isc1, Isc0)	do {Reg = (Reg & (~((1<<Isc1) | (1<<Isc0)))) | ((1<<Isc1) | (0<<Isc0));} while (0)
//Прерывание по любому перепаду не для всех прерываний доступно поэтому не сделано! (для  AtMega64 только от 4 и выше)
//Прерывание по низкому уровню
#define IntLowLevel(Reg, Isc1, Isc0)	do {Reg = (Reg & (~((1<<Isc1) | (1<<Isc0)))) | ((0<<Isc1) | (0<<Isc0));} while (0)

//Срабатывание оптронных концевиков. Высокий уровень - концевик сработал
#define SensorIsAlarmDirect(SensPort, SensPin) BitIsSet(SensPort, SensPin)
//Срабатывание концевиков - выключателей. Низкий уровень - концевик сработал
#define SensorIsAlarmInvert(SensPort, SensPin) BitIsClear(SensPort, SensPin)

//Обязательно должно быть от 0 до 3 поскольку используется как номера битов
enum AxisType{
	AXIS_X = 0,
	AXIS_Y = 1,
	AXIS_Z = 2,
	AXIS_DRILL = 3,
};

//------------------------------------ Двигатели
//Направления вращения двигателей, сделаны разные что бы не путается
enum DirType{
	DIR_Z_UP = 0,													//Поднять сверло
	DIR_Z_DOWN = 1,													//Опустить сверло
	DIR_AXIS_TO_MIN	= 2,											//Перемещать шпиндель к минимальному положению
	DIR_AXIS_TO_MAX	= 3,											//К максимальному
	DIR_DRILL_CLOCKWISE = 4,										//Вращать сверлом по часовой стрелке
	DIR_DRILL_ANTICLOCKWISE = 5,									//Против часовой
};

#define X_STEP_MIN_PERIOD	1										//Максимальная скорость шагания, больше нее шаговик не может шагать
#define X_SPEED_START		1										//Скорость шагания в миллисекундах умноженная на 2. По документам на двигатель M35SP-9 скорость шагания может достигать 700 Гц (1,4 ms)
#define Y_STEP_MIN_PERIOD	1										//Максимальная скорость шагания, больше нее шаговик не может шагать
#define Y_SPEED_START		50										//Скорость шагания в миллисекундах умноженная на 2

//DC моторы. Чем больше значение тем больше мощность на мотор. Для 16-и битного таймера 0xff - cкважинность ШИМ 25%, 0x1ff - 50%, 0x2ff - 75%
#define Z_SPEED_START		0xf										//Z
#define	DRIL_SPEED_START	0xf										//Начальная скорость вращения сверла


#define X_AXIS_MAX			1130									//В кликах энкодера
#define Z_AXIS_MAX			85										//Максимальное значение энкодера, получено экспериментально

#define AXIS_TIME_PROTECT	1000									//Если за этот период энкодер не сработает, то ось заклинило и двигатель оси надо выключить

extern volatile u08 CommonFlags,									//Общие флаги
					AxisFlags,										//Флаги осей
					AxisStepSize,									//Размер шага для осей с шаговыми двигателями. Здесь только указывается какой шаг нужен, а сам шаг переключается при разрешении сигнала Home
					AxisProtect;									//Флаги защиты осей
//Значения битов из CommonFlags
#define X_AXIS_BUSY			AXIS_X									//0 Ось X занята (идет операция) - не путать с X_AXIS_ON - включена!
#define Y_AXIS_BUSY			AXIS_Y									//1 -/- Y
#define Z_AXIS_BUSY			AXIS_Z									//2 -/- Z
#define DRiLL_BUSY			AXIS_DRILL								//3 -/- сверло
//4
//5
#define KEY_STOP_DOWN		6										//Нажата кнопка стоп, все операции с двигателями запрещены
#define KEY_LED_FLAG		7										//Мигать светодиодом на клавиатуре

#define KeyLedFlashOn()		SetBit(CommonFlags, KEY_LED_FLAG)
#define KeyLedFlashOff()	ClearBit(CommonFlags, KEY_LED_FLAG)

//Значения битов AxisFlag
#define X_AXIS_ON			AXIS_X									//0 Ось X включена, т.е. двигатель включен (выполняет шаги или ожидает команды ШАГ), не путать с X_AXIS_BUSY! идет операция на оси X (двигатель может быть выключен в ожидании стабилизации системы)
#define Y_AXIS_ON			AXIS_Y									//1 Ось Y включена
#define Z_AXIS_ON			AXIS_Z									//2 Ось Z включена
#define DRiLL_ON			AXIS_DRILL								//3 Сверло работает
//4							
//5
//6
#define Z_AXIS_TO_DOWN		7										//Ось Z движется вниз

enum StepSizeType{
	STEP_SIZE_FULL = 0,												//Специальные номера для определения микрошага
	STEP_SIZE_HALF = 1,
	STEP_SIZE_QUARTER = 2,
	STEP_SIZE_EIGHTH = 3,
};

/* Значения битов AxisStepSize:
	бит 1	бит 2	шаг
	0		0		Полный (Full)
	1		0		Половинный (Half)
	0		1		Одна четвертая (Quarter)
	1		1		Одна восьмая (Eagtht)
*/
#define X_STEP_SIZE1		0										//Биты для выводов микрошага для оси X, бит 1
#define X_STEP_SIZE2		1										//бит 2
#define Y_STEP_SIZE1		2
#define Y_STEP_SIZE2		3
//4
//5
//6
//7
#define XSizeStepShift		0										//Смещение для маски оси Х
#define XSizeStepMask		(Bit(X_STEP_SIZE1) | Bit(X_STEP_SIZE2))
#define YSizeStepShift		2										//Смещение для маски оси Y
#define YSizeStepMask		(Bit(Y_STEP_SIZE1) | Bit(Y_STEP_SIZE2))
//Установить требуемый размер шага для шагового двигателя. Само переключение будет выполнено в момент появления сигнала HOME
#define StepSizeSet(SizeStep, AxisMask, AxisShift) do {AxisStepSize = (AxisStepSize & AxisMask) | (SizeStep << AxisShift);} while (0)
#define StepSizeGet(AxisMask, AxisShift) ((AxisStepSize & AxisMask) >> AxisShift)

//Шагать осью X
#define XAxisStepOn()		SetBit(AxisFlags, X_AXIS_ON)
#define XAxisStepOff()		ClearBit(AxisFlags, X_AXIS_ON)
#define XAxisStepIsOn()		BitIsSet(AxisFlags, X_AXIS_ON)			//Подача команды ШАГ выключена
//Направление шага
#define XAxisSetToMax()		SetBit(X_DIR_STEP_DRIVE_PORT, X_DIR_STEP_DRIVE_OUT)		
#define XAxisSetToMin()		ClearBit(X_DIR_STEP_DRIVE_PORT, X_DIR_STEP_DRIVE_OUT)
#define XAxisGoesToMax()	BitIsSet(X_DIR_STEP_DRIVE_PORT, X_DIR_STEP_DRIVE_OUT)
//Размер шага
#define XAxisFullStep()		do{ClearBit(X_SIZE1_STEP_DRIVE_PORT, X_SIZE1_STEP_DRIVE_OUT); ClearBit(X_SIZE2_STEP_DRIVE_PORT, X_SIZE2_STEP_DRIVE_OUT);}while(0)
#define XAxisHalfStep()		do{SetBit(X_SIZE1_STEP_DRIVE_PORT, X_SIZE1_STEP_DRIVE_OUT); ClearBit(X_SIZE2_STEP_DRIVE_PORT, X_SIZE2_STEP_DRIVE_OUT);}while(0)
#define XAxisQuarterStep()	do{ClearBit(X_SIZE1_STEP_DRIVE_PORT, X_SIZE1_STEP_DRIVE_OUT); SetBit(X_SIZE2_STEP_DRIVE_PORT, X_SIZE2_STEP_DRIVE_OUT);}while(0)
#define XAxisEighthStep()	do{SetBit(X_SIZE1_STEP_DRIVE_PORT, X_SIZE1_STEP_DRIVE_OUT); SetBit(X_SIZE2_STEP_DRIVE_PORT, X_SIZE2_STEP_DRIVE_OUT);}while(0)

//Шагать осью Y
#define YAxisStepOn()		SetBit(AxisFlags, Y_AXIS_ON)			//Включить ось Y
#define YAxisStepOff()		ClearBit(AxisFlags, Y_AXIS_ON)
#define YAxisStepIsOn()		BitIsSet(AxisFlags, Y_AXIS_ON)			//Подача команды ШАГ выключена
//Нарпавление шага
#define YAxisSetToMax()		SetBit(Y_DIR_STEP_DRIVE_PORT, Y_DIR_STEP_DRIVE_OUT)
#define YAxisSetToMin()		ClearBit(Y_DIR_STEP_DRIVE_PORT, Y_DIR_STEP_DRIVE_OUT)
//Размер шага
#define YAxisFullStep()		do{ClearBit(Y_SIZE1_STEP_DRIVE_PORT, Y_SIZE1_STEP_DRIVE_OUT); ClearBit(Y_SIZE2_STEP_DRIVE_PORT, Y_SIZE2_STEP_DRIVE_OUT);}while(0)
#define YAxisHalfStep()		do{SetBit(Y_SIZE1_STEP_DRIVE_PORT, Y_SIZE1_STEP_DRIVE_OUT); ClearBit(Y_SIZE2_STEP_DRIVE_PORT, Y_SIZE2_STEP_DRIVE_OUT);}while(0)
#define YAxisQuarterStep()	do{ClearBit(Y_SIZE1_STEP_DRIVE_PORT, Y_SIZE1_STEP_DRIVE_OUT); SetBit(Y_SIZE2_STEP_DRIVE_PORT, Y_SIZE2_STEP_DRIVE_OUT);}while(0)
#define YAxisEighthStep()	do{SetBit(Y_SIZE1_STEP_DRIVE_PORT, Y_SIZE1_STEP_DRIVE_OUT); SetBit(Y_SIZE2_STEP_DRIVE_PORT, Y_SIZE2_STEP_DRIVE_OUT);}while(0)

//Занятость осей, не путать с включением-выключением! Занятость оси говорит о том что операция с осью еше выполняется несмотря на то что двигатель может быть выключен
#define ProcessGoing()		((CommonFlags & (Bit(X_AXIS_ON) | Bit(Y_AXIS_ON) | Bit(Z_AXIS_ON))) != 0)		//Идет обработка операции по осям
#define ProcessIsFree()		((CommonFlags & (Bit(X_AXIS_ON) | Bit(Y_AXIS_ON) | Bit(Z_AXIS_ON))) == 0)		//Станок свободен
#define ProcessStart(AxisBit) SetBit(CommonFlags, AxisBit)			//Занять ось
#define ProcessEnd(AxisBit)	ClearBit(CommonFlags, AxisBit)			//Освободить ось
#define ProcessAxisIsGo(AxisBit)	BitIsSet(CommonFlags, AxisBit)	//Работа с осью еще не закончена

#define ZAxisSetToDown()	SetBit(AxisFlags, Z_AXIS_TO_DOWN)
#define ZAxisSetToUp()		ClearBit(AxisFlags, Z_AXIS_TO_DOWN)
#define ZAxisIsGoesToDown()	BitIsSet(AxisFlags, Z_AXIS_TO_DOWN)

void InitHAL(void);													//Инициализация железа
void RaiseAlarm(void);												//Остановить все моторы
void AxisScan(void);												//Сканирование концевиков осей и энкодера оси Z. Должна быть как можно быстродействующей, т.к. отрабатывается в прерывании
void KeyScan(void);													//Сканирование клавиатуры
void PortInIni(u16 DDRPort, u16 OutPort, u08 Pin);
void PortOutIni(u16 DDRPort, u16 OutPort, u08 Pin);
void AxisPortIni(enum AxisType Axis);								//Инициализация портов оси

void DriveDCOn(enum AxisType Axis, enum DirType Dir, u16 Speed);		//Включить двигатель оси Axis в направлении Dir со скоростью Speed
void DriveSteppingOn(enum AxisType Axis, enum DirType Dir, enum StepSizeType StepSize);//Включить Stepping двигатель оси Axis в направлении Dir с шагом StepSize
void DriveBreaking(enum AxisType Axis);								//Затормозить двигатель
void XAxisOff(void);												//Выключение двигателей осей
void YAxisOff(void);
void ZAxisOff(void);
void DrillOff(void);
void SetStepSize(enum AxisType Axis, enum StepSizeType Size);		//Размер шага для шагового двигателя

#endif /* HAL_H_ */