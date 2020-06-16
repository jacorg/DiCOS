#include "DiC_OS_Tasks.h"
//#include "DiC_OS_API.h"


//Author: Jacobo Salvador
//Defino las 8 tareas del sistema operativ, por requerimientos las prioridades van de 0 a 3


void Task_0(void){
/*
Acá coloco el setup de las interrupciones para que se activen tanto en flanco de bajada
como de subida.
*/
    
    Chip_SCU_GPIOIntPinSel (TEC1_i,0 ,4 ) ;
    Chip_PININT_ClearIntStatus (LPC_GPIO_PIN_INT,PININTCH( TEC1_i ));
    Chip_PININT_SetPinModeEdge (LPC_GPIO_PIN_INT,PININTCH( TEC1_i ));
    Chip_PININT_EnableIntLow   (LPC_GPIO_PIN_INT,PININTCH( TEC1_i ));
    Chip_PININT_EnableIntHigh  (LPC_GPIO_PIN_INT,PININTCH( TEC1_i ));
    addingInterrupt( TEC1_Handler ,PIN_INT0_IRQn+TEC1_i,255);

    statusTEC1=FALLING;                            //Variabld de la máquina de estados
    ticksFallingTEC1=0;
    ticksRisingTEC1=0;
    while(1){
        
        semTake(&semTEC1);
        /*
        Genero una maquina de estados a partir del semaforo que libera
        la interrupcion de la tecla TEC1. Cuando la presiono la primera vez
        pasa a FALLING, en el siguiente estado se entra una vez que la tecla se solto.
        */
        switch (statusTEC1)
        {
        case FALLING:
            ticksFallingTEC1=getTicksFromOS();
            statusTEC1=RISING; //preparo para el proximo estado
            break;

        case RISING:
            ticksRisingTEC1=getTicksFromOS();
            statusTEC1=FALLING;
            semGive(&endTEC1);   //Indico final de un ciclo de pulsación
            break;
        
        default:
            break;
        }
    }
}

void Task_1(void){
    
    Chip_SCU_GPIOIntPinSel(TEC2_i,0 ,8 ) ;
    Chip_PININT_ClearIntStatus (LPC_GPIO_PIN_INT,PININTCH(TEC2_i));
    Chip_PININT_SetPinModeEdge (LPC_GPIO_PIN_INT,PININTCH(TEC1_i));
    Chip_PININT_EnableIntLow   (LPC_GPIO_PIN_INT,PININTCH(TEC2_i));
    Chip_PININT_EnableIntHigh  (LPC_GPIO_PIN_INT,PININTCH(TEC2_i));
    addingInterrupt( TEC2_Handler ,PIN_INT0_IRQn+TEC2_i,255); 
    
    statusTEC2=FALLING;                            //Variabld de la máquina de estados
    ticksFallingTEC2=0;
    ticksRisingTEC2=0;
    
    while(1){

    semTake(&semTEC2);
    /*
    Genero una maquina de estados a partir del semaforo que libera
    la interrupcion de la tecla TEC1. Cuando la presiono la primera vez
    pasa a FALLING, en el siguiente estado se entra una vez que la tecla se solto.
    */    

   switch (statusTEC2)
        {
        case FALLING:
            ticksFallingTEC2=getTicksFromOS();
            statusTEC2=RISING; //preparo para el proximo estado
            break;

        case RISING:
            ticksRisingTEC2=getTicksFromOS();
            statusTEC2=FALLING;
            semGive(&endTEC2);  //Indico final de un ciclo de pulsación
            break;
        
        default:
            break;
        }

    }
}




/*
En esta tarea me encargo de tomar el juego de tiempos de TEC1 y TEC2 completado su ciclo 
completo de pulsación y soltado, y empiezo a genera la diferencia de tiempos entre TEC1 y TEC2.
En base a el resultado de la diferencia de tiempos empezare a clasificar los cuatro (4) eventos.
*/

void Task_2(void){
    int32_t t1=0;
    int32_t t2=0;
    uint32_t ton=0;
    while(1){
        semTake(&endTEC1); //con los semaforos binarios me permite asegurar que se termino
        semTake(&endTEC2); //el ciclo de pulsado y soltado en ambas teclas.
                           //ahora me dedico a calcular la diferencia de tiempos y analizar 
                           //los cuatro casos posibles.
        
        t1=(int32_t)ticksFallingTEC1-(int32_t)ticksFallingTEC2;
        t2=(int32_t)ticksRisingTEC1-(int32_t)ticksRisingTEC2;
        ton=(uint32_t)(abs(t1)+abs(t2));             //Sumo el valor absoluto de los dos tiempos
        
        
        if (t1<=0 && t2<=0){      //LED VERDE
            gpioWrite(LED3,true);
            os_Delay(ton);
            gpioWrite(LED3,false);

        }
        else if(t1<=0 && t2>0){  // LED ROJO
            gpioWrite(LED1,true);
            os_Delay(ton);
            gpioWrite(LED1,false);

        }
        else if(t1>0 && t2<=0){  // LED AMARILLO
            gpioWrite(LED2,true);
            os_Delay(ton);
            gpioWrite(LED2,false);

        }

        else if(t1>0 && t2>0){  // LED AZUL
            gpioWrite(LEDB,true);
            os_Delay(ton);
            gpioWrite(LEDB,false);

        }
        else
            gpioWrite(LEDR,true);

    }
}





void Task_3(void){
    uint32_t d = 0;
    while(1){
        d++;
    }
}



}






//estas son las funciones que se instalan en el osHandlerVector para ser llamadas ante una
//interrupcion de teclas.
void TEC1_Handler(void)
{
   Chip_PININT_ClearIntStatus (LPC_GPIO_PIN_INT ,PININTCH(TEC1_i)); // limpia el flag
                                             
    
    semGive(&semTEC1);
    
    
}
//idem gpio1handler
void TEC2_Handler(void)
{
   Chip_PININT_ClearIntStatus (LPC_GPIO_PIN_INT,PININTCH(TEC2_i));
   
   semGive(&semTEC2);
}
