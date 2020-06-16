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
	if((currentTask->ticks_blocked == 0) && (currentTask->taskStatusRRB == TASK_BLOCKED))
		currentTask->taskStatusRRB == TASK_READY;
}


/*
Intoducción semaforos

Los semáforos no tienen dependencia de ninguna tarea.
Pueden ser tomados por una tarea y liberados por otra.

Hay dos tipos:
Binarios: Sólo pueden tomar estados binarios (0 o 1).
Contadores: Pueden tener N estados (N-1 a 0).

Se pueden tomar dos acciones con un semáforo:
-.Give: Liberar el semáforo.
-.Take: Tomar el semáforo.
Si un semáforo está tomado, al ejecutar take la tarea entra en estado blocked.

Otra tarea debe hacer un give para que la tarea bloqueada pase a ready.
Siempre inician tomados.

*/
void semInit(semDiCOS_t* sem){
	sem->semState = TAKE;
	sem->task = NULL;
}

void semTake(semDiCOS_t* sem){
	
	taskStructure_t* taskCurrent;
	bool quit = false;

	taskCurrent = os_getCurrentTask();

	if (taskCurrent->taskStatusRRB == TASK_RUNNING){
		while(!quit){
			if(sem->semState==TAKE){
				taskCurrent->taskStatusRRB = TASK_BLOCKED;
				sem->task = taskCurrent;
				CpuYield();
			}
/*
Se entra acá en el caso de que el semafo fue liberado por un GIVE entonces el semaforo 
se vuelve a activar porque se ejecuto nuevamente semTAKE. Como queda dentro del while true
la tarea se bloquea.
*/
			else{ 
				quit=true;
				sem->semState=TAKE;
			}
		}
	}

}

void semGive(semDiCOS_t* sem){
	
	taskStructure_t* taskCurrent;
	taskCurrent = os_getCurrentTask();

	/*
	El condicional verifica si la tarea actual esta en modo RUNNING y ademas pregunta por el 
	semafoto asociado de la variable sem. Si esta tomado y la tarea que tiene no es la 
	NULL quiere decir que se puede desbloquear, pongo el semaforo en GIVE y la tarea la paso a READY.
	*/
	if (taskCurrent->taskStatusRRB == TASK_RUNNING && sem->semState == TAKE &&	sem->task != NULL){
		sem->semState = GIVE;
		sem->task->taskStatusRRB = TASK_READY;
	}

}

/*-----------------------------------------------------------------------------------------
---------------------------------------Queues----------------------------------------------
------------------------------------------------------------------------------------------*/
/*
REQUERIMIENTOS:
Mínima implementación: colas de un tipo de datos intrínseco (ejemplo: int) sin timeout.
-.La cantidad de elementos debe poder establecerse mediante #define QUEUE_SIZE 64
-.Si la cola está llena, la tarea que envía un dato nuevo debe bloquearse hasta que haya lugar
-.Si la cola está vacía, la tarea que lee un dato debe bloquearse
-.hasta que haya un dato que leer.
-.Una vez funcionando, si se desea, puede implementarse timeout.
*/

void queueInit(queue_t* queue){
	queue->idx_head = 0;
	queue->idx_tail = 0;
	queue->task = NULL;
	//queue->maxlen=QUEUE_SIZE;
}

/*https://embedjournal.com/implementing-circular-buffer-embedded-c/*/
void queuePut(queue_t* queue, dataTypeQueue_t* data){
	taskStructure_t* currentTask;
	uint16_t next;

    /*
	Si la tarea del otro lado de la cola estaba bloqueada porque no habia datos la desbloqueo 
	en este punto porque se supone que colocaré un dato
	*/
	if(((queue->idx_head == queue->idx_tail) && queue->task != NULL) && queue->task->taskStatusRRB == TASK_BLOCKED){
			queue->task->taskStatusRRB = TASK_READY;
	}
	
	currentTask=os_getCurrentTask();
	if(currentTask->taskStatusRRB == TASK_RUNNING){
		next = queue->idx_head + 1;  // next is where head will point to after this write.
    	if (next >= QUEUE_SIZE)
        	next = 0;

    	while (next == queue->idx_tail){                  // if the head + 1 == tail, circular buffer is full
       		currentTask->taskStatusRRB = TASK_BLOCKED;//return -1;  tarea se deberia bloquer ACA!!!!!!!!!!!!!1
			queue->task = currentTask;
			CpuYield();   
		}									
    	queue->data[queue->idx_head] = *data;   // Load data and then move
    	queue->idx_head = next;                 // head to next data offset.
                                                //return 0;  // return success to indicate successful push.
	}
}

/*https://embedjournal.com/implementing-circular-buffer-embedded-c/*/
void queueGet(queue_t* queue, dataTypeQueue_t* data){
	uint16_t next;
	taskStructure_t *currentTask;

	 /*
	Si la tarea del otro lado de la cola estaba bloqueada porque estaba llena para escribir dato 
	se desbloque en este punto porque se supone que leeré un dato.
	*/
	if( (queue->idx_head==queue->idx_tail) && (queue->task != NULL) && (queue->task->taskStatusRRB == TASK_BLOCKED))
		queue->task->taskStatusRRB = TASK_READY;
	
	currentTask = os_getCurrentTask();
	if(currentTask->taskStatusRRB == TASK_RUNNING){
		
		while (queue->idx_head == queue->idx_tail){  // if the head == tail, we don't have any data                                 
			currentTask->taskStatusRRB = TASK_BLOCKED;
			queue->task = currentTask;
			//queue->idx_head=queue->idx_tail=0;
			CpuYield();	
		}							  
	
		next = queue->idx_tail + 1;  // next is where tail will point to after this read.
    	if(next >= QUEUE_SIZE)
        	next = 0;
		
		*data = queue->data[queue->idx_tail];  	// Read data and then move
    	queue->idx_tail = next;              	// tail to next offset.   									
	}
	
}