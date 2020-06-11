#ifndef DiC_OS_Tasks
#define DiC_OS_Tasks

#include <stdint.h>
#include <stdbool.h>
#include "board.h"

/*==========Definicion de prioridad, cuatro niveles. El maximo es 0 y el minimo es 3=====*/
#define PRIORITY_0 0
#define PRIORITY_1 1
#define PRIORITY_2 2
#define PRIORITY_3 3

//Prototipo de las tareas
void Task_0(void);
void Task_1(void);
void Task_2(void);
void Task_3(void);
void Task_4(void);
void Task_5(void);
void Task_6(void);
void Task_7(void);

#endif