/*
 * DiC_OS_Kernel.c
 *
 *  Author: Jacobo Salvador
 *  Se toma como guía  el OS desarrollado por Mg. Gonzales Sanchez. 
 */

/*------------------------------Funciones en DiC_OS_Kernel------------------------------

|------>void createTask(void *entryPoint, taskStructure_t *task,uint8_t priority)
					
|------>void os_Init(void) 
				|------>static void initTaskIdle(void)

|------>void SysTick_Handler(void)
                    |------>static void scheduler(void)

|------>PendSV_Handler
				|------>uint32_t getNextContext(uint32_t sp_actual)
*/

#include "DiC_OS_Kernel.h"

/*==================[definicion de variables globales]=================================*/

static controlOS_t controlOS;
static taskStructure_t taskIdleStructure;

void __attribute__((weak)) taskIdle(void)  {
	while(1)  {
		__WFI();
	}
}

void __attribute__((weak)) returnHook(void)  {
	while(1);
}
/*==================[definicion de prototipos static]=================================*/
/*Inicialización de la tareas TaskIdle esta es una tarea que el OS deberia ejecutar 
en el caso de que no haya ninguna tareas ejecutandose o en su defecto si todas las tareas 
pasan a BLOCKED
*/

static void initTaskIdle(void); 



/*************************************************************************************************
/*createTask: Instancia la tareas en el OS a partir de un entry point que es el puntero a la función
una estructura taskStructure que contiene el contexto de la tarea y también se configura
la prioridad de la tarea, va de 0: máx prioridad a 3 mín priorida.
***************************************************************************************************/
void createTask(void *entryPoint, taskStructure_t *task,uint8_t priority)  {
	static uint8_t id = 0;				//el id sera correlativo a medida que se generen mas tareas

	/*
	 * Al principio se efectua un pequeño checkeo para determinar si llegamos a la cantidad maxima de
	 * tareas que pueden definirse para este OS. En el caso de que se traten de inicializar mas tareas
	 * que el numero maximo soportado, se guarda un codigo de error en la estructura de control del OS
	 * y la tarea no se inicializa.
	 */

	if(controlOS.cantidad_Tareas < MAX_TASK_COUNT || entryPoint == taskIdle)  {

		task->stack[STACK_SIZE/4 - XPSR] = INIT_XPSR;					//necesario para bit thumb
		task->stack[STACK_SIZE/4 - PC_REG] = (uint32_t)entryPoint;		//direccion de la tarea (ENTRY_POINT)


		/*
		 * El valor previo de LR (que es EXEC_RETURN en este caso) es necesario dado que
		 * en esta implementacion, se llama a una funcion desde dentro del handler de PendSV
		 * con lo que el valor de LR se modifica por la direccion de retorno para cuando
		 * se termina de ejecutar getContextoSiguiente
		 */
		task->stack[STACK_SIZE/4 - LR_PREV_VALUE] = EXEC_RETURN;

		task->stack_pointer = (uint32_t) (task->stack + STACK_SIZE/4 - FULL_STACKING_SIZE);

		/*
		 Todas las tareas al ingresan se encuentran en estado TASK_READY.
		 */
		task->entry_point = entryPoint;
		task->id = id;
		task->taskStatusRRB = TASK_READY;

		/*
		 * Actualizacion de la estructura de control del OS, guardando el puntero a la estructura de tarea
		 * que se acaba de inicializar, y se actualiza la cantidad de tareas definidas en el sistema.
		 * Luego se incrementa el contador de id, dado que se le otorga un id correlativo a cada tarea
		 * inicializada, segun el orden en que se inicializan.
		 */
		controlOS.listaTareas[id] = task;
		controlOS.cantidad_Tareas++;

		id++;
	}

	else {
		controlOS.error = ERR_OS_CANT_TAREAS;		//excedimos la cantidad de tareas posibles
	}
}


/*
Seteo el OS inicialmente, ajusto la prioridad del NVIC para que PendSV tenga menos prrioridad
que el systick Handler. Si el OS recien se inicia se detecta que viene de un reset, CurrentTask y nextTask = NULL
*/
void os_Init(void)  {
	/*
	 * Todas las interrupciones tienen prioridad 0 (la maxima) al iniciar la ejecucion. Para que
	 * no se de la condicion de fault mencionada en la teoria, debemos bajar su prioridad en el
	 * NVIC. La cuenta matematica que se observa da la probabilidad mas baja posible.
	 */
	NVIC_SetPriority(PendSV_IRQn, (1 << __NVIC_PRIO_BITS)-1);

	initTaskIdle();

	/*
	 * Al iniciar el OS se especifica que se encuentra en la primer ejecucion desde un reset.
	 * Este estado es util para cuando se debe ejecutar el primer cambio de contexto. Los
	 * punteros de tarea_actual y tarea_siguiente solo pueden ser determinados por el scheduler
	 */
	controlOS.statusOS = OS_FROM_RESET;
	controlOS.currentTask = NULL;
	controlOS.nextTask = NULL;


	/*
	 * El vector de tareas termina de inicializarse asignando NULL a las posiciones que estan
	 * luego de la ultima tarea. Esta situacion se da cuando se definen menos de 8 tareas.
	 * Estrictamente no existe necesidad de esto, solo es por seguridad.
	 */

	for (uint8_t i = 0; i < MAX_TASK_COUNT; i++)  {
		if(i>=controlOS.cantidad_Tareas)
			controlOS.listaTareas[i] = NULL;
	}
}


static void scheduler(void)  {
	uint8_t indice;		//variable auxiliar para legibilidad

	/*
	 * El scheduler recibe la informacion desde la variable estado de sistema si es el primer ingreso
	 * desde el ultimo reset. Si esto es asi, determina que la tarea actual es la primer tarea.
	 */
	if (controlOS.statusOS == OS_FROM_RESET)  {
		controlOS.currentTask = (taskStructure_t *) controlOS.listaTareas[0];
	}
	else {

		/*
		 * Obtenemos la siguiente tarea en el vector. Si se llega a la ultima tarea disponible
		 * se hace un reset del contador
		 */
		indice = controlOS.currentTask->id+1;
		if(indice < controlOS.cantidad_Tareas)  {
			controlOS.nextTask = (taskStructure_t *) controlOS.listaTareas[indice];
		}
		else  {
			controlOS.nextTask = (taskStructure_t *) controlOS.listaTareas[0];
		}
	}

}


/*
 SysTick Handler.

El handler del Systick no debe estar a la vista del usuario. En este handler se llama al
scheduler y luego de determinarse cual es la tarea siguiente a ejecutar, se setea como
pendiente la excepcion PendSV.
*/
void SysTick_Handler(void)  {

	/*
	 * Dentro del SysTick handler se llama al scheduler. Separar el scheduler de
	 * getContextoSiguiente da libertad para cambiar la politica de scheduling en cualquier
	 * estadio de desarrollo del OS. Recordar que scheduler() debe ser lo mas corto posible
	 */

	scheduler();

	/**
	 * Se setea el bit correspondiente a la excepcion PendSV
	 */
	SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;

	/**
	 * Instruction Synchronization Barrier; flushes the pipeline and ensures that
	 * all previous instructions are completed before executing new instructions
	 */
	__ISB();

	/**
	 * Data Synchronization Barrier; ensures that all memory accesses are
	 * completed before next instruction is executed
	 */
	__DSB();
}




/*Cambio de contexto a partir de la excepción Pend_SV del NVIC.
an example sequence of context switching using PendSV with
the following event sequence:

1. Task A calls SVC for task switching (for example, waiting for some work to
complete).
2. The OS receives the request, prepares for context switching, and pends the
PendSV exception.
3. When the CPU exits SVC, it enters PendSV immediately and does the context
switch.
4. When PendSV finishes and returns to Thread level, it executes Task B.
5. An interrupt occurs and the interrupt handler is entered.
6. While running the interrupt handler routine, a SYSTICK exception (for OS
tick) takes place.
7. The OS carries out the essential operation, then pends the PendSV exception
and gets ready for the context switch.
8. When the SYSTICK exception exits, it returns to the interrupt service routine.
9. When the interrupt service routine completes, the PendSV starts and does the
actual context-switch operations.
10. When PendSV is complete, the program returns to Thread level; this time it
returns to Task A and continues the processing.

Besides context switching in an OS environment, PendSV can also be used in
systems without an OS. For example, an interrupt service can need a fair amount
of time to process. The first portion of the processing might need a high priority,
but if the whole ISR is executed with a high priority level, other interrupt services
would be blocked out for a long time.

Fuente:Yiu, J. (2013). The Definitive Guide to ARM® Cortex®-M3 and Cortex®-M4 Processors. Newnes.

*/
uint32_t getNextContext(uint32_t sp_actual)  {
	uint32_t sp_siguiente;


	/*
	 * En la primera llamada a getNextContext, se designa que la primer tarea a ejecutar sea
	 * la tarea actual, la cual es la primer tarea inicializada y cuyo puntero de estructura fuese
	 * cargado por la funcion scheduler (observar flujo de programa). Como todas las tareas se crean
	 * en estado READY, directamente se cambia a estado RUNNING y se actualiza la variable de estado
	 * de sistema
	 */

	if (controlOS.statusOS == OS_FROM_RESET)  {
		sp_siguiente = controlOS.currentTask->stack_pointer;
		controlOS.currentTask->taskStatusRRB = TASK_RUNNING;
		controlOS.statusOS = OS_NORMAL_RUN;
	}

	/*
	 * En el caso que no sea la primera vez que se ejecuta esta funcion, se hace un cambio de contexto
	 * de manera habitual. Se guarda el MSP (sp_actual) en la variable correspondiente de la estructura
	 * de la tarea corriendo actualmente. Como a este punto no hay mas estados implementados (solamente
	 * READY y RUNNING) se pasa la tarea actual a estado READY.
	 * Se carga en la variable sp_siguiente el stack pointer de la tarea siguiente, que fue definida
	 * por el scheduler. Se actualiza la misma a estado RUNNING y se retorna al handler de PendSV
	 */
	else {
		controlOS.currentTask->stack_pointer = sp_actual;
		controlOS.currentTask->taskStatusRRB = TASK_READY;

		sp_siguiente = controlOS.nextTask->stack_pointer;

		controlOS.currentTask = controlOS.nextTask;
		controlOS.currentTask->taskStatusRRB = TASK_RUNNING;
	}

	return sp_siguiente;
}


/*Esta tarea es una tarea especial en el caso que el OS no tenga tareas definidas por el usuario
o que todas las tareas esten en estado BLOCKED.
============================================================================================
The idle task is created automatically when the RTOS scheduler is started to ensure there 
is always at least one task that is able to run. It is created at the lowest possible 
priority to ensure it does not use any CPU time if there are higher priority application 
tasks in the ready state.
The idle task is responsible for freeing memory allocated by the RTOS to tasks that have 
since been deleted. It is therefore important in applications that make use of the 
vTaskDelete() function to ensure the idle task is not starved of processing time. 
The idle task has no other active functions so can legitimately be starved of 
microcontroller time under all other conditions.
It is possible for application tasks to share the idle task priority 
(tskIDLE_PRIORITY). See the configIDLE_SHOULD_YIELD configuration parameter for 
information on how this behaviour can be configured.

Fuente:freeRTOS.org
*/
static void initTaskIdle(void)  {
	taskIdleStructure.stack[STACK_SIZE/4 - XPSR] = INIT_XPSR;					//necesario para bit thumb
	taskIdleStructure.stack[STACK_SIZE/4 - PC_REG] = (uint32_t)taskIdle;		//direccion de la tarea (ENTRY_POINT)
	taskIdleStructure.stack[STACK_SIZE/4 - LR] = (uint32_t)returnHook;			//Retorno de la tarea (no deberia darse)


	taskIdleStructure.stack[STACK_SIZE/4 - LR_PREV_VALUE] = EXEC_RETURN;
	taskIdleStructure.stack_pointer = (uint32_t) (taskIdleStructure.stack + STACK_SIZE/4 - FULL_STACKING_SIZE);

	taskIdleStructure.entry_point = taskIdle;
	taskIdleStructure.id = 0xFF;
	taskIdleStructure.taskStatusRRB = TASK_READY;
}




