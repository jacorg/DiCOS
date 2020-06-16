/*
 * DiC_OS_Kernel.c
 *
 *  Author: Jacobo Salvador
 *  Se toma como guía  el OS desarrollado por Mg. Gonzales Sanchez.
 * (https://github.com/gonzaloesanchez/MSE_OS) 
 */

/*------------------------------Funciones en DiC_OS_Kernel------------------------------

|------>void createTask(void *entryPoint, taskStructure_t *task,uint8_t priority)
					
|------>void os_Init(void) 
				|------>static void initTaskIdle(void)

|------>void SysTick_Handler(void)
					|------>setTicksOS()
                    |------>static void scheduler(void)
					|------>checkTasksTiks()

|------>scheduler()
					|------>getFirstTask()
					|------>nTaskBlocked
					|------>selectTasks
				
|------>PendSV_Handler
				|------>uint32_t getNextContext(uint32_t sp_actual)
				
|------>os_getCurrentTask()
*/

#include "DiC_OS_Kernel.h"

/*==================[definicion de variables globales]=================================*/
static priorOS_t prior;
static controlOS_t controlOS;
static taskStructure_t taskIdleStructure;


static void triggerPendSV(void);

void __attribute__((weak)) tickHook(void)  {
	__asm volatile( "nop" );
}

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
		task->taskStatusRRB = TASK_READY;    //Siempre la tarea al crearse se inicia en READY

		task->priority=priority;             //Almaceno la prioridad de la tarea
     	
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
	Inicializo el sistema de Ticks del OS para calcular los tiempos de pulsados de teclas 
	y otro tipo de requerimiento que necesite la medición de tiempos.
	*/
	initTicksFromOS();
	
	
	
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
	/*Inicializo vector del sistema de prioridades con el valor NULL al inicio del OS.
	Luego, en el siguiente paso cargare los valores almacenados en listaTareas
	*/
	for(uint8_t j=0;j<CANT_PRIOR;j++){
		//controlOS.nBlockTasks[j]=0;              //inicializo el contador de tareas BLOCK en 0
		controlOS.taskxPrior[j]=0;               //inicializo el contador en 0
		for(uint8_t i=0;i < MAX_TASK_COUNT; i++) // inicializo el array de punteros a tareas en NULL
			controlOS.priorScheme[j][i]=NULL;
	}
	/*
	Recorro la lista de tareas que se cargaron. En base a la prioridad se va incrementando el taskxPrior
	de esa manera conozco la cantidad tareas por prioridad.	
	*/
	for(uint8_t i=0;i < MAX_TASK_COUNT; i++)  //  i: tareas
		if (controlOS.listaTareas[i] != NULL){ // por si el sistema tiene memos de 8 tareas
			controlOS.priorScheme[((taskStructure_t *) controlOS.listaTareas[i])->priority]     [controlOS.taskxPrior[((taskStructure_t *) controlOS.listaTareas[i])->priority]]=(taskStructure_t *) controlOS.listaTareas[i];
			controlOS.taskxPrior[((taskStructure_t *) controlOS.listaTareas[i])->priority]=controlOS.taskxPrior[((taskStructure_t *) controlOS.listaTareas[i])->priority]+1;
		//((taskStructure_t *) controlOS.listaTareas[i])->id // id de la tareas
		}
}


static void scheduler(void)  {
	uint8_t array_nTaskBlocked[CANT_PRIOR]; //En esta variable puntero almacenos todas
										    //las tareas blockeadas según su nivel de privilegio.
	
	uint8_t i=0;
	static uint8_t idx=0;             //indica estatico para moverme entre las tareas de una misma prioridad
	static uint8_t TaskPriortbExec=0; // Esta variable retiene el valor dd la prioriddad de la primera 
	                                  //tarea a ser ejecutada

	static uint8_t idx_tasks[CANT_PRIOR];	//En este araay tengo los indices a cada tarea que se ejecuta
	taskStructure_t *currentTask;                                        						  


	/*Acá verifico que la cant de tareas no sea cero o que todas las tareas esten
	bloqueadas en cuyo caso se entra a la función taskIdle
	*/
	
	
	TaskPriortbExec=getFirstTask();

	//if (controlOS.cantidad_Tareas==0 || controlOS.cantidad_Tareas==totalTasksBlocked())
	//	controlOS.currentTask = (taskStructure_t *) &taskIdle;
	if (controlOS.statusOS == OS_FROM_RESET && TaskPriortbExec!=ERROR_PRIOR )  {
		initIndexTasks(&idx_tasks);   //inicialiaza los indices de las tareas por prioridad
		controlOS.changeContext=true; //Es mi primera vez en el scheduler aunque no haya tareas deberia cambiar el contexto a idle
		controlOS.currentTask = (taskStructure_t *) controlOS.priorScheme[TaskPriortbExec][idx];
	}
	//else{
		/*
		ACA DEBERIA COLOCAR LA TAREA IDLE PORQUE SIGNIFICA QUE EL OS NO TIENE NINGUNA TAREA ACTIVA
		NI NINGUNA POR EJECUTARSE ES UN ERROR DEL PROGRAMADOR
		*/
	//}

	/*
	Aca hago una doble validación primero verifico si todas las tareas no estan bloqueadas y luego si el 
	indice es menor que la cantidad de tareas para la misma priorida.
	*/
	
	else {  
	
	/*
	La función nTaskBlocked es la responsable de determinar las cantidad de tareas que estan en
	estado BLOCKED. A partir de esta información, luego hace la diferencia con la cantidad total de tareas cargadas por nivel
	de prioridad y me da como resultado la cantidad de tareas en estado (RUNNING o READY) por 
	cada nivel. Si por ejemplo existen 3 tareas corriendo en el nivel 0 y las 3 se encuentran bloqueadas
	porque se usaron delays, entonces nTaskBlocked devuelve para el nivel 0 un valor igual a 0. los if-else if
	debajo determinan a que nivel de prioridad se debe ir a buscar otras tareas en estado READY.
	*/

		nTaskBlocked(&array_nTaskBlocked);

		/*============================== Tareas nivel prioridad 0 ==================================*/
		if(array_nTaskBlocked[PRIOR_0]!=0)
			selectTasks(PRIOR_0, &idx_tasks, &array_nTaskBlocked);
		
		/*============================== Tareas nivel prioridad 1 ==================================*/
		else if(array_nTaskBlocked[PRIOR_1]!=0)
			selectTasks(PRIOR_1, &idx_tasks, &array_nTaskBlocked);
		
		/*============================== Tareas nivel prioridad 2 ==================================*/
		else if(array_nTaskBlocked[PRIOR_1]!=0)
			selectTasks(PRIOR_2, &idx_tasks, &array_nTaskBlocked);

		/*============================== Tareas nivel prioridad 3 ==================================*/
		else if(array_nTaskBlocked[PRIOR_3]!=0)
			selectTasks(PRIOR_3, &idx_tasks, &array_nTaskBlocked);
		
		else{
		/*==Acá se deberia ejecutar la tarea idle porque no hay tareas activas o todas estan blockeadas*/
		/*El sistema se inicia en OS_FROM_RESET. Si el usuario no genero ninguna tarea apenas se 
		inicia el OS la tareas que comenzará a ejecutarse sera la taskIdle.

		Si el usuario, genero tareas pero en un momento determinado TODAS se encuentran bloqueadas 
		entonces también en ese caso se pasa a la tarea idle. Es importante para este punto determinar
		si se necesita hacer o no un cambio de contexto. Para ello se carga el contexto actual de 
		la tarea y se verifica el ID. La tarea IDLE tieen un ID muy diferente a las tareas ordinarias.
		*/
		if(controlOS.statusOS == OS_FROM_RESET){
			controlOS.currentTask = (taskStructure_t *) &taskIdleStructure;
			controlOS.changeContext = true;
		    }
		else{
			controlOS.nextTask = (taskStructure_t *) &taskIdleStructure;
			currentTask=os_getCurrentTask();
			if (currentTask->priority==ID_TASK_IDLE)
				controlOS.changeContext = false;
			else
				controlOS.changeContext = true;	
		}

		}	
	}

	if(controlOS.changeContext==true)
		triggerPendSV();


}

/*
Esta función permite seleccionar tareas en un mismo nivel de prioridad.
Se le pasa como parametros la prioridad del nivel y un indice que permite barrer las tareas
*/

void selectTasks(priorOS_t prior, uint8_t *idx_tasks, uint8_t *array_nTaskBlocked){

	bool found_READY=false;

		while(!found_READY){
					if(((taskStructure_t *) controlOS.priorScheme[prior][idx_tasks[prior]])->taskStatusRRB==TASK_READY){
					controlOS.nextTask = (taskStructure_t *) controlOS.priorScheme[prior][idx_tasks[prior]];
					controlOS.changeContext=true;
					found_READY=true;
					}
					else if(((taskStructure_t *) controlOS.priorScheme[prior][idx_tasks[prior]])->taskStatusRRB==TASK_RUNNING && array_nTaskBlocked[prior]==1){
						controlOS.changeContext=false;
						found_READY=true;
					}
					
					else{
					idx_tasks[prior]=idx_tasks[prior]+1;
						if(idx_tasks[prior]==controlOS.taskxPrior[prior])
							idx_tasks[prior]=0;
					}
		}
}

/*
Esta función es la reponsable de decirme cual es la primer tarea en ejecutarse
despues de un OS_FROM_RESET. Busca en el array de puntero a tareas. Busca en la primer columna
hasta encontrar la primer tarea como así su prioridad. La primera vez que se entra al scheduler
todas las tareas estan en estado READY
*/


uint8_t getFirstTask(void){
uint8_t getFirst=0xFF;
    //Busco la cantidad d tareas por prioridad de 0 a 3
	// el primer valor diferente de 0 tomo el indice 
	if(controlOS.taskxPrior[PRIOR_0]!=0)
		getFirst=PRIOR_0;
	else if (controlOS.taskxPrior[PRIOR_1]!=0)
		getFirst=PRIOR_1;	
	else if (controlOS.taskxPrior[PRIOR_2]!=0)	
		getFirst=PRIOR_2;
	else if (controlOS.taskxPrior[PRIOR_3]!=0)	
		getFirst=PRIOR_3;
	else
	    getFirst=ERROR_PRIOR;
	
	return getFirst;
}

/*
Simple función de inicializacion de los indices por cada prioridad a 0. Esto se realiza cuando 
se ingresa desde un OS_FROM_RESET.
*/
void initIndexTasks(uint8_t *idx_tasks){
	idx_tasks[PRIOR_0]=0;
	idx_tasks[PRIOR_1]=0;
	idx_tasks[PRIOR_2]=0;
	idx_tasks[PRIOR_3]=0;
}

/*
checkTasksTiks recorre todas las tareas y decrementa el valor de ticks en el caso 
de que ticks_blocked sea mayor que cero. Si se determina que la tarea estaba en estado 
BLOCKED y los ticks llegaron a cero entonces se procede a pasar al estado READY.
*/
void checkTasksTiks(void){

	uint8_t i=0,j=0;
/*El ciclo for debajo lo modifique porque se corre el riego de que ademas de desbloquear tareas
que esten BLOCKEADAS por timer tambien lo haga por el manejo de semaforo.
*/
	for(i=0;i<CANT_PRIOR;i++)
		for(j=0;j<controlOS.taskxPrior[i];j++)
			if(((taskStructure_t *)controlOS.priorScheme[i][j])!=NULL){
				if(((taskStructure_t *)controlOS.priorScheme[i][j])->ticks_blocked>0){
					((taskStructure_t *)controlOS.priorScheme[i][j])->ticks_blocked--;
					if(((taskStructure_t *)controlOS.priorScheme[i][j])->taskStatusRRB==TASK_BLOCKED && ((taskStructure_t *)controlOS.priorScheme[i][j])->ticks_blocked==0)
						((taskStructure_t *)controlOS.priorScheme[i][j])->taskStatusRRB=TASK_READY;				
				}
			
			}
}

/*
Determina la cantidad d tareas en estado RUNNING o READY por cada nivel de prioridad.
Se hace la diferencia entre la cantidad de tareas creadas en un nivel de prioridad menos la 
cantidad de tareas bloqueadas en el momento de hacer el scheduling.
*/

void nTaskBlocked(uint8_t *nBlocked){

	uint8_t i=0,j=0;
	uint8_t nBlockedAux[CANT_PRIOR];

	for(i=0;i<CANT_PRIOR;i++){
		nBlockedAux[i]=0;  //inicializo la variable de acumulacion de tareas bloqueadas.
		nBlocked[i]=0;
	}

	for (i=0;i<CANT_PRIOR;i++){
		for(j=0;j<controlOS.taskxPrior[i];j++){
			if(((taskStructure_t *)controlOS.priorScheme[i][j])!=NULL)
				if(((taskStructure_t *)controlOS.priorScheme[i][j])->taskStatusRRB==TASK_BLOCKED)
					nBlockedAux[i]=nBlockedAux[i]+1;
		}
	}
	/*
	Hago la diferencia entre la cantidad de tareas activas menos las blockeadas para 
	saber cuantas tareas tengo READY o RUNNING por prioridad
	*/
	for(i=0;i<CANT_PRIOR;i++)
		nBlocked[i]=controlOS.taskxPrior[i]-nBlockedAux[i];


}


/*
 SysTick Handler.

El handler del Systick no debe estar a la vista del usuario. En este handler se llama al
scheduler y luego de determinarse cual es la tarea siguiente a ejecutar, se setea como
pendiente la excepcion PendSV.
*/
void SysTick_Handler(void)  {

	/*
	Incremento TicksOS en uno, lo coloco acá porque el ssytick es la interrupcion de mayor 
	prioridad.
	*/
	setTicksOS();
	
	
	/*
	Acá debo cada vez que ingreso chequear las tareas que estan en estado BLOCKED
	y decrementar el valor de los ticks_blocked, cuando alcanzan un valor igual a cero
	la tarea pasa automaticamente al estado READY
	*/
	
	checkTasksTiks();
	
	
	/*
	 * Dentro del SysTick handler se llama al scheduler. Separar el scheduler de
	 * getContextoSiguiente da libertad para cambiar la politica de scheduling en cualquier
	 * estadio de desarrollo del OS. Recordar que scheduler() debe ser lo mas corto posible
	 */

	scheduler();

	//if(controlOS.changeContext)
	//	triggerPendSV();
	
	tickHook();
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
		
		if (controlOS.currentTask->taskStatusRRB == TASK_RUNNING)
			controlOS.currentTask->taskStatusRRB = TASK_READY;

		sp_siguiente = controlOS.nextTask->stack_pointer;
		controlOS.currentTask = controlOS.nextTask;
		controlOS.currentTask->taskStatusRRB = TASK_RUNNING;
	}

	controlOS.changeContext=false;
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
	taskIdleStructure.id = ID_TASK_IDLE;
	taskIdleStructure.taskStatusRRB = TASK_READY;
	taskIdleStructure.priority = ID_TASK_IDLE;
}

taskStatus_t* os_getCurrentTask(void)  {
	return controlOS.currentTask;
}

void CpuYield(void)  {
	scheduler();
}


void triggerPendSV(void){
	
	controlOS.changeContext=false;
	
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



/*
Devuelve los ticks relativos despues de un RESET del sistema, lo utilizo para calcular los tiempos.
*/
uint32_t getTicksFromOS(void){
	return controlOS.tickOS;
}

/*
Incrementos ticksOS del DiC_OS
*/
void setTicksOS(void){
	controlOS.tickOS++;
}

/*
Inicializo ticksOS del OS, despues de un RESET cada vez que ingresa en el systick se incrementa en 1
*/
void initTicksFromOS(void){
	controlOS.tickOS=0;
}

