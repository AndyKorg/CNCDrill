/*
 * ������ �������������� ������ � �������
 */ 


#ifndef COMANDLIST_H_
#define COMANDLIST_H_

//-------------------- ������ ������ � ���������� ��� ������ �� UART ---------------------------------------------------

//-------------- "N/C Drill"
#define CMD_NUM_MAX				19						//���������� ������

#define CMD_NC_HEAD				"M48"					//������ ���������
#define CMD_NC_METRIC			"METRIC"				//���������� � �����������
#define CMD_NC_CHANGE_TOOL		"T"						//������� ���������� ��� T0 - ���������� ��� �����������
#define CMD_NC_POS				"X"						//����������� ����������� �� ��������� ����������. ���������� ������������� �� �������������� ����
#define CMD_NC_END				"M30"					//����� ���������
#define CMD_NC_END_HEADER		"%"						//����� ������������� ���������
#define CMD_T_SUFFIX_TOOL		'C'						//������� ������� "T", �� ��������� �������
#define CMD_X_SUFFIX_TOOL		'Y'						//������� ������� "X", �� ��������� �������

//-------------- ��������� ������� - �� ������ � N/C Drill
//������ ������� CMD_FORWARD � CMD_REWIND: ����� ������� ���� ����� �� 0 �� 3, ������������ ���. ���� ���� � AxisType
//����� ����� ������ ����� ��� �������� ��� ��� ���� 0001, 0002, 0003, 0004 ��� ������� ����������. ���� ��������� ������� ������ ���� ������ �� ���� ����!
#define CMD_FORWARD_X			"FX"					//����� X ������
#define CMD_REWIND_X			"RX"					//����� X ������� �����
#define CMD_SHIFT_X				"SX"					//�������� � �� ��������� ����� �������
#define CMD_FORWARD_Y			"FY"					//Y
#define CMD_REWIND_Y			"RY"
#define CMD_DOWN_Z				"DZ"					//Z
#define CMD_UP_Z				"UZ"
#define CMD_SHIFT_Z				"SZ"					//������� �� ������� �������� �������� ������������ �����
#define CMD_FORWARD_DRILL		"FD"					//Drill
#define CMD_REWIND_DRILL		"RD"
#define CMD_STOP_DRILL			"SD"					//���������� ��������
#define CMD_ENCODE_RESET		"ER"					//�������� �������� ���������
#define CMD_GET_ENCODER			"GE"					//�������� ������� �������� ���������

#define CMD_STOP				'S'						//���� ��������, ����������� �������

#define CMD_ANSWER_OK			"OK"					//�������� ���������, ���� ��������� �������
#define CMD_ANSWER_ERR			"ERROR"					//������
#define CMD_ANSWER_ALARM		"ALARM"					//������

#endif /* COMANDLIST_H_ */