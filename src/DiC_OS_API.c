#include "DiC_OS_API.h"


void os_Delay(uint32_t ticks)  {
	taskStructure_t* currentTask;

	/*
	 * La estructura control_OS solo puede ser accedida desde el archivo core, por lo que
	 * se provee una funcion para obtener la tarea actual (equivale a acceder a
	 * control_OS.tarea_actual)
	 */
	currentTask = (taskStructure_t *)os_getCurrentTask();

	/*
	 * Se carga la cantidad de ticks a la tarea actual si la misma esta en running
	 * y si los ticks son mayores a cero
	 */
	if (currentTask->taskStatusRRB == TASK_RUNNING && ticks > 0)  {
		currentTask->ticks_blocked = ticks;

		/*
		 * El proximo bloque while tiene la finalidad de asegurarse que la tarea solo se desbloquee
		 * en el momento que termine la cuenta de ticks. Si por alguna razon la tarea se vuelve a
		 * ejecutar antes que termine el periodo de bloqueado, queda atrapada.
		 *
		 */

		while (currentTask->ticks_blocked > 0)  {
			currentTask->taskStatusRRB = TASK_BLOCKED;
			CpuYield();
		}
	}
}