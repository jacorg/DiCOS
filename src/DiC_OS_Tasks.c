#include "DiC_OS_Tasks.h"
//#include "DiC_OS_API.h"
#include "sapi.h"

//Author: Jacobo Salvador
//Defino las 8 tareas del sistema operativ, por requerimientos las prioridades van de 0 a 3
void Task_0(void){
    
    while(1){
        //semTake(&semLed1);
        os_Delay(400);
        gpioWrite(LED1,true);
        os_Delay(400);
        gpioWrite(LED1,false);
      
       
/*       DatosQueue.dato_int=5;
        DatosQueue.txt[0]='a';
        DatosQueue.txt[1]='b';
        DatosQueue.txt[2]='c';
        queuePut(&Tarea1,&DatosQueue);
        DatosQueue.dato_int=10;
        DatosQueue.txt[0]='x';
        DatosQueue.txt[1]='y';
        DatosQueue.txt[2]='z';
        queuePut(&Tarea1,&DatosQueue);*/
    }
}

void Task_1(void){
    
    while(1){
         os_Delay(250);
        gpioWrite(LED2,true);
        os_Delay(250);
        gpioWrite(LED2,false);
        //queueGet(&Tarea1,&DatosQueRx);
    }
}

void Task_2(void){
    
    while(1){
        /*os_Delay(5000);
        gpioWrite(LED3,true);
        os_Delay(5000);
        gpioWrite(LED3,false);*/
    }
}

void Task_3(void){
    uint32_t d = 0;
    while(1){
        d++;
    }
}

void Task_4(void){
    uint32_t e = 0;
    while(1){
        e++;
    }
}
void Task_5(void){
    uint32_t f = 0;
    while(1){
        f++;
    }
}

void Task_6(void){
    uint32_t g = 0;
    while(1){
        g++;
    }
}

void Task_7(void){
    uint32_t h = 0;
    while(1){
        h++;
    }
}