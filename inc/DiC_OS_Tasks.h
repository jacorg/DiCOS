#ifndef DiC_OS_Tasks
#define DiC_OS_Tasks

#include <stdint.h>
#include <stdbool.h>
#include "board.h"
//#include "DiC_OS_API.h"
#include "sapi.h"
#include "DiC_OS_Interrupts.h"
#include "DiC_OS_keys.h"

/*==========Definicion de prioridad, cuatro niveles. El maximo es 0 y el minimo es 3=====*/
#define PRIORITY_0 0
#define PRIORITY_1 1
#define PRIORITY_2 2
#define PRIORITY_3 3

/*-----------------Enumeraciones máquina de estados para pulsado, soltado----------------*/

enum statusTEC{
    FALLING,
    RISING
};
typedef enum statusTEC statusTEC_t;


enum colorLEDs{
    VERDE,
    ROJO,
    AMARILLO,
    AZUL
};
typedef enum colorLEDs colorLEDs_t;

statusTEC_t statusTEC1,statusTEC2;
colorLEDs_t colorLED;

/*-----------------------------Variables globales tarea 1 y 2----------------------------*/



char msg1[20];
char charTon[20];
/*--------------------------------------Sección colas------------------------------------*/
/*Declaración dato para cola entre tarea 1, 2 y 3*/

queue_t msgTimeTECs;
dataTypeQueue_t timeEventTx1,timeEventTx2,timeEventRx1,timeEventRx2;
/*----------------------------------Prototipo de las tareas-------------------------------*/
void Task_0(void);
void Task_1(void);
void Task_2(void);
/*
void Task_3(void);
void Task_4(void);
void Task_5(void);
void Task_6(void);
void Task_7(void);
*/

/*------------------Prototipo handler de interrupcion para teclas 1 y 2------------------*/


void printMessages(colorLEDs_t colorLED,int32_t * t1, int32_t *t2); //Permite la impresión de mensajes en la UART en 
                                                                    //función del color de LED.
void sendDataTimeUART(int32_t * t1, int32_t *t2);

char* itoa(int value, char* result, int base); //Conversion de datos númericos int en strings

void nothingFunc(void);
#endif