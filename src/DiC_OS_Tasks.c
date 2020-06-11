#include "DiC_OS_Tasks.h"
#include "DiC_OS_API.h"

//Author: Jacobo Salvador
//Defino las 8 tareas del sistema operativ, por requerimientos las prioridades van de 0 a 3
void Task_0(void){
    uint32_t a = 0;
    while(1){
        a++;
        os_Delay(2);
    }
}

void Task_1(void){
    uint32_t b = 0;
    while(1){
        b++;
        os_Delay(5);
    }
}

void Task_2(void){
    uint32_t c = 0;
    while(1){
        c++;
//        os_Delay(20);
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