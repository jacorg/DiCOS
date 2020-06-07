/*
 * DiC_OS_Kernel.h
 *
 *  Created on: 26 mar. 2020
 *      Author: gonza
 */

#ifndef ISO_I_2020_MSE_OS_INC_MSE_OS_CORE_H_
#define ISO_I_2020_MSE_OS_INC_MSE_OS_CORE_H_

#include <stdint.h>
#include <stdbool.h>
#include "board.h"


/************************************************************************************
 * 			Tamaño del stack predefinido para cada tarea expresado en bytes
 ***********************************************************************************/

#define STACK_SIZE 256

//----------------------------------------------------------------------------------



/************************************************************************************
 * 	Posiciones dentro del stack de los registros que lo conforman
 ***********************************************************************************/

#define XPSR			1
#define PC_REG			2
#define LR				3
#define R12				4
#define R3				5
#define R2				6
#define R1				7
#define R0				8
#define LR_PREV_VALUE	9
#define R4				10
#define R5				11
#define R6				12
#define R7				13
#define R8				14
#define R9				15
#define R10 			16
#define R11 			17

//----------------------------------------------------------------------------------


/************************************************************************************
 * 			Valores necesarios para registros del stack frame inicial
 ***********************************************************************************/

#define INIT_XPSR 	1 << 24				//xPSR.T = 1
#define EXEC_RETURN	0xFFFFFFF9			//retornar a modo thread con MSP, FPU no utilizada

//----------------------------------------------------------------------------------


/************************************************************************************
 * 						Definiciones varias
 ***********************************************************************************/
#define STACK_FRAME_SIZE			8
#define FULL_STACKING_SIZE 			17	//16 core registers + valor previo de LR

#define TASK_NAME_SIZE				10	//tamaño array correspondiente al nombre
#define MAX_TASK_COUNT				8	//cantidad maxima de tareas para este OS



/*==================[definicion codigos de error de OS]=================================*/
#define ERR_OS_CANT_TAREAS		-1



/*==================[definicion de datos para el OS]=================================*/


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*       Definición de enumeraciones y estructuras para el sistema de tareas
*********************************************************************************
 *               Definicion de los estados posibles para las tareas
********************************************************************************/

enum taskStatus  {
	TASK_READY,
	TASK_RUNNING,
	TASK_BLOCKED
};
typedef enum taskStatus taskStatus_t;

/********************************************************************************
 *               Definicion de la estructura mínima para cada tarea
 *******************************************************************************/
struct taskStructure  {
	uint32_t stack[STACK_SIZE/4];
	uint32_t stack_pointer;
	void *entry_point;
	uint8_t id;                   //id de la tarea
	taskStatus_t taskStatusRRB;         //estado puede ser RUN, READY o BLOCK
	uint8_t priority;            // La implementación de prioridades será de 4 niveles 0:max 3:mín
	uint32_t tick_blocked;       
};
typedef struct taskStructure taskStructure_t;



/*ESTRCUTURA DEL OS


/********************************************************************************
 * Definicion de los estados posibles de nuestro OS
 *******************************************************************************/

enum statusOS  {
	OS_NORMAL_RUN,
	OS_FROM_RESET
};
typedef enum statusOS statusOS_t;

/********************************************************************************
 * Definicion de la estructura de control para el sistema operativo
 *******************************************************************************/
struct controlOS  {
	void *listaTareas[MAX_TASK_COUNT];		//array de punteros a tareas
	int32_t error;							//variable que contiene el ultimo error generado
	uint8_t cantidad_Tareas;				//cantidad de tareas definidas por el usuario para cada prioridad
	statusOS_t statusOS;					//Informacion sobre el estado del OS

	taskStructure_t *currentTask;			//definicion de puntero para tarea actual
	taskStructure_t *nextTask;		//definicion de puntero para tarea siguiente
};
typedef struct controlOS controlOS_t;


/*==================[definicion de prototipos]=================================*/

//void os_InitTarea(void *entryPoint, taskStructure_t *task);
void os_Init(void);
void createTask(void *entryPoint, taskStructure_t *task, uint8_t priority);

/* Definicion de funcion static*/
//static void taskIdle(void);

#endif /* ISO_I_2020_MSE_OS_INC_MSE_OS_CORE_H_ */
