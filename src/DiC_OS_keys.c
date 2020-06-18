#include "DiC_OS_keys.h"


/*
La función setupTECxInterrups es la responsable de configurar las interrupciones
de teclas como tambien el flanco de activacion que puden ser:
FALLING_INT
RISING_INT
FALLING_AND_RISING_INT
*/

void setupTECsInterrupts(tec_t key,port_t port,tec_bit_eval_t tec_bit_eval,edgeActivation_t edge){

    Chip_SCU_GPIOIntPinSel (key,port,tec_bit_eval);
    Chip_PININT_ClearIntStatus (LPC_GPIO_PIN_INT,PININTCH( key ));
    Chip_PININT_SetPinModeEdge (LPC_GPIO_PIN_INT,PININTCH( key ));
    switch (edge){
    case FALLING_INT:
        Chip_PININT_EnableIntLow   (LPC_GPIO_PIN_INT,PININTCH( key ));
        break;
    case RISING_INT:
        Chip_PININT_EnableIntHigh  (LPC_GPIO_PIN_INT,PININTCH( key ));
        break;
    case FALLING_AND_RISING_INT:
        Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT,PININTCH(key));
        Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT,PININTCH( key ));
        break;
    
    default:
        break;
    }
}

/*
Agrego handler de interrupciones para la tecla 1 y 1
*/
void addingInterrupts(void){
    addingInterrupt( TEC1_Handler ,PIN_INT0_IRQn+TEC1_i,255);
    addingInterrupt( TEC2_Handler ,PIN_INT0_IRQn+TEC2_i,255); 
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