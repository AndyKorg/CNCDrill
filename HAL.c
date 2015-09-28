/*
	* HAL.c
	* �������:
	*	0 - ��� ������
	*	1 - ��� ��� � -- �� ����� ������������� ������ �����
	*	2 - RTOS
	*	3 - ��� ��� Z
 */ 

#include "HAL.h"
#include "ComandList.h"
#include "EERTOS.h"
#include "usart.h"

volatile u08 CommonFlags = 0,								//����� �����
			AxisFlags = 0,									//����� ����
			AxisStepSize,									//������ ���� ��� ���� � �������� �����������. ����� ������ ����������� ����� ��� �����, � ��� ��� ������������� ��� ���������� ������� Home
			AxisProtect = 0;

u16 KeyLedFlashPeriodMS = 250;								//������ ������� ������������ �����������, � �������������

volatile u16 XSpeed = X_SPEED_START,
			 YSpeed = Y_SPEED_START;

volatile tEncoder Encoder,									//������� ��������
				  EncoderOffset,							//�������� ���� ��� �������� �����
				  EncoderNeed;								//����������� �������� ��������

//================================ CODE ======================================
//��� ����������������
inline void PortInIni(u16 DDRPort, u16 OutPort, u08 Pin){
	ClearBit(DDRPort, Pin);					//���� �� ����
	SetBit(OutPort, Pin);					//�������� ��������
}

inline void PortOutIni(u16 DDRPort, u16 OutPort, u08 Pin){
	SetBit(DDRPort, Pin);
	SetBit(OutPort, Pin);
}

//���������� ���������� ����
void XAxisOff(void){
	XAxisStepOff();
	ClearBit(AxisProtect, AXIS_X);
}
void YAxisOff(void){
	_YAxisOff();
	ProcessEnd(AXIS_Y);
	ClearBit(AxisProtect, AXIS_Y);
}
void ZAxisOff(void){
	PWM_PinOff(Z_DRIVE_PWM_REG_A, Z_DRIVE_PWM_COM_1, Z_DRIVE_PWM_COM_0);
	ClearBit(Z_DRIVE_PWM_PORT, Z_DRIVE_PWM_OUT);					//��������� ������ ��������
	ClearBit(Z_DRIVE_PLUS_PORT, Z_DRIVE_PLUS_OUT);					//���������� ������ ��������� �� �����
	ClearBit(Z_DRIVE_MINUS_PORT, Z_DRIVE_MINUS_OUT);
	ProcessEnd(AXIS_Z);
	ClearBit(AxisProtect, AXIS_Z);
	
}
void DrillOff(void){
	PWM_PinOff(DRILL_DRIVE_PWM_REG_A, DRILL_DRIVE_PWM_COM_1, DRILL_DRIVE_PWM_COM_0);
	ClearBit(DRILL_DRIVE_PWM_PORT, DRILL_DRIVE_PWM_OUT);			//��������� ������ ��������
	ClearBit(DRILL_DRIVE_PLUS_PORT, DRILL_DRIVE_PLUS_OUT);			//���������� ������ ��������� �� �����
	ClearBit(DRILL_DRIVE_MINUS_PORT, DRILL_DRIVE_MINUS_OUT);
	ProcessEnd(AXIS_DRILL);
	ClearBit(AxisProtect, AXIS_DRILL);
}

//������ ����
ISR(KEY_STOP_INT_vect){
	if BitIsSet(KEY_STOP_DDR, KEY_STOP_PIN){						//���������� ���� � ���������� ����� ���� �����
		PortInIni(KEY_STOP_DDR, KEY_STOP_PORT_OUT, KEY_STOP_PIN);
	}
	DrillOff();
	XAxisOff();
	YAxisOff();
	ZAxisOff();
	if BitIsClear(CommonFlags, KEY_STOP_DOWN){						//��� �� ��������� ������ ���� ������
		usart_putstr(CMD_ANSWER_ALARM);
	}
	SetBit(CommonFlags, KEY_STOP_DOWN);
}

//����������� ��������� ���������� ������ � ������� �������� ���������� �� ������� ������ STOP
void RaiseAlarm(void){
usart_putchar('P');
	cli();
	SetBit(KEY_STOP_DDR, KEY_STOP_PIN);
	SetBit(KEY_STOP_PORT_OUT, KEY_STOP_PIN);
	sei();
}

//������ ���������
void DrvProt(enum AxisType Axis){
	u08 DrvOn=0;
	switch (Axis){
		case AXIS_X:
			if XAxisStepIsOn() DrvOn++;
			break;
		case AXIS_Y:
			if YAxisStepIsOn() DrvOn++;
			break;
		case AXIS_Z:
			if BitIsSet(Z_DRIVE_PWM_PORT, Z_DRIVE_PWM_OUT)//�� ��������
				DrvOn++;
			break;
		case AXIS_DRILL:
			break;
		default:
			break;
	}
	if (DrvOn){														//��������� �������
		if BitIsSet(AxisProtect, Axis){								//���� ������ �� ��� ��� ������� � �� ������� ��������� ��� ��������, �������� ������
			RaiseAlarm();
			usart_putstr("Axis ");
			usart_hex(Axis);
			usart_putstr(" protect\r\n");
		}
		else{
			SetBit(AxisProtect, Axis);								//������� ���� ������
		}
	}
	else{
		ClearBit(AxisProtect, Axis);
	}
}

//������ ���������� �� ������������
void DriveProtect(void){
#ifdef DEBUG
//	DrvProt(AXIS_X);
#endif
	//DriveProtect(AXIS_Y);//���� �� �����������, �.�. ��� �������� ����� ��� ���� ��� (��������)
	DrvProt(AXIS_Z);
	SetTimerTask(DriveProtect, AXIS_TIME_PROTECT);
}

//������������� ������ ���
void AxisPortIni(enum AxisType Axis){
	switch (Axis){
		case AXIS_X:
			SetBit(X_DIR_STEP_DRIVE_DDR, X_DIR_STEP_DRIVE_OUT);
			SetBit(X_SIZE1_STEP_DRIVE_DDR, X_SIZE1_STEP_DRIVE_OUT);
			SetBit(X_SIZE2_STEP_DRIVE_DDR, X_SIZE2_STEP_DRIVE_OUT);
			SetBit(X_STEP_STEP_DRIVE_DDR, X_STEP_STEP_DRIVE_OUT);
	
			ClearBit(X_STEP_STEP_DRIVE_PORT, X_STEP_STEP_DRIVE_OUT);
/*			PortOutIni(X_STEP_STEP_DRIVE_DDR, X_STEP_STEP_DRIVE_PORT, X_STEP_STEP_DRIVE_OUT);
			XAxisOff();
			ClearBit(X_STEP_STEP_DRIVE_PORT, X_STEP_STEP_DRIVE_OUT);//����� ���� ������� � ������
			PortOutIni(X_DIR_STEP_DRIVE_DDR, X_DIR_STEP_DRIVE_PORT, X_DIR_STEP_DRIVE_OUT);
			PortOutIni(X_SIZE1_STEP_DRIVE_DDR, X_SIZE1_STEP_DRIVE_PORT,	X_SIZE1_STEP_DRIVE_OUT);
			PortOutIni(X_SIZE2_STEP_DRIVE_DDR, X_SIZE2_STEP_DRIVE_PORT,	X_SIZE2_STEP_DRIVE_OUT);*/
			
			XAxisFullStep();
			PortInIni(X_MONITOR_SENS_DDR, X_MONITOR_SENS_OUT, X_MONITOR_SENS_PIN);
			SetBit(X_MONITOR_SENS_OUT, X_MONITOR_SENS_PIN);			//�������� �� ����� HOME
			//����� ��������
			//���������
			ClearBit(X_MAX_SENS_DDR, X_MAX_SENS_PIN);				//�� ����
			SetBit(X_MAX_SENS_OUT, X_MAX_SENS_PIN);					//��������
			ClearBit(X_MIN_SENS_DDR, X_MIN_SENS_PIN);
			SetBit(X_MIN_SENS_OUT, X_MIN_SENS_PIN);
			//�� ������ �������� ����� ��� �������� �� �����? � �������� ����� ���� ���-�� ��� 3.3 � �� ������ ���-��
			ClearBit(X_ENCOD1_SENS_DDR, X_ENCOD1_SENS_PIN);			//�� ����
			IntFalling(X_ENCOD1_SENS_INT_REG, X_ENCOD1_SENS_ISC1, X_ENCOD1_SENS_ISC0);
			SetBit(X_ENCOD1_SENS_INT_MASK, X_ENCOD1_SENS_INT);
			ClearBit(X_ENCOD2_SENS_DDR, X_ENCOD2_SENS_PIN);
			IntFalling(X_ENCOD2_SENS_INT_REG, X_ENCOD2_SENS_ISC1, X_ENCOD2_SENS_ISC0);
			SetBit(X_ENCOD2_SENS_INT_MASK, X_ENCOD2_SENS_INT);
			break;
		case AXIS_Y:													//������� ���������
			SetBit(Y_ENABL_STEP_DRIVE_DDR, Y_ENABL_STEP_DRIVE_OUT);		//���������-���������
			YAxisOff();
			SetBit(Y_DIR_STEP_DRIVE_DDR, Y_DIR_STEP_DRIVE_OUT);			//����������� ����
			SetBit(Y_STEP_STEP_DRIVE_DDR, Y_STEP_STEP_DRIVE_OUT);		//���
			SetBit(Y_SIZE1_STEP_DRIVE_DDR, Y_SIZE1_STEP_DRIVE_PORT);	//������ ����
			SetBit(Y_SIZE2_STEP_DRIVE_DDR, Y_SIZE2_STEP_DRIVE_OUT);
			YAxisStepOff();
			PortInIni(Y_MONITOR_SENS_DDR, Y_MONITOR_SENS_OUT, Y_MONITOR_SENS_PIN); //������� ��������
			//����� ��������
			//���������
			ClearBit(Y_MAX_SENS_DDR, Y_MAX_SENS_PIN);				//�� ����
			SetBit(Y_MAX_SENS_OUT, Y_MAX_SENS_PIN);					//��������
			ClearBit(Y_MIN_SENS_DDR, Y_MIN_SENS_PIN);
			SetBit(Y_MIN_SENS_OUT, Y_MIN_SENS_PIN);
			//�� ������ �������� ����� ��� �������� �� �����? � �������� ����� ���� ���-�� ��� 3.3 � �� ������ ���-��
			ClearBit(Y_ENCOD1_SENS_DDR, Y_ENCOD1_SENS_PIN);			//�� ����
			IntRising(Y_ENCOD1_SENS_INT_REG, Y_ENCOD1_SENS_ISC1, Y_ENCOD1_SENS_ISC0);
			SetBit(Y_ENCOD1_SENS_INT_MASK, Y_ENCOD1_SENS_INT);
			ClearBit(Y_ENCOD2_SENS_DDR, Y_ENCOD2_SENS_PIN);
			IntRising(Y_ENCOD2_SENS_INT_REG, Y_ENCOD2_SENS_ISC1, Y_ENCOD2_SENS_ISC0);
			SetBit(Y_ENCOD2_SENS_INT_MASK, Y_ENCOD2_SENS_INT);
			break;
		case AXIS_Z:
			SetBit(Z_DRIVE_PLUS_DDR, Z_DRIVE_PLUS_OUT);
			SetBit(Z_DRIVE_MINUS_DDR, Z_DRIVE_MINUS_OUT);
			SetBit(Z_DRIVE_PWM_DDR, Z_DRIVE_PWM_OUT);
			ZAxisOff();
			//��������� ��� - �����
			Z_DRIVE_PWM_REG_A = (Z_DRIVE_PWM_REG_A & (~((1<<Z_DRIVE_WGM1) | (1<<Z_DRIVE_WGM0)))) | ((1<<Z_DRIVE_WGM1) | (1<<Z_DRIVE_WGM0)); //Fast PWM 10 bit
			Z_DRIVE_PWM_REG_B = (Z_DRIVE_PWM_REG_B & (~((1<<Z_DRIVE_WGM3) | (1<<Z_DRIVE_WGM2)))) | ((0<<Z_DRIVE_WGM3) | (1<<Z_DRIVE_WGM2));
			Z_DRIVE_TCNT = 0;
			//������������
			Z_DRIVE_PWM_REG_B = (Z_DRIVE_PWM_REG_B & (~((1<<Z_DRIVE_CS2) | (1<<Z_DRIVE_CS1) | (1<<Z_DRIVE_CS0)))) | ((0<<Z_DRIVE_CS2) | (0<<Z_DRIVE_CS1) | (1<<Z_DRIVE_CS0)); //1
			//����� ��������
			//���������
			ClearBit(Z_HIGHT_SENS_DDR, Z_HIGHT_SENS_PIN);				//�� ����
			SetBit(Z_HIGHT_SENS_OUT, Z_HIGHT_SENS_PIN);					//��������
			ClearBit(Z_LOW_SENS_DDR, Z_LOW_SENS_PIN);
			SetBit(Z_LOW_SENS_OUT, Z_LOW_SENS_PIN);
			//����� ������� �������
			ClearBit(Z_ENCOD_SENS_DDR, Z_ENCOD_SENS_PIN);
			SetBit(Z_ENCOD_SENS_OUT, Z_ENCOD_SENS_PIN);
			break;
		case AXIS_DRILL:
			SetBit(DRILL_DRIVE_PLUS_DDR, DRILL_DRIVE_PLUS_OUT);
			SetBit(DRILL_DRIVE_MINUS_DDR, DRILL_DRIVE_MINUS_OUT);
			SetBit(DRILL_DRIVE_PWM_DDR, DRILL_DRIVE_PWM_OUT);
			DrillOff();
			//��������� ��� - �����
			DRILL_DRIVE_PWM_REG_A = (DRILL_DRIVE_PWM_REG_A & (~((1<<DRILL_DRIVE_WGM1) | (1<<DRILL_DRIVE_WGM0)))) | ((1<<DRILL_DRIVE_WGM1) | (1<<DRILL_DRIVE_WGM0)); //Fast PWM
			DRILL_DRIVE_TCNT = 0;
			//������������
			DRILL_DRIVE_PWM_REG_A = (DRILL_DRIVE_PWM_REG_A & (~((1<<DRILL_DRIVE_CS2) | (1<<DRILL_DRIVE_CS1) | (1<<DRILL_DRIVE_CS0)))) | ((0<<DRILL_DRIVE_CS2) | (0<<DRILL_DRIVE_CS1) | (1<<DRILL_DRIVE_CS0)); //1
			break;
		default:
			break;
	}
}

//����������� ���������
void DriveBreaking(enum AxisType Axis){
	switch (Axis){
		case AXIS_X:												//������� ���������
			break;
		case AXIS_Y:												//������� ���������
			break;
		case AXIS_Z:
			PWM_PinOff(Z_DRIVE_PWM_REG_A, Z_DRIVE_PWM_COM_1, Z_DRIVE_PWM_COM_0);
			SetBit(Z_DRIVE_PWM_PORT, Z_DRIVE_PWM_OUT);		
			ClearBit(Z_DRIVE_PLUS_PORT, Z_DRIVE_PLUS_OUT);	
			ClearBit(Z_DRIVE_MINUS_PORT, Z_DRIVE_MINUS_OUT);
			ProcessEnd(AXIS_Z);
			break;
		case AXIS_DRILL:
			PWM_PinOff(DRILL_DRIVE_PWM_REG_A, DRILL_DRIVE_PWM_COM_1, DRILL_DRIVE_PWM_COM_0);
			SetBit(DRILL_DRIVE_PWM_PORT, DRILL_DRIVE_PWM_OUT);		
			ClearBit(DRILL_DRIVE_PLUS_PORT, DRILL_DRIVE_PLUS_OUT);	
			ClearBit(DRILL_DRIVE_MINUS_PORT, DRILL_DRIVE_MINUS_OUT);
			ProcessEnd(AXIS_DRILL);
			break;
		default:
			break;
	}
}

//������� ���� ��� ������� ���������� ��� X, ����������� ����� XSpeed
void XStepGo(void){
	static u08 PinState = 0;
	if (XSpeed <= X_STEP_MIN_PERIOD)								//������ ����������� �������� �������, �� ����������� ����������
		XSpeed = X_STEP_MIN_PERIOD;
//	if BitIsSet(X_STEP_STEP_DRIVE_PORT, X_STEP_STEP_DRIVE_OUT){		//���� �������
	if (PinState){		//���� �������
		PinState = 0;
		ClearBit(X_STEP_STEP_DRIVE_PORT, X_STEP_STEP_DRIVE_OUT);
		if BitIsClear(X_MONITOR_SENS_IN, X_MONITOR_SENS_PIN){		//��� � ��������� ��� ������� ����� ������� ������ ����.
			switch StepSizeGet(XSizeStepMask, XSizeStepShift){
				case STEP_SIZE_FULL:
					XAxisFullStep();
					break;
				case STEP_SIZE_HALF:
					XAxisHalfStep();
					break;
				case STEP_SIZE_QUARTER:
					XAxisQuarterStep();
					break;
				case STEP_SIZE_EIGHTH:
					XAxisEighthStep();
					break;
				default:
					break;
			}
		}
	}
	else {
		if XAxisStepIsOn(){											//��� ��������, ������ ���
			PinState = 1;
			SetBit(X_STEP_STEP_DRIVE_PORT, X_STEP_STEP_DRIVE_OUT);	//����� ������� - ������� ���
		}
	}
	SetTimerTask(XStepGo, XSpeed);
}


//������� ���� ��� ������� ���������� ��� Y, ����������� ����� YSpeed
void YStepGo(void){
	if (YSpeed <= Y_STEP_MIN_PERIOD)								//������ ����������� �������� �������, �� ����������� ����������
		YSpeed = Y_STEP_MIN_PERIOD;
	if BitIsSet(Y_STEP_STEP_DRIVE_PORT, Y_STEP_STEP_DRIVE_OUT){		//���� �������
		ClearBit(Y_STEP_STEP_DRIVE_PORT, Y_STEP_STEP_DRIVE_OUT);
		if BitIsClear(Y_MONITOR_SENS_IN, Y_MONITOR_SENS_PIN){		//��� � ��������� ��� ������� ����� ������� ������ ����.
			switch StepSizeGet(YSizeStepMask, YSizeStepShift){
				case STEP_SIZE_FULL:
					YAxisFullStep();
					break;
				case STEP_SIZE_HALF:
					YAxisHalfStep();
					break;
				case STEP_SIZE_QUARTER:
					YAxisQuarterStep();
				case STEP_SIZE_EIGHTH:
					YAxisEighthStep();
					break;
				default:
					break;
			}
		}
	}
	else {
		if YAxisStepIsOn(){
			SetBit(Y_STEP_STEP_DRIVE_PORT, Y_STEP_STEP_DRIVE_OUT);	//����� �������
		}
	}
	SetTimerTask(YStepGo, YSpeed);
}

//�������� Stepping ��������� ��� Axis � ����������� Dir � ����� StepSize
void DriveSteppingOn(enum AxisType Axis, enum DirType Dir, enum StepSizeType StepSize){
	if BitIsSet(CommonFlags, KEY_STOP_DOWN)							//���� ������ ������ ����, ������ �� ��������
		return;
	switch (Axis){
		case AXIS_X:
			if (Dir == DIR_AXIS_TO_MAX){							//������
				if (SensorIsAlarmDirect(X_MAX_SENS_IN, X_MAX_SENS_PIN)){	//��� ����� � ������� ���������, ������������ �������
					break;
				}
				XAxisSetToMax();
			}
			else if (Dir == DIR_AXIS_TO_MIN){						//�����
				if (SensorIsAlarmDirect(X_MIN_SENS_IN, X_MIN_SENS_PIN)){	//��� ����� � ������� ���������, ������������ �������
					break;
				}
				XAxisSetToMin();
			}
			else{													//������������ �������
				RaiseAlarm();
				break;
			}
			StepSizeSet(StepSize, XSizeStepMask, XSizeStepShift);
			XAxisStepOn();
			break;
		case AXIS_Y:												//������� ���������
			if (Dir == DIR_AXIS_TO_MAX){
				if (SensorIsAlarmDirect(Y_MAX_SENS_IN, Y_MAX_SENS_PIN)){	//��� ����� � ������� ���������, ������������ �������
					break;
				}
				SetBit(Y_DIR_STEP_DRIVE_PORT, Y_DIR_STEP_DRIVE_OUT);
				YAxisSetToMax();
			}
			else if (Dir == DIR_AXIS_TO_MIN){
				if (SensorIsAlarmDirect(Y_MIN_SENS_IN, Y_MIN_SENS_PIN)){	//��� ����� � ������� ���������, ������������ �������
					break;
				}
				ClearBit(Y_DIR_STEP_DRIVE_PORT, Y_DIR_STEP_DRIVE_OUT);
				YAxisSetToMin();
			}
			else{
				RaiseAlarm();
				break;
			}
			StepSizeSet(StepSize, YSizeStepMask, YSizeStepShift);
			_YAxisOn();
			YAxisStepOn();											//������ ������
			ProcessStart(AXIS_Y);
			break;
		case AXIS_Z:
			//��� D� ��������� ��������� ����������
			break;
		case AXIS_DRILL:
			//��� D� ��������� ��������� ����������
			break;
		default:
			break;
	}
}

//�������� DC ��������� ��� Axis � ����������� Dir � ��������� Power 
void DriveDCOn(enum AxisType Axis, enum DirType Dir, u16 Power){
	if BitIsSet(CommonFlags, KEY_STOP_DOWN)							//���� ������ ������ ����, ������ �� ��������
		return;
	switch (Axis){
		case AXIS_X:
			break;
		case AXIS_Y:												//������� ���������
			break;
		case AXIS_Z:
			if (Dir == DIR_Z_UP){
				if (SensorIsAlarmInvert(Z_HIGHT_SENS_IN, Z_HIGHT_SENS_PIN)){	//��� ����� � ������� ���������, ������������ �������
					break;
				}
				SetBit(Z_DRIVE_PLUS_PORT, Z_DRIVE_PLUS_OUT);
				ClearBit(Z_DRIVE_MINUS_PORT, Z_DRIVE_MINUS_OUT);
				ZAxisSetToUp();
			}
			else if (Dir == DIR_Z_DOWN){
				if (SensorIsAlarmInvert(Z_LOW_SENS_IN, Z_LOW_SENS_PIN)){	//��� ����� � ������� ���������, ������������ �������
					break;
				}
				ClearBit(Z_DRIVE_PLUS_PORT, Z_DRIVE_PLUS_OUT);
				SetBit(Z_DRIVE_MINUS_PORT, Z_DRIVE_MINUS_OUT);
				ZAxisSetToDown();
			}
			else{													//������������ �������
				RaiseAlarm();
				break;
			}
			Z_DRIVE_OCR = Power;
			PWM_PinOn(Z_DRIVE_PWM_REG_A, Z_DRIVE_PWM_COM_1, Z_DRIVE_PWM_COM_0);
			ProcessStart(AXIS_Z);
			break;
		case AXIS_DRILL:
			if (Power > 0xff){										//��� ��������� �������� ������������ �� ����������� ��������
				Power = 0;
			}
			DRILL_DRIVE_OCR = (u08) Power;
			PWM_PinOn(DRILL_DRIVE_PWM_REG_A, DRILL_DRIVE_PWM_COM_1, DRILL_DRIVE_PWM_COM_0);
			if (Dir == DIR_DRILL_CLOCKWISE){
				ClearBit(DRILL_DRIVE_PLUS_PORT, DRILL_DRIVE_PLUS_OUT);
				SetBit(DRILL_DRIVE_MINUS_PORT, DRILL_DRIVE_MINUS_OUT);
			}
			else if (Dir == DIR_DRILL_ANTICLOCKWISE){
				SetBit(DRILL_DRIVE_PLUS_PORT, DRILL_DRIVE_PLUS_OUT);
				ClearBit(DRILL_DRIVE_MINUS_PORT, DRILL_DRIVE_MINUS_OUT);
			}
			else{													//������������ �������
				RaiseAlarm();
			}
			ProcessStart(AXIS_DRILL);
			break;
		default:
			break;
	}
}

//��������
//�
ISR(X_ENCOD1_SENS_INT_vect){	//�� ���� ���� �������� X2 ���������� � ������� 0xb ������ �������� X1 ��� �������� �� min � max � 0x4b ������ �� max � min 
	Encoder.X1++;				//������ ����� ��� ���������� �� �����, �.�. ������ �������
}

ISR(X_ENCOD2_SENS_INT_vect){
	if BitIsSet(X_ENCOD1_SENS_IN, X_MIN_SENS_PIN){	//�������� �� ������������� � 0
		Encoder.X2--;
	}
	else{											//�������� �� 0 � �������������
		Encoder.X2++;
	}
	ClearBit(AxisProtect, AXIS_X);					//�.�. ������� �������� �� ��������� ��������, ������ �������
}
//Y
ISR(Y_ENCOD1_SENS_INT_vect){
	Encoder.Y1++;
}
ISR(Y_ENCOD2_SENS_INT_vect){
	Encoder.Y2++;
}
//Z - ������������

//�������� ��� ���������� ������ ����
#define SENS_TEST_DIRECT	0
#define SENS_TEST_INVERT	1

inline u08 SensorTest(u16 SensorPortIn, u08 SensorPin, u08 *Flag, u08 FlagBit, u08 Invert){
	u08 Ret = 0, SensSet = 0;
	
	if (Invert == SENS_TEST_INVERT){
		if SensorIsAlarmInvert(SensorPortIn, SensorPin){
			SensSet++;
		}
	}
	else{
		if SensorIsAlarmDirect(SensorPortIn, SensorPin){
			SensSet++;
		}
	}
	if (SensSet){			//�������� �������
		if BitIsClear(*Flag, FlagBit){						//���������� �������� ���� �� �������
			SetBit(*Flag, FlagBit);							//������ ���������� �������
			Ret++;
#ifdef DEBUG
			usart_putchar('D');
			usart_hex(FlagBit);
			usart_putchar(0xd);
			usart_putchar(0xa);
#endif
		}
	}
	else{
		ClearBit(*Flag, FlagBit);							//���������� ��������� �� �������
	}
	return Ret;
}

void XEncoderReset(void){
	Encoder.X1 = 0;
	Encoder.X2 = 0;
}

//����� �������� �������� Z ������ ��������� ����� ����� ������� �� ��������� ������� ����� ������� �.�. �������� ��� �������� ����� �������� �����
void ZEncoderReset(void){
	Encoder.Z = 0;
}

//������������ ���������� ���� � �������� ��� Z. ������ ���� ��� ����� �����������������, �.�. �������������� � ����������
void AxisScan(void){
	
	static u08 ZEncoderPrev = 0, Flag = 0;
	
	#define XFlagMax			0
	#define XFlagMin			1
	#define YFlagMax			2
	#define YFlagMin			3
	#define ZFlagMax			4
	#define ZFlagMin			5
	

	if (SensorTest(X_MAX_SENS_IN, X_MAX_SENS_PIN, &Flag, XFlagMax, SENS_TEST_DIRECT) || SensorTest(X_MIN_SENS_IN, X_MIN_SENS_PIN, &Flag, XFlagMin, SENS_TEST_DIRECT)){	//X
		XAxisOff();															//������������� ���
		ProcessEnd(AXIS_X);
		if (SensorIsAlarmInvert(X_MAX_SENS_IN, X_MIN_SENS_PIN)){
			SetTimerTask(XEncoderReset, 50);
		}
#ifdef DEBUG
usart_putstr("Encoder X1=");
usart_hex((u08)((u16)(Encoder.X1)>>8));
usart_hex((u08)(Encoder.X1));
usart_putstr(" X2=");
usart_hex((u08)((u16)(Encoder.X2)>>8));
usart_hex((u08)(Encoder.X2));
usart_putchar(0xd);
usart_putchar(0xa);
#endif
	}
	if (SensorTest(Y_MAX_SENS_IN, Y_MAX_SENS_PIN, &Flag, YFlagMax, SENS_TEST_DIRECT) || SensorTest(Y_MIN_SENS_IN, Y_MIN_SENS_PIN, &Flag, YFlagMin, SENS_TEST_DIRECT)){	//Y
		YAxisOff();															//������������� ���
	}

	if (ZEncoderPrev != (Z_ENCOD_SENS_IN & Bit(Z_ENCOD_SENS_PIN))){			//������� Z ������������
		ZEncoderPrev = (Z_ENCOD_SENS_IN & Bit(Z_ENCOD_SENS_PIN));
		if ZAxisIsGoesToDown()
			Encoder.Z++;
		else
			Encoder.Z--;
		ClearBit(AxisProtect, AXIS_Z);										//�.�. ������� �������� �� ��������� ��������, ������ �������
		if ((EncoderNeed.Z == Encoder.Z) && (EncoderNeed.Z != ENCODER_OFF)){//���������� ������ �������� �������� � ����������� ���������� �������� ��������������, ������������� ��������� 
			ZAxisOff();
			//			DriveBreaking(AXIS_Z); �������
#ifdef DEBUG
usart_putstr("Encoder break Z=");
usart_hex((u08)((u16)(Encoder.Z)>>8));
usart_hex((u08)(Encoder.Z));
usart_putchar(0xd);
usart_putchar(0xa);
#endif
		}
	}
	if (SensorTest(Z_HIGHT_SENS_IN, Z_HIGHT_SENS_PIN, &Flag, ZFlagMax, SENS_TEST_INVERT) || SensorTest(Z_LOW_SENS_IN, Z_LOW_SENS_PIN, &Flag, ZFlagMin, SENS_TEST_INVERT)){//Z
//		DriveBreaking(AXIS_Z);
		ZAxisOff();															//������������� ���
		if (SensorIsAlarmInvert(Z_HIGHT_SENS_IN, Z_HIGHT_SENS_PIN)){		//��������� ����, ����� �������� ��� Z
			SetTimerTask(ZEncoderReset, 100);
		}
	}
}

//������������ ����������
void KeyScan(void){
	#define KEYS_ROWS_MAX	3										//���������� ����� � ����������

	static u08 Scan = 0;
	u08 ScanCode = 0, BitCount=0;
	
	if BitIsClear(KEY_COL1_PORT_IN, KEY_COL1_PIN){
		ScanCode = 0b00010000;
		BitCount++;
	}
	if BitIsClear(KEY_COL2_PORT_IN, KEY_COL2_PIN){
		ScanCode = 0b00100000;
		BitCount++;
	}
	if BitIsClear(KEY_COL3_PORT_IN, KEY_COL3_PIN){
		ScanCode = 0b01000000;
		BitCount++;
	}
	if BitIsClear(KEY_COL4_PORT_IN, KEY_COL4_PIN){
		ScanCode = 0b10000000;
		BitCount++;
	}
	if (ScanCode){													//���� ������� ������������
		if (BitCount == 1){											//������� �������, ���� ����������
			//Scan+ScanCode = ��� �������
			ScanCode = (ScanCode & 0xf0) | (Scan & 0x0f);
		}
	}
	if (Scan == 0) SetBit(KEY_ROW1_PORT_OUT, KEY_ROW1_PIN);			//��������� ���
	else ClearBit(KEY_ROW1_PORT_OUT, KEY_ROW1_PIN);
	if (Scan == 1) SetBit(KEY_ROW2_PORT_OUT, KEY_ROW2_PIN);
	else ClearBit(KEY_ROW2_PORT_OUT, KEY_ROW2_PIN);
	if (Scan == 2) SetBit(KEY_ROW3_PORT_OUT, KEY_ROW3_PIN);
	else ClearBit(KEY_ROW3_PORT_OUT, KEY_ROW3_PIN);
	Scan++;
	if (Scan == KEYS_ROWS_MAX)
		Scan = 0;
	SetTimerTask(KeyScan, KEY_PERIOD);
}

void KeyLedFlash(void){
	if BitIsClear(CommonFlags, KEY_LED_FLAG)
		KeyLedOff();
	else
		KEY_LED_PORT_OUT ^= Bit(KEY_LED_PIN);
	SetTimerTask(KeyLedFlash, KeyLedFlashPeriodMS);
}

//������������� ������
void InitHAL(void){
	//������ ����
	PortInIni(KEY_STOP_DDR, KEY_STOP_PORT_OUT, KEY_STOP_PIN);
	//	IntLowLevel(KEY_STOP_INT_REG, KEY_STOP_ISC1, KEY_STOP_ISC0);
	IntFalling(KEY_STOP_INT_REG, KEY_STOP_ISC1, KEY_STOP_ISC0);
	SetBit(KEY_STOP_INT_MASK, KEY_STOP_INT);
	TimerRTOSFunc = AxisScan;								//�������� ����������� ����
		
	//���
	AxisPortIni(AXIS_Z);									//��� Z ���������������� ������ � ���������� ����� ������� ����� �����
	AxisPortIni(AXIS_DRILL);
	AxisPortIni(AXIS_X);
	AxisPortIni(AXIS_Y);
	SetTimerTask(DriveProtect, AXIS_TIME_PROTECT);			//������ �� ������������

	//����������
	PortInIni(KEY_COL1_DDR, KEY_COL1_PORT_OUT, KEY_COL1_PIN);
	PortInIni(KEY_COL2_DDR, KEY_COL2_PORT_OUT, KEY_COL2_PIN);
	PortInIni(KEY_COL3_DDR, KEY_COL3_PORT_OUT, KEY_COL3_PIN);
	PortInIni(KEY_COL4_DDR, KEY_COL4_PORT_OUT, KEY_COL4_PIN);
	PortOutIni(KEY_ROW1_DDR, KEY_ROW1_PIN, KEY_ROW1_PIN);	//������ ��������� � ����� �� �������
	PortOutIni(KEY_ROW2_DDR, KEY_ROW2_PIN, KEY_ROW2_PIN);
	PortOutIni(KEY_ROW3_DDR, KEY_ROW3_PIN, KEY_ROW3_PIN);
	
	SetBit(KEY_LED_DDR, KEY_LED_PIN);
		
	SetTimerTask(KeyScan, KEY_PERIOD);						//�������� ������������ ����������
	
	SetTimerTask(XStepGo, XSpeed);							//������ ���������� ����� ��� �������� ��� X
	SetTimerTask(YStepGo, YSpeed);							//������ ���������� ����� ��� �������� ��� Y
	EncoderOffset.X2 = 0;									//�������� ���� ���� ���� :)
	EncoderOffset.Y2 = 0; 
	EncoderNeed.X1 = ENCODER_OFF;
	EncoderNeed.X2 = ENCODER_OFF;
	EncoderNeed.Y1 = ENCODER_OFF;
	EncoderNeed.Y2 = ENCODER_OFF;
	EncoderNeed.Z = ENCODER_OFF;
	
}