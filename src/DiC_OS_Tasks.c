#include "DiC_OS_Tasks.h"
#include "DiC_OS_keys.h"


//Author: Jacobo Salvador
//Defino las 3 tareas del sistema operativo.

/*
Tarea 1 asociada con el manejo de la tecla 1, la comunicacion con la tarea 3 se hace por medio
de colas
*/
void Task_0(void){

    statusTEC1=FALLING;                            //Variabld de la máquina de estados
    uint32_t ticksFallingTEC1=0;
    uint32_t ticksRisingTEC1=0;
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
            timeEventTx1.timeTECs.tecla=TIPO_TEC1;
            timeEventTx1.timeTECs.t1=ticksFallingTEC1;
 
            break;

        case RISING:
            ticksRisingTEC1=getTicksFromOS();
            timeEventTx1.timeTECs.t2=ticksRisingTEC1;
            statusTEC1=FALLING;
            queuePut(&msgTimeTEC1,&timeEventTx1);
            //semGive(&endTEC1);   //Indico final de un ciclo de pulsación
            break;
        
        default:
            break;
        }
    }
}
/*
Tarea 2 asociada con el manejo de la tecla 2, la comunicacion con la tarea 3 se hace por medio
de colas
*/
void Task_1(void){
  
    statusTEC2=FALLING;                            //Variabld de la máquina de estados
    uint32_t ticksFallingTEC2=0;
    uint32_t ticksRisingTEC2=0;
    
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
            timeEventTx2.timeTECs.tecla=TIPO_TEC2;
            timeEventTx2.timeTECs.t1=ticksFallingTEC2;
            statusTEC2=RISING; //preparo para el proximo estado
            break;

        case RISING:
            ticksRisingTEC2=getTicksFromOS();
            timeEventTx2.timeTECs.t2=ticksRisingTEC2;
            statusTEC2=FALLING;
            queuePut(&msgTimeTEC2,&timeEventTx2);
            //semGive(&endTEC2);  //Indico final de un ciclo de pulsación
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
    uint32_t ticksFallingTEC1;  //No es necesario resetear estos valores
    uint32_t ticksRisingTEC1;   //a cero dentro de la while true 
                                //porque la medición de tiempo es relativa a otra tecla.

    uint32_t ticksFallingTEC2;   //No es necesario resetear estos valores
    uint32_t ticksRisingTEC2;    //a cero dentro de la while true 
                                 //porque la medición de tiempo es relativa a otra tecla.
    int32_t t1=0;
    int32_t t2=0;
    uint32_t ton=0;

	uartWriteString (UART_USB ,"DiC_OS: Examen de ISO-I-Jacobo Salvador.\n\r");
    
    while(1){
              
        
        /*
        -.SE DOCUMENTA QUE PRESIONES DESBALANCESADAS 10 VECES LA TECLA 1 Y LUEGO UNA
        COMBINACION CORRECTA PRODUCE QUE ESA UNICA VISUALIZACION NO SE PRENDA, ES POR 
        LA CARGA DE DATOS DE LA COLA

        -.SE CORRIGE UN ERROR QUE ES CUANDO SE PRESIONAN SIN QUE LAS TECLAS SE SOLAPEN
        FUNCIONA.

        -.PARA EL SOLAPAMIENTO TODAS LAS COMBINACIONES SOLAPADAS FUNCIONANA BIEN
        */
        
        /*
        Extraigo dato de la cola y analizo que tecla fue la que llego
        clasifico y calculo los tiempos
        */
       queueGet(&msgTimeTEC1,&timeEventRx1);
       if (timeEventRx1.timeTECs.tecla==TIPO_TEC1){
           ticksFallingTEC1=timeEventRx1.timeTECs.t1;
            ticksRisingTEC1=timeEventRx1.timeTECs.t2;
       }
 
        queueGet(&msgTimeTEC2,&timeEventRx2);
       if (timeEventRx2.timeTECs.tecla==TIPO_TEC2){
           ticksFallingTEC2=timeEventRx2.timeTECs.t1;
           ticksRisingTEC2=timeEventRx2.timeTECs.t2;
       }

        /*
        El usuario presiono equivocadamente y no hubo solapamiento d teclas asi que chequeo
        */
       if((ticksRisingTEC1<ticksFallingTEC2)||(ticksRisingTEC2<ticksFallingTEC1)){
            t1=0;
            t2=0;
            ton=0; //no hace nada no hay solapamiento
       }

       else{
        t1=(int32_t)ticksFallingTEC1-(int32_t)ticksFallingTEC2;
        t2=(int32_t)ticksRisingTEC1-(int32_t)ticksRisingTEC2;
        ton=(uint32_t)(abs(t1)+abs(t2));  //Sumo el valor absoluto de los dos tiempos
       }
        
        
/*
En los if, else if debajo verifico la condición de que LED corresponde encenderse y genero
el delay, tiempo durante el cual la tarea esta en estado BLOCKEADA.
El systick es el responsable de barrer entre todas las tarees y descontar el ticks_blocked--
hasta que sea cero. Luego la tarea pasa al estado READY.
*/  

        if (t1<0 && t2<0){      //LED VERDE
            gpioWrite(LED3,true);
            os_Delay(ton);
            gpioWrite(LED3,false);
            printMessages(VERDE,&t1,&t2);
        }
        else if(t1<0 && t2>0){  // LED ROJO
            gpioWrite(LED1,true);
            os_Delay(ton);
            gpioWrite(LED1,false);
            uartWriteString (UART_USB ,"Led Rojo encendido\n\r");
            printMessages(ROJO,&t1,&t2);
        }
        else if(t1>0 && t2<0){  // LED AMARILLO
            gpioWrite(LED2,true);
            os_Delay(ton);
            gpioWrite(LED2,false);
            uartWriteString (UART_USB ,"Led Amarillo encendido\n\r");
            printMessages(AMARILLO,&t1,&t2);
        }

        else if(t1>0 && t2>0){  // LED AZUL
            gpioWrite(LEDB,true);
            os_Delay(ton);
            gpioWrite(LEDB,false);
            uartWriteString (UART_USB ,"Led Azul encendido\n\r");
            printMessages(AZUL,&t1,&t2);
        }
        else
            nothingFunc();
    
    
    //inicializo estas variables las uso para determinar errores
    ticksRisingTEC1=ticksFallingTEC1=ticksRisingTEC2=ticksFallingTEC2=0;
    }

}

/*
Muestra los mensajes por UART de los eventos producidos. Tiempo de encendido,
tiempo entre flancos de bajada y de subida.
*/
void printMessages(colorLEDs_t colorLED,int32_t * t1, int32_t *t2){

    switch (colorLED)
    {
    case VERDE:
        uartWriteString (UART_USB ,"\n\r");
        uartWriteString (UART_USB ,"Led Verde encendido\n\r");
        sendDataTimeUART(t1, t2);  
        break;
    
    case ROJO:
        uartWriteString (UART_USB ,"\n\r");
        uartWriteString (UART_USB ,"Led Rojo encendido\n\r");
        sendDataTimeUART(t1, t2);
        break;
    case AMARILLO:
        uartWriteString (UART_USB ,"\n\r");
        uartWriteString (UART_USB ,"Led Amarillo encendido\n\r");
        sendDataTimeUART(t1, t2);
        break;
    
    case AZUL:
        uartWriteString (UART_USB ,"\n\r");
        uartWriteString (UART_USB ,"Led Azul encendido\n\r");
        sendDataTimeUART(t1, t2);
        break;
    
    default:
        break;
    }
}

/*
Utilizo esta función para enviar datos de tiempos a la UART desde las variavles ton, t1 y t2
*/
void sendDataTimeUART(int32_t * t1, int32_t *t2){
    uartWriteString (UART_USB,"Tiempo encendido:");
    itoa(abs(*t1)+abs(*t2), charTon, 10 ); /* base 10 significa decimal */
    uartWriteString (UART_USB,charTon);
    uartWriteString (UART_USB," ms\r\n");

    uartWriteString (UART_USB,"Tiempo entre flancos descendentes:");
    itoa(abs(*t1), charTon, 10 ); /* base 10 significa decimal */
    uartWriteString (UART_USB,charTon);
    uartWriteString (UART_USB," ms \r\n");

    uartWriteString (UART_USB,"Tiempo entre flancos ascendentes:");
    itoa(abs(*t2), charTon, 10 ); /* base 10 significa decimal */
    uartWriteString (UART_USB,charTon);
    uartWriteString (UART_USB," ms \r\n");
}

/**
 * C++ version 0.4 char* style "itoa":
 * Written by Lukás Chmela
 * Released under GPLv3.
 */
char* itoa(int value, char* result, int base) {
   // check that the base if valid
   if (base < 2 || base > 36) { *result = '\0'; return result; }

   char* ptr = result, *ptr1 = result, tmp_char;
   int tmp_value;

   do {
      tmp_value = value;
      value /= base;
      *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
   } while ( value );

   // Apply negative sign
   if (tmp_value < 0) *ptr++ = '-';
   *ptr-- = '\0';
   while(ptr1 < ptr) {
      tmp_char = *ptr;
      *ptr--= *ptr1;
      *ptr1++ = tmp_char;
   }
   return result;
}


void nothingFunc(void){
    queueInit(&msgTimeTEC1);
    queueInit(&msgTimeTEC2);
}