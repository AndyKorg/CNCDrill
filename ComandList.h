/*
 * Список поддерживаемых команд и ответов
 */ 


#ifndef COMANDLIST_H_
#define COMANDLIST_H_

//-------------------- Список команд и переменных для приема по UART ---------------------------------------------------

//-------------- "N/C Drill"
#define CMD_NUM_MAX				19						//Количество команд

#define CMD_NC_HEAD				"M48"					//Начало программы
#define CMD_NC_METRIC			"METRIC"				//Рамерность в милимметрах
#define CMD_NC_CHANGE_TOOL		"T"						//Сменить инструмент или T0 - остановить все инструменты
#define CMD_NC_POS				"X"						//Переместить инструмента на указанную координату. Координата отсчитывается от относительного ноля
#define CMD_NC_END				"M30"					//Конец программы
#define CMD_NC_END_HEADER		"%"						//Конец описательного заголовка
#define CMD_T_SUFFIX_TOOL		'C'						//Суффикс команды "T", не отдельная команда
#define CMD_X_SUFFIX_TOOL		'Y'						//Суффикс команды "X", не отдельная команда

//-------------- Сервисные команды - не входят в N/C Drill
//формат команды CMD_FORWARD и CMD_REWIND: после команды идет цифра от 0 до 3, обозначающая ось. Коды осей в AxisType
//После цифры четрые цифпы для делителя ШИМ или коды 0001, 0002, 0003, 0004 для шаговых двигателей. Коды сервисных команды должны быть только из двух букв!
#define CMD_FORWARD_X			"FX"					//Мотор X вперед
#define CMD_REWIND_X			"RX"					//Мотор X крутить назад
#define CMD_SHIFT_X				"SX"					//Сдвинуть х на указанное число позиций
#define CMD_FORWARD_Y			"FY"					//Y
#define CMD_REWIND_Y			"RY"
#define CMD_DOWN_Z				"DZ"					//Z
#define CMD_UP_Z				"UZ"
#define CMD_SHIFT_Z				"SZ"					//Указать на сколько сдвинуть шпиндель относительно верха
#define CMD_FORWARD_DRILL		"FD"					//Drill
#define CMD_REWIND_DRILL		"RD"
#define CMD_STOP_DRILL			"SD"					//Остановить вращение
#define CMD_ENCODE_RESET		"ER"					//Сбросить счетчики энкодеров
#define CMD_GET_ENCODER			"GE"					//Получить текущее значение энкодеров

#define CMD_STOP				'S'						//Стоп процесса, специальная команда

#define CMD_ANSWER_OK			"OK"					//Операция закончена, ждем следующую команду
#define CMD_ANSWER_ERR			"ERROR"					//Ошибка
#define CMD_ANSWER_ALARM		"ALARM"					//Авария

#endif /* COMANDLIST_H_ */