#ifndef DiC_OS_keys
#define DiC_OS_keys

#include "DiC_OS_API.h"


/*---------------------Definiciones para menejo de interrupciones con teclas-------------*/
//#define TEC1_i 1
//#define TEC2_i 2


enum tec{
    TEC1_i=1,
    TEC2_i
};
typedef enum tec tec_t;

enum port{
    TEC1_PORT=0,
    TEC2_PORT=0
};
typedef enum port port_t;

enum tec_bit_eval{
    TEC1_BIT_EVAL=4,
    TEC2_BIT_EVAL=8
};
typedef enum tec_bit_eval tec_bit_eval_t;

enum edgeActivation{
    FALLING_INT,
    RISING_INT,
    FALLING_AND_RISING_INT
};

typedef enum edgeActivation edgeActivation_t;
/*-------------------------------Variable globales semaforos-----------------------------*/
/*
Genero dos semaforos para controlar la interrupcion del teclado. La tecla al ser 
presionada debera soltarse en un tiempo dado. El semaforo controla cuando la tecla es
presionada y cuando es soltada.
*/
semDiCOS_t semTEC1, semTEC2, endTEC1, endTEC2;


/*-----------------------------Prototipo funciones Teclas----------------------------------*/
void setupTECsInterrupts(tec_t key,port_t port,tec_bit_eval_t tec_bit_eval,edgeActivation_t edge);
void TEC1_Handler(void);
void TEC2_Handler(void);
void addingInterrupts(void);//Agrego handler de interrupciones para la tecla 1 y 2
#endif




