
/* DiCOS: Es un sistema operativo pra el procesamiento y control digital
+de un sistema de sensado remoto LiDAR.
Se toma como guía  el OS desarrollado por Mg. Gonzales Sanchez. 
*/

/*==================[inclusions]=============================================*/
#include "main.h"
#include "board.h" 
#include "sapi.h"
#include "DiC_OS_Kernel.h" //Definicion y variables usadas por el OS
#include "DiC_OS_Tasks.h"  //Definición de las tareas del sistema operativo
#include "DiC_OS_Interrupts.h" // Definición de interrupciones
/*==================[macros and definitions]=================================*/

#define MILISEC		1000

/*==================[Global data declaration]==============================*/
/*Variable globales para tareas*/
taskStructure_t taskStructure_0;
taskStructure_t taskStructure_1;
taskStructure_t taskStructure_2;
taskStructure_t taskStructure_3;
taskStructure_t taskStructure_4;
taskStructure_t taskStructure_5;
taskStructure_t taskStructure_6;
taskStructure_t taskStructure_7;

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/** @brief hardware initialization function
 *	@return none
 */
static void initHardware(void)  {
	Board_Init();
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / MILISEC);		//systick 1ms
}

/*==================[Definicion de tareas para el OS]==========================*/




/*============================================================================*/
DEBUG_PRINT_ENABLE
CONSOLE_PRINT_ENABLE
int main(void)  {

	initHardware();
	debugPrintConfigUart ( UART_USB, 115200 );

    //Instancio la tarea con su correspondiente prioridad
	/*La tarea 1 se encarga de manejar la tecla TEC1 por medio de la interrupción*/
	createTask(Task_0, &taskStructure_0, PRIORITY_0);
	/*La tarea 2 se encarga de manejar la tecla TEC2 por medio de la interrupción*/
	createTask(Task_1, &taskStructure_1, PRIORITY_0);
	/*Clasifica los tiempos t1 y t2 y calcula tiempo de encendido y delay en leds*/
	createTask(Task_2, &taskStructure_2, PRIORITY_0);
	/*Impresión de mensajes por medio de UART*/
	createTask(Task_3, &taskStructure_3, PRIORITY_1);


	semInit(&semTEC1);//Los uso para menejo de la interrupción de teclado y 
					  //determinar presionado o soltado de TEC1
	
	semInit(&semTEC2);//Los uso para menejo de la interrupción de teclado y 
					  //determinar presionado o soltado de TEC2
	
	semInit(&endTEC1);
	semInit(&endTEC2);

/*Despues de haber creado todas las tareas con asignación de sus prioridades y configurado las
variables del OS corro la función initHardware. Si el sistema tiene menos de 8 tareas entonces la 
lista de tareas del OS se asignan con NULL
*/
	os_Init();

	while (1) {
	}
}

/*==================[end of file]============================================*/
