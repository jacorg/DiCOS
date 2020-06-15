#include "DiC_OS_Kernel.h"

/*
Definición de la estructura para implementación de semaforos binarios sobre DiC_OS.
*/

#define TAKE 1
#define GIVE 0

#define QUEUE_SIZE 64


struct dataTypeQueue{
    int32_t dato_int;
	char txt[3];

};
typedef struct dataTypeQueue dataTypeQueue_t;

struct queue {
	dataTypeQueue_t data[QUEUE_SIZE];
	taskStructure_t* task;
	uint16_t idx_head;
	uint16_t idx_tail;
    uint16_t maxlen;
};

typedef struct queue queue_t;


struct semDiCOS {
	taskStructure_t* task;
	bool semState;
};
typedef struct semDiCOS semDiCOS_t;


extern controlOS_t controlOS;

void os_Delay(uint32_t ticks);

/*Prototipo de funciones para manejod de semaforos*/
void semInit(semDiCOS_t* sem);
void semTake(semDiCOS_t* sem);
void semGive(semDiCOS_t* sem);

/*Prototipo de las funciones para manejo de colas*/
void queueInit(queue_t* queue);
void queuePut(queue_t* queue, dataTypeQueue_t* data);
void queueGet(queue_t* queue, dataTypeQueue_t* data);