/*
 * CNCDrill.c
 *
 * Created: 04.07.2015 17:04:38
 *  Author: Admin
 */ 

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "CNCDrill.h"
#include "ComandList.h"
#include "EERTOS.h"
#include "usart.h"

FINISH_CMD XFinish = NULL, YFinish = NULL, ZFinish = NULL;	//������� ����������� �� ��������� ����������� ���

#define MEASURE_NUM			3								//���������� ���������
#define MEASURE_MAX			(2*MEASURE_NUM)
tMeasure Measure[MEASURE_MAX];
u08 MeasureCount = 0;										//����� �������� ���������

#define IsDigit(symbol)		((symbol >= '0') && (symbol <= '9'))

//------ N/C Drill
volatile u08 LanguageFlags = 0;								//����� ������������ �������������� �����
#define SCRIPT_STATED		0								//���� ������� ������ ���������
#define	SCRIPT_HEADER_END	1								//��������� ��������� ��������� ��������
//2
//3
//4
//5
//6
//7

#define CmdHeaderSet()		SetBit(LanguageFlags, SCRIPT_HEADER_END)
#define CmdHeaderClear()	ClearBit(LanguageFlags, SCRIPT_HEADER_END)
#define CmdHeaderIsSet()	BitIsSet(LanguageFlags, SCRIPT_HEADER_END)
#define CmdHeaderIsClear()	BitIsClear(LanguageFlags, SCRIPT_HEADER_END)

#define CmdStartSet()		SetBit(LanguageFlags, SCRIPT_STATED)
#define CmdStartClear()		do{ClearBit(LanguageFlags, SCRIPT_STATED); CmdHeaderClear();}while(0)
#define CmdStartIsSet()		BitIsSet(LanguageFlags, SCRIPT_STATED)
#define CmdStartIsClear()	BitIsClear(LanguageFlags, SCRIPT_STATED)

char *ErrorText = NULL;										//����� ������

const char PROGMEM											//������ ������
	err_metric[] = "Command METRIC is bad",
	err_metric_unexp[] = "Unexpected METRIC comand",
	err_metric_undef[] = "Not specified Accuracy",
	err_start_unexp[] = "Unexpected start comand",
	err_header_unexp[] = "Unexpected header end comand",
	err_tool_unexp[] = "Unexpected T comand",
	err_tool[] = "Comand T syntax error",
	err_tool_more[] = "description tools more 5",
	err_tool_repeat[] = "Repeated description of the tool",
	err_tool_undesc[] = "Tool undescribed",
	err_cmd_mem[] = "Out of emeory",
	err_cmd_unknown[] = "Unknown command";

u08 ZeroCountBefor = 0, ZeroCountAfter = 0;					//������ ����� ���������� Before - ���������� ���� � ����� �����, After - � �������
#define TOOL_DESCR_MAX		5								//������������ ���������� �������� �����������
char ToolsDescription[TOOL_DESCR_MAX][4];					//�������� ����������� �� ������� TXXCXX.X

u08 StartHead(u08 *val);									//������ ���������
u08 Metric(u08 *val);										//���������� � �����������
u08 ChangeAndStop(u08 *val);								//������� ���������� ��� T0 - ���������� ��� �����������
u08 Position(u08 *val);										//����������� ����������� �� ��������� ����������. ���������� ������������� �� �������������� ����
u08 EndProgeam(u08 *val);									//����� ���������
u08 EndHeader(u08 *val);									//����� ������������� ���������
//------ Service
u08 ForwardX(u08 *val);										//����� X ������
u08 RewindX(u08 *val);										//����� X �����
u08 ShiftSetX(u08 *val);
u08 ForwardY(u08 *val);										//Y
u08 RewindY(u08 *val);
u08 DownZ(u08 *val);										//Z
u08 UpZ(u08 *val);
u08 ShiftSetZ(u08 *val);
u08 GetEncoders(u08 *val);
u08 ForwardDrill(u08 *val);									//Drill
u08 RewindDrill(u08 *val);
u08 StopDrill(u08 *val);
u08 EncoderReset(u08 *val);

const pCmdElement PROGMEM ListCmd[CMD_NUM_MAX] = {
	//Code			Func
	{CMD_NC_HEAD, StartHead},								//������ ���������
	{CMD_NC_METRIC, Metric},								//���������� � �����������
	{CMD_NC_CHANGE_TOOL, ChangeAndStop},					//������� ���������� ��� T0 - ���������� ��� �����������
	{CMD_NC_POS, Position},									//����������� ����������� �� ��������� ����������. ���������� ������������� �� �������������� ����
	{CMD_NC_END, EndProgeam},								//����� ���������
	{CMD_NC_END_HEADER, EndHeader},							//����� ������������� ���������
	{CMD_FORWARD_X, ForwardX},								//������ ������� - ����� X ������
	{CMD_REWIND_X, RewindX},								//�����
	{CMD_SHIFT_X, ShiftSetX},
	{CMD_FORWARD_Y, ForwardY},
	{CMD_REWIND_Y, RewindY},
	{CMD_DOWN_Z, DownZ},
	{CMD_UP_Z, UpZ},
	{CMD_SHIFT_Z, ShiftSetZ},
	{CMD_GET_ENCODER, GetEncoders},
	{CMD_FORWARD_DRILL, ForwardDrill},
	{CMD_REWIND_DRILL, RewindDrill},
	{CMD_STOP_DRILL, StopDrill},							//���������� �������� ������
	{CMD_ENCODE_RESET, 	EncoderReset},
};

#define CmdExec(id)			*((VOID_CMD*)pgm_read_word(&ListCmd[id].Func))


//������ � ���� �������
void CmdError(void){
	usart_putstr(CMD_ANSWER_ERR);
	if (ErrorText != NULL){
		usart_putstr(ErrorText);
		free(ErrorText);
		ErrorText = NULL;
	}
}

//������� ����� ������
void CreateError(const char *str_P){
	u08 len = strlen_P(str_P);
	ErrorText = malloc(len+1);
	if (ErrorText != NULL){
		strcpy_P(ErrorText, str_P);
	}
	CmdStartClear();
}

//����������� ������ ���� � ����� �� ������� ����������� �������
s32 strToLong(char *str){
	u08 i = 0, flag = 0;
	s32 Ret = 0;
	
	for (;i<strlen((char*)str);i++){						//���� �����-�� �����
		if (*(str+i) != '0'){
			flag++;
		}
	}
	if (flag){
		Ret = strtol((char*)str, NULL, 10);
	}
	return Ret;
}

//��� ������� � ������ ������ ���� �������. ������ ������ ������ ���� ���� ����� ���� ����
u08 StrDigitTest(char *str){
	u08 Ret = RET_ERR, i = (strlen((str)+1));
	
	if ((cmdByte(str, 0) == '+') || (cmdByte(str, 0) == '-')){
		for (;i;i--){
			if ((*(str+i) < '0') || (*(str+i) > '0')) break;
		}
		if (i == 0) Ret = RET_OK;
	}
	return Ret;
}

//================================================================ ������� ����� "N/C Drill"
//������ ���������
u08 StartHead(u08 *val){
	u08 Ret = RET_ERR, i;
	if CmdStartIsClear(){									//������ ��������� ��� �� ����
		if ((strlen((char*)val) == 3)){						//���������� ����� �������
			CmdStartSet();	
			ZeroCountBefor = 0;
			ZeroCountAfter = 0;
			for (i=0;i<TOOL_DESCR_MAX; i++){
				ToolsDescription[i][0] = 0;
			}
			Ret = RET_OK;
		}
		else{
			CreateError(err_cmd_unknown);
		}
	}
	else{
		CreateError(err_start_unexp);
	}
	return Ret;
}

//���������� � ����������� (METRIC,0000.00)
u08 Metric(u08 *val){
	u08 Ret = RET_ERR, Fl = 0;
	if (CmdStartIsSet() && CmdHeaderIsClear()){				//������ ��������� ���� � ��������� ��� ��������
		char *i = strchr((char*)val, ',');
		//���� �������
		if (i != NULL){
			while ((*i == '0') || (*i == '.')){
				if (*i == '.'){								//����������� ��������
					Fl++;
					if (Fl == 2){							//��� ��� ���� �����������, ������ � �������
						break;
					}
				}
				else{
					if (Fl){								//��� ���������� �����������
						ZeroCountAfter++;
					}
					else{
						ZeroCountBefor++;
					}
				}
			}
			if ((*i != 0)									//������������ ������
				|| (Fl == 2)								//������� �����������
				|| (ZeroCountBefor = 0)						//��� �� ������ �����
				|| (ZeroCountAfter>2)){						//������� ����� ������ ����� �������
					CreateError(err_metric);
			}
			else{
				Ret = RET_OK;
			}
		}
	}
	else{
		CreateError(err_metric_unexp);
	}
	return Ret;
}

//����� ������������� ���������
u08 EndHeader(u08 *val){
	u08 Ret = RET_ERR, i=0, ToolsSeting = 0;
	
	if (CmdStartIsSet() && CmdHeaderIsClear()){				//������ ��������� ���� � ��������� ��� ��������
		if (cmdByte(val, 1) == 0){							//������� ������� �� 1 �������
			//��������� ���������� ��������� ������������
			for (;i<TOOL_DESCR_MAX;i++){
				if (*ToolsDescription[i]){
					ToolsSeting++;
					break;
				}
			}
			if (ToolsSeting == 0){							//��� �� ������ �������� �����������
				CreateError(err_tool_undesc);
			}
			else{
				if (ZeroCountBefor == 0){					//�� ������� ��������
					CreateError(err_metric_undef);
				}
				else{
					CmdHeaderSet();							//��������� ��������
					Ret = RET_OK;
				}
			}
		}
		else{
			CreateError(err_cmd_unknown);
		}
	}
	else{
		CreateError(err_header_unexp);
	}
	return Ret;
}


//������� ���������� ��� T00 - ���������� ��� ����������� ��� "T01C0,1" ������� ����������
u08 ChangeAndStop(u08 *val){
	u08 Ret = RET_ERR;
	char *SecondPart;
	const char* ErrTxt = NULL;
	
	if CmdStartIsClear() ErrTxt = err_tool_unexp;					//�� ���� ������ ���������
	if ((ErrTxt == NULL) &&
		(!(IsDigit(cmdByte(val, 1)) && IsDigit(cmdByte(val, 2))))	//������ ���� ��� �����
		){
		ErrTxt = err_tool;
	}
	if ((ErrTxt == NULL) &&
		(!((cmdByte(val, 1) == '0') && ((cmdByte(val, 1) & 0xf)<TOOL_DESCR_MAX)))  //�� ������ TOOL_DESCR_MAX
		){
		ErrTxt = err_tool_more;
	}
	if (ErrTxt == NULL){
		//------ ��������� ��� �� ��������, ������ ���� ������� ���� TXXCXX.X
		u08 NumTool = (cmdByte(val, 2) & 0xf);
		if (CmdHeaderIsClear()){
			SecondPart = strchr((char*)val, (int)CMD_T_SUFFIX_TOOL);
			if (SecondPart == NULL){								//�� ������� ����� C
				ErrTxt = err_tool;
			}
			else{
				if (*ToolsDescription[NumTool] != 0){				//����� ����� ����������� ��� ������
					ErrTxt = err_tool_repeat;
				}
				else{
					strcpy(ToolsDescription[NumTool], SecondPart);	//�������� �������� ����������� � ������
				}
			}
		}
		//------ ���� ���� �������, ������ ���� ������� TXX ��� T00
		else{
			if (NumTool == 0){										//���������� ����������
				//�������
				//�� ��������� ������� ���������
			}
			else if (*ToolsDescription[NumTool] != 0){				//������� ����������
				//������� ����������
				//���������� ������
				//������� � ����� ����� �����������
				//�������� ������ ��������� � ����������
				//���������� ���������� �������
			}
			else{
				ErrTxt = err_tool_undesc;
			}
		}
	}
	if (ErrTxt == NULL)
		CreateError(ErrTxt);
	return Ret;
}


//����������� ���������� �� ��������� ����������. ���������� ������������� �� �������������� ���� ���������������� ����� �������� �������
u08 Position(u08 *val){
	u08 Ret = RET_ERR;
	const char* ErrTxt = NULL;
	char *NumericX = NULL, *NumericY = NULL;
	s16 XNeed = 0, YNeed = 0;

	if (CmdStartIsSet() && CmdHeaderIsSet()){						//������ ��������� ���� � ��������� ��������
		if (cmdByte(val, (ZeroCountBefor+ZeroCountAfter+1)) != CMD_X_SUFFIX_TOOL)		//����� ��������� X ������ ���� ������� ������� Y
			ErrTxt = err_cmd_unknown;
		if (ErrTxt == NULL){
			NumericX = malloc(ZeroCountBefor+ZeroCountAfter+2);		//2 - ��� ���� � �����
			NumericY = malloc(ZeroCountBefor+ZeroCountAfter+2);		//2 - ��� ���� � �����
			if ((NumericX != NULL) && (NumericY != NULL)){
				memcpy(NumericX, val+1, ZeroCountBefor+ZeroCountAfter+1);//�������� ����� �� X ������ �� ������ �����
				*(NumericX+ZeroCountBefor+ZeroCountAfter+2) = 0;			//����������� 0 � ������
				memcpy(NumericY, val+ZeroCountBefor+ZeroCountAfter+2, ZeroCountBefor+ZeroCountAfter+1);
				*(NumericY+ZeroCountBefor+ZeroCountAfter+2) = 0;			
				if ((StrDigitTest(NumericX) == RET_ERR) || (StrDigitTest(NumericY) == RET_ERR))
					ErrTxt = err_cmd_unknown;
				if (ErrTxt == NULL){
					XNeed = strToLong(NumericX);
					YNeed = strToLong(NumericY);
					
				}
			}
			else{
				ErrTxt = err_cmd_mem;
			}
		}
	}
	if (NumericX != NULL)
		free(NumericX);
	if (NumericY != NULL)
		free(NumericY);
	if (ErrTxt == NULL)
		CreateError(ErrTxt);
	return Ret;
}

//����� ���������
u08 EndProgeam(u08 *val){
	u08 Ret = RET_ERR;
	CmdStartClear();
	return Ret;
}

//�������� ����� ��������� ������� � ��������� ���������, ���� 0 �� ������ � ������� ��� ���������
u16 GetArgService(u08 *str){
	u16 Ret = RET_ERR;
	char Value[5];
	s32 Arg32s;
	
	if (CmdStartIsClear()){											//������� ������ �� �����������
		if ((strlen((char*)str) == 6)){								//���������� ����� �������
			memcpy(Value, str+2, 4);								//�������� ��������
			Value[4] = 0;
			Arg32s = strToLong(Value);
			if (Arg32s>0){
				Ret = (u16) Arg32s;
			}
		}
	}
	return Ret;
}
	
//�������� � ������ ��������� ������
//--- X
u08 ForwardX(u08 *val){
	u08 Ret = RET_ERR;
	u16 Power = GetArgService(val);
	enum StepSizeType sz = STEP_SIZE_FULL;
	
	if (Power != RET_ERR){
		if ((Power >= 1) && (Power <=4))
			sz = (enum StepSizeType) (Power-1);
#ifdef DEBUG
Encoder.X1 = 0;
Encoder.X2 = 0;
#endif
		EncoderNeed.X2 = ENCODER_OFF;							//��������� �������� ���������
		ProcessStart(AXIS_X);
#ifdef DEBUG
usart_putstr("X Go/n/r");
#endif
		DriveSteppingOn(AXIS_X, DIR_AXIS_TO_MAX, sz);
		Ret = RET_OK;
	}
	return Ret;
}
u08 RewindX(u08 *val){
	u08 Ret = RET_ERR;
	u16 Power = GetArgService(val);
	if (Power != RET_ERR){
		EncoderNeed.X2 = ENCODER_OFF;							//��������� �������� ���������
		ProcessStart(AXIS_X);
		DriveSteppingOn(AXIS_X, DIR_AXIS_TO_MIN, STEP_SIZE_FULL);
		Ret = RET_OK;
	}
	return Ret;
}
u16 XAxisNeed;

void XAxisNeedGo(void){
	XFinish = NULL;
	EncoderNeed.X2 = XAxisNeed;									//������������ � ������ ����������
}

#define X_SHIFT_ADDED			50								//������ ����� ���������� ������ ������ ������������� ������� ���������� � �������, � ����� ������������ �� ������ ����������

u08 ShiftSetX(u08 *val){
	u08 Ret = RET_ERR;
	u16 Shift = GetArgService(val), Ab;
	
	if ((Shift != RET_ERR) && (Shift != Encoder.X2)){
		ProcessStart(AXIS_X);									//������ ���
		if (EncoderNeed.X2>Encoder.X2)
			Ab = EncoderNeed.X2-Encoder.X2;
		else
			Ab = Encoder.X2-EncoderNeed.X2;
		if (Ab<X_SHIFT_ADDED){									//��������� ������� ������ ���� � �����, ���������� ��������, � ����� ������
			XAxisNeed = Shift;
			XFinish = XAxisNeedGo;
			if (Encoder.X2+(X_SHIFT_ADDED*3)>X_AXIS_MAX)
				EncoderNeed.X2 = Encoder.X2-(X_SHIFT_ADDED*3);
			else
				EncoderNeed.X2 = Encoder.X2+(X_SHIFT_ADDED*3);
		}
		else
			EncoderNeed.X2 = Shift;
		Ret = RET_OK;
	}
	return Ret;
}
//--- Y
u08 ForwardY(u08 *val){
	u08 Ret = RET_ERR;
	u16 Power = GetArgService(val);
	if (Power != RET_ERR){
		DriveSteppingOn(AXIS_Y, DIR_AXIS_TO_MAX, STEP_SIZE_FULL);
		Ret = RET_OK;
	}
	return Ret;
}
u08 RewindY(u08 *val){
	u08 Ret = RET_ERR;
	u16 Power = GetArgService(val);
	if (Power != RET_ERR){
		DriveSteppingOn(AXIS_Y, DIR_AXIS_TO_MIN, STEP_SIZE_FULL);
		Ret = RET_OK;
	}
	return Ret;
}
//--- Z
u08 DownZ(u08 *val){
	u08 Ret = RET_ERR;
	u16 Power = GetArgService(val);
	if (Power != RET_ERR){
		DriveDCOn(AXIS_Z, DIR_Z_DOWN, Power);
		Ret = RET_OK;
	}
	return Ret;
}
u08 UpZ(u08 *val){
	u08 Ret = RET_ERR;
	u16 Power = GetArgService(val);
	if (Power != RET_ERR){
		DriveDCOn(AXIS_Z, DIR_Z_UP, Power);
		Ret = RET_OK;
	}
	return Ret;
}
u08 ShiftSetZ(u08 *val){
	u08 Ret = RET_ERR;
	u16 Shift = GetArgService(val);
	if (Shift != RET_ERR){
		if (Shift<=Z_AXIS_MAX){
			EncoderNeed.Z = Shift;
			if (Encoder.Z > EncoderNeed.Z){			//����� �������
				DriveDCOn(AXIS_Z, DIR_Z_UP, 720);
			}
			else if(Encoder.Z == EncoderNeed.Z){	//��� � ������ �������, ������ ���������� ����� �� ������ ������
				ZAxisOff();
			}
			else{									//����� ��������
				DriveDCOn(AXIS_Z, DIR_Z_DOWN, 650);
			}
			Ret = RET_OK;
		}
	}
	return Ret;
}

u08 GetEncoders(u08 *val){
	u08 Ret = RET_ERR;
	u16 Shift = GetArgService(val);
	if (Shift != RET_ERR){
#ifdef DEBUG
usart_putstr("Enc curr X1 X2=");
usart_hex((u08)((u16)(Encoder.X1)>>8));
usart_hex((u08)(Encoder.X1));
usart_putchar(' ');
usart_hex((u08)((u16)(Encoder.X2)>>8));
usart_hex((u08)(Encoder.X2));
usart_putchar(0xd);
usart_putchar(0xa);
usart_putstr("Y1 Y2=");
usart_hex((u08)((u16)(Encoder.Y1)>>8));
usart_hex((u08)(Encoder.Y1));
usart_putchar(' ');
usart_hex((u08)((u16)(Encoder.Y2)>>8));
usart_hex((u08)(Encoder.Y2));
usart_putchar(0xd);
usart_putchar(0xa);
usart_putstr("Z=");
usart_hex((u08)((u16)(Encoder.Z)>>8));
usart_hex((u08)(Encoder.Z));
usart_putchar(0xd);
usart_putchar(0xa);
#endif
		Ret = RET_OK;
	}
	return Ret;
}
//--- DRILL
u08 ForwardDrill(u08 *val){
	u08 Ret = RET_ERR;
	u16 Power = GetArgService(val);
	if (Power != RET_ERR){
		DriveDCOn(AXIS_DRILL, DIR_DRILL_CLOCKWISE, Power);
		Ret = RET_OK;
	}
	return Ret;
}
u08 RewindDrill(u08 *val){
	u08 Ret = RET_ERR;
	u16 Power = GetArgService(val);
	if (Power != RET_ERR){
		DriveDCOn(AXIS_DRILL, DIR_DRILL_ANTICLOCKWISE, Power);
		Ret = RET_OK;
	}
	return Ret;
}
u08 StopDrill(u08 *val){
	u08 Ret = RET_ERR;
	u16 Power = GetArgService(val);
	if (Power != RET_ERR){
		DrillOff();
		Ret = RET_OK;
	}
	return Ret;
}

u08 EncoderReset(u08 *val){
	u08 Ret = RET_ERR;
	u16 Power = GetArgService(val);
	if (Power != RET_ERR){
		Encoder.X1 = 0;
		Encoder.X2 = 0;
		Encoder.Y1 = 0;
		Encoder.Y2 = 0;
		Encoder.Z = 0;
		Ret = RET_OK;
	}
	return Ret;
}

//������ ������ ������ uart � ���������� �������
void ParseCmd(void){
	u08 *cmd, i=0, flag = 0;

	SetTimerTask(ParseCmd, 100);
	
	if (ProcessIsFree() && (!FIFO_IS_EMPTY(CmdBuf))){					//������ ��������, ���� ������� � ������� ���� � �������
		cmd = FIFO_FRONT(CmdBuf);
		FIFO_POP(CmdBuf);
		for (;i<CMD_NUM_MAX;i++){
			if (strncmp_P((char*)cmd, ListCmd[i].Code, strlen_P(ListCmd[i].Code))==0){
				if (((VOID_CMD)&CmdExec(i))(cmd) == RET_OK){
					flag = 1;
					break;
				}
			}
		}
		if (flag == 0){
			CmdError();
		}
		free((void*)cmd);												//���������� ������ �� ����������� �������
	}
}

#define PID_X_PERIOD		25											//������ �������������, ������ �� ����������� ��������� ������ ������� ����� ���������� ������ - 15-18 �� ��� ������������ ���� 
#define REST_COUNT			4											//���� � �������� ����� ���������� �������� �������� ��� �� ����, �� ���������, ��� ���� ���������
void AxisXPID(void){
	enum DirType Dir = DIR_AXIS_TO_MAX;
	static u08 CountRestCycle = REST_COUNT;

	if BitIsClear(CommonFlags, KEY_STOP_DOWN){							//���� ������ ������ ����, ������ �� ��������
		if (EncoderNeed.X2 != ENCODER_OFF){								//���� ������������� � ����������� �� ��� X
			if (EncoderNeed.X2 < Encoder.X2){
				Dir = DIR_AXIS_TO_MIN;
//				if ((Encoder.X2 - EncoderNeed.X2)<30)
//					Power = 600;
			}
			else{
//				if ((EncoderNeed.X2-Encoder.X2)<30)
//					Power = 600;
			}
			if (EncoderNeed.X2 != Encoder.X2){
				CountRestCycle = REST_COUNT;
				DriveSteppingOn(AXIS_X, Dir, STEP_SIZE_FULL);
			}
			else{
				CountRestCycle--;
				if ((CountRestCycle == 0) && ProcessAxisIsGo(AXIS_X)){	//�� ���� �������� �� ��� X �������� ���������� ��������, ������� ������� �����������
					ProcessEnd(AXIS_X);		
					if (XFinish != NULL){
						XFinish();
					}
				}
			}
		}
	}
	SetTimerTask(AxisXPID, PID_X_PERIOD);
}

int main(void)
{
	FIFO_FLUSH(CmdBuf);
	SerilalIni();
	
	//����� RTOS
	InitRTOS();
	RunRTOS();

	InitHAL();

	usart_putstr("R\r\n");

	ParseCmd();
	KeyLedFlashOn();										//������ � ���� ������ �������������

	//��������� 0
	//------ ��� X
	if (SensorIsAlarmDirect(X_MIN_SENS_IN, X_MIN_SENS_PIN)){//������� ����� � ����������� ���������
usart_putchar('M');
		DriveSteppingOn(AXIS_X, DIR_AXIS_TO_MAX, STEP_SIZE_FULL);
usart_putchar('F');
		EncoderNeed.X2 = 200;
		while(Encoder.X2 != EncoderNeed.X2)
			TaskManager();
	}
usart_putchar('G');
	DriveSteppingOn(AXIS_X, DIR_AXIS_TO_MIN, STEP_SIZE_FULL);	//������� �� ������������ ������� - ��� ����� ����.
	Encoder.X2 = 0;
	while(1){
		TaskManager();
	}
}