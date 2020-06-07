
/* DiCOS: Es un sistema operativo pra el procesamiento y control digital
+de un sistema de sensado remoto LiDAR.
Se toma como guía  el OS desarrollado por Mg. Gonzales Sanchez. 
*/

/*==================[inclusions]=============================================*/
#include "main.h"
#include "board.h" 
#include "DiC_OS_Kernel.h" //Definicion y variables usadas por el OS
#include "DiC_OS_Tasks.h"  //Definición de las tareas del sistema operativo
/*==================[macros and definitions]=================================*/

#define MILISEC		1000

/*==================[Global data declaration]==============================*/

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

int main(void)  {

	initHardware();

    //Instancio la tarea con su correspondiente prioridad

	createTask(Task_0, &taskStructure_0, PRIORITY_0);
	createTask(Task_1, &taskStructure_1, PRIORITY_0);
	createTask(Task_2, &taskStructure_2, PRIORITY_1);
	createTask(Task_3, &taskStructure_3, PRIORITY_1);
	createTask(Task_4, &taskStructure_4, PRIORITY_2);
	createTask(Task_5, &taskStructure_5, PRIORITY_2);
	createTask(Task_6, &taskStructure_6, PRIORITY_3);
	createTask(Task_7, &taskStructure_7, PRIORITY_3);

	os_Init();

	while (1) {
	}
}

/*==================[end of file]============================================*/
