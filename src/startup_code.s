.syntax unified
.cpu cortex-m4
.thumb

.section .text.Reset_Handler
.global Reset_Handler
.type Reset_Handler, %function


Reset_Handler:                           
    BL resetHandlerHelper
    B main
hang:
    B .

.size Reset_Handler, . - Reset_Handler

/************************ Default_Handler start**********************/
.section .text.Default_Handler
.global Default_Handler
.type Default_Handler, %function
Default_Handler :
    B .

.size Default_Handler, . - Default_Handler

/************************ NMI_Handler start**********************/
.section .text.NMI_Handler
.global NMI_Handler
.type NMI_Handler, %function
NMI_Handler :
    B .

.size NMI_Handler, . - NMI_Handler

/************************ DebugMon_Handler start**********************/
.section .text.DebugMon_Handler
.global DenubMon_Handler
.type DebugMon_Handler, %function
DebugMon_Handler :
    B .

.size DebugMon_Handler, . - DebugMon_Handler

/************************ HardFault_Handler start**********************/
.section .text.HardFault_Handler
.global HardFault_Handler 
.type HardFault_Handler, %function
HardFault_Handler :
    B .

.size HardFault_Handler, . - HardFault_Handler 

/************************ MemManage_Handler start**********************/
.section .text.MemManage_Handler
.global MemManage_Handler 
.type MemManage_Handler, %function
MemManage_Handler :
    B .

.size MemManage_Handler, . - MemManage_Handler 

/************************ BusFault_Handler start**********************/
.section .text.BusFault_Handler
.global BusFault_Handler
.type BusFault_Handler, %function
 BusFault_Handler :
    B .

.size BusFault_Handler, . - BusFault_Handler 

/************************ UsageFault_Handler start**********************/
.section .text.UsageFault_Handler
.global UsageFault_Handler 
.type UsageFault_Handler, %function
 UsageFault_Handler :
    B .

    .size UsageFault_Handler, . - UsageFault_Handler 

/************************ SVC_Handler start**********************/
.section .text.SVC_Handler
.global SVC_Handler 
.type SVC_Handler, %function
SVC_Handler :
    B .

    .size SVC_Handler, . - SVC_Handler 

/************************ PendSV_Handler start**********************/
.section .text.PendSV_Handler
.global PendSV_Handler 
.type PendSV_Handler, %function
PendSV_Handler :
    B .

    .size PendSV_Handler, . - PendSV_Handler 

/************************ SysTick_Handler start**********************/
.section .text.SysTick_Handler
.global SysTick_Handler 
.type SysTick_Handler, %function
SysTick_Handler :
    B .

    .size SysTick_Handler, . - SysTick_Handler 



/************************ USART1_IRQHandler **********************/
.section .text.USART1_IRQHandler
.global  USART1_IRQHandler
.type USART1_IRQHandler, %function
 USART1_IRQHandler :
    BL USART1_IRQHandler_c

    .size USART1_IRQHandler, . - USART1_IRQHandler 


/***************************** vtable *******************************/

.section .isr_vector, "a", %progbits
.global vector_table
.type vector_table, %object

vector_table:
    .word _estack           // msp value
    .word Reset_Handler     
    
    .word NMI_Handler           /*  NMI handler */
    .word HardFault_Handler     /*  Hard fault handler */
    .word MemManage_Handler     /*  mem management handler */
    .word BusFault_Handler      /*  bus fault */
    .word UsageFault_Handler    /*  usage fault */
    .word Default_Handler       /*  reserved */
    .word Default_Handler       /*  reserved */
    .word Default_Handler       /*  reserved */
    .word Default_Handler       /*  reserved */
    .word SVC_Handler           /*  SVC call handler */
    .word DebugMon_Handler      /*  Debug moniter */
    .word Default_Handler       /*  reserver  */
    .word PendSV_Handler        /*  pend sv handler */
    .word SysTick_Handler       /*  systick timer handler */
    
    .rept 37
        .word Default_Handler
    .endr
    .word USART1_IRQHandler

.size vector_table, . - vector_table


