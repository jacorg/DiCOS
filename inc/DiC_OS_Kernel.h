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


/*==========Definicion de prioridad, cuatro niveles. El maximo es 0 y el minimo es 3=====*/
#define ID_TASK_IDLE 0xFF
#define ERROR_PRIOR 0xFF
/*==================[definicion codigos de error de OS]=================================*/
#define ERR_OS_CANT_TAREAS		-1
#define CANT_PRIOR               4 //Cantidad de prioridades del OS, se utiliza en la estructura
								   // del OS para definir el diagrama de scheduling

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
	uint8_t id;                         //id de la tarea
	taskStatus_t taskStatusRRB;         //estado puede ser RUN, READY o BLOCK
	uint8_t priority;                   // La implementación de prioridades será de 4 niveles 0:max 3:mín
	uint32_t ticks_blocked;       
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
/*
Enumeración para las prioridades del sistema. Se usa en la implementación del scheduling
*/

enum priorOS  {
	PRIOR_0,
	PRIOR_1,
	PRIOR_2,
	PRIOR_3
};
typedef enum priorOS priorOS_t;

/********************************************************************************
 * Definicion de la estructura de control para el sistema operativo
 *******************************************************************************/
struct controlOS  {
	void *listaTareas[MAX_TASK_COUNT];		//array de punteros a tareas
	int32_t error;							//variable que contiene el ultimo error generado
	uint8_t cantidad_Tareas;				//cantidad de tareas definidas por el usuario para cada prioridad
	statusOS_t statusOS;					//Informacion sobre el estado del OS

/*=Sección destinada al scheduling x prioridades con RUNNING READY BLOCKED y 4 prioridades*/

	void *priorScheme[CANT_PRIOR][MAX_TASK_COUNT];   //Acá en os_Init cargo todas las tareas
	                                                 // creadas con su prioridad por filas
													 // y el id lo uso para determinar en que lugar
													 // de la columna se encuentran
													 //Los datos los tomo de la listaTareas
	
	uint8_t taskxPrior[CANT_PRIOR];   		//En este array mantengo la cant de tareas activas por prioridad               
	bool changeContext;                     //false or true: indico si es necesario cambiar el contexto 
											//puede suceder que haya solo una tarea en una 
											//determinada prioridad corriendo y no sea necesario cambiarla.

/*========================================================================================*/
	taskStructure_t *currentTask;            //definicion de puntero para tarea actual
	taskStructure_t *nextTask;		         //definicion de puntero para tarea siguiente
};
typedef struct controlOS controlOS_t;


/*==================[definicion de prototipos]=================================*/

//void os_InitTarea(void *entryPoint, taskStructure_t *task);
void os_Init(void);
void createTask(void *entryPoint, taskStructure_t *task, uint8_t priority);
taskStatus_t* os_getCurrentTask(void);
void CpuYield(void);
uint8_t totalTasksBlocked(void);
uint8_t getFirstTask(void);       //Toma la primera tarea disponible de mayor prioridad para 
								  //comenzar el scheduling.
void checkTasksTiks(void);        //verifico que los tick_blocked sean mayor que cero y luego decremento
								  //en el scheduler antes de hacer cualquier cosa verifico 
								  //que si hay tareas con tick =0 luego pasen a ready.

//void nTaskBlocked(uint8_t *nBlocked);
void initIndexTasks(uint8_t *idx_tasks); // Permite la inicialiazon de los indices que recorren las tareas por prioridad.

/*Permite seleccionar tareas disponibles dentro de una determinada prioridad*/
void selectTasks(priorOS_t prior, uint8_t *idx_tasks, uint8_t *array_nTaskBlocked);


/* Definicion de funcion static*/
//static void taskIdle(void);

#endif /* ISO_I_2020_MSE_OS_INC_MSE_OS_CORE_H_ */
