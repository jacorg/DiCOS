#ifndef DiC_OS_Interrupts
#define DiC_OS_Interrupts

void addingInterrupt ( void (*handlerN )(void), uint8_t irqCh, uint8_t prior);
void deletingInterrupt       ( uint8_t irqch );
void DiCOS_InterruptHandler( uint8_t irqCh );
void nothing (void);
#endif
