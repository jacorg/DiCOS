#ifndef DiC_OS_Tasks
#define DiC_OS_Tasks

#include <stdint.h>
#include <stdbool.h>
#include "board.h"
#include "DiC_OS_API.h"
#include "sapi.h"
#include "DiC_OS_Interrupts.h"

/*==========Definicion de prioridad, cuatro niveles. El maximo es 0 y el minimo es 3=====*/
#define PRIORITY_0 0
#define PRIORITY_1 1
#define PRIORITY_2 2
#define PRIORITY_3 3
/*---------------------Definiciones para menejo de interrupciones con teclas-------------*/
#define TEC1_i 1
#define TEC2_i 2

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
/*-------------------------------Variable globales semaforos-----------------------------*/
/*
Genero dos semaforos para controlar la interrupcion del teclado. La tecla al ser 
presionada debera soltarse en un tiempo dado. El semaforo controla cuando la tecla es
presionada y cuando es soltada.
*/
semDiCOS_t semTEC1, semTEC2, endTEC1, endTEC2;

/*-----------------------------Variables globales tarea 1 y 2----------------------------*/
    

uint32_t ticksFallingTEC1,ticksRisingTEC1;  //No es necesario resetear estos valores
                                            //a cero dentro de la while true 
                                            //porque la medición de tiempo es relativa a otra tecla.



uint32_t ticksFallingTEC2,ticksRisingTEC2;  //No es necesario resetear estos valores
                                            //a cero dentro de la while true 
                                            //porque la medición de tiempo es relativa a otra tecla.



char msg1[20];
char charTon[20];
/*--------------------------------------Sección colas------------------------------------*/
/*Declaración dato para cola entre tarea 1 y 2*/
//dataTypeQueue_t  DatosQueue,DatosQueRx;
//queue_t          Tarea1;                   //Cola para la tarea 1



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
void TEC1_Handler(void);

void TEC2_Handler(void);

void printMessages(colorLEDs_t colorLED,int32_t * t1, int32_t *t2); //Permite la impresión de mensajes en la UART en 
                                                                    //función del color de LED.
void sendDataTimeUART(int32_t * t1, int32_t *t2);

char* itoa(int value, char* result, int base); //Conversion de datos númericos int en strings

#endif