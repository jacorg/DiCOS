#include "DiC_OS_Tasks.h"
//#include "DiC_OS_API.h"


//Author: Jacobo Salvador
//Defino las 3 tareas del sistema operativo.


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
    
    //Configuración de la UART y msj de bienvenida
    uartConfig(UART_USB, 115200 );
	uartWriteString (UART_USB ,"DiC_OS: Examen de ISO-I-Jacobo Salvador.\n\r");
    
    while(1){
        semTake(&endTEC1); //con los semaforos binarios me permite asegurar que se termino
        semTake(&endTEC2); //el ciclo de pulsado y soltado en ambas teclas.
                           //ahora me dedico a calcular la diferencia de tiempos y analizar 
                           //los cuatro casos posibles.
        
        t1=(int32_t)ticksFallingTEC1-(int32_t)ticksFallingTEC2;
        t2=(int32_t)ticksRisingTEC1-(int32_t)ticksRisingTEC2;
        ton=(uint32_t)(abs(t1)+abs(t2));  //Sumo el valor absoluto de los dos tiempos
        
/*
En los if, else if debajo verifico la condición de que LED corresponde encenderse y genero
el delay, tiempo durante el cual la tarea esta en estado BLOCKEADA.
El systick es el responsable de barrer entre todas las tarees y descontar el ticks_blocked--
hasta que sea cero. Luego la tarea pasa al estado READY.
*/     
        if (t1<=0 && t2<=0){      //LED VERDE
            gpioWrite(LED3,true);
            os_Delay(ton);
            gpioWrite(LED3,false);
            printMessages(VERDE,&t1,&t2);
        }
        else if(t1<=0 && t2>0){  // LED ROJO
            gpioWrite(LED1,true);
            os_Delay(ton);
            gpioWrite(LED1,false);
            uartWriteString (UART_USB ,"Led Rojo encendido\n\r");
            printMessages(ROJO,&t1,&t2);
        }
        else if(t1>0 && t2<=0){  // LED AMARILLO
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
            gpioWrite(LEDR,true);
    }
}



//estas son las funciones que se instalan en el osHandlerVector para ser llamadas ante una
//interrupcion de teclas.
void TEC1_Handler(void)
{
   Chip_PININT_ClearIntStatus (LPC_GPIO_PIN_INT ,PININTCH(TEC1_i)); // limpia el flag                                                 
    semGive(&semTEC1); //Libero semaforo de la Tecla 1
}
//idem gpio1handler
void TEC2_Handler(void)
{
   Chip_PININT_ClearIntStatus (LPC_GPIO_PIN_INT,PININTCH(TEC2_i));
   semGive(&semTEC2);//Libero semaforo de la Tecla 2
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
