.syntax unified
.cpu cortex-m4
.thumb

.section .vector_table, "a"
.globl g_vector_table
g_vector_table:
    .word _estack           /* 0: Initial Stack Pointer */
    .word Reset_Handler     /* 1: Reset Handler */
    .word NMI_Handler       /* 2: NMI Handler */
    .word HardFault_Handler /* 3: Hard Fault Handler */
    .word MemManage_Handler /* 4: MPU Fault Handler */
    .word BusFault_Handler  /* 5: Bus Fault Handler */
    .word UsageFault_Handler/* 6: Usage Fault Handler */
    .word 0                 /* 7: Reserved */
    .word 0                 /* 8: Reserved */
    .word 0                 /* 9: Reserved */
    .word 0                 /* 10: Reserved */
    .word SVC_Handler       /* 11: SVCall Handler */
    .word DebugMon_Handler  /* 12: Debug Monitor Handler */
    .word 0                 /* 13: Reserved */
    .word PendSV_Handler    /* 14: PendSV Handler */
    .word SysTick_Handler   /* 15: SysTick Handler */
    
    /* External Interrupts - ALL must have valid handlers */
    .word Default_Handler   /* 16: WWDG */
    .word Default_Handler   /* 17: PVD */
    .word Default_Handler   /* 18: TAMP_STAMP */
    .word Default_Handler   /* 19: RTC_WKUP */
    .word Default_Handler   /* 20: FLASH */
    .word Default_Handler   /* 21: RCC */
    .word Default_Handler   /* 22: EXTI0 */
    .word Default_Handler   /* 23: EXTI1 */
    .word Default_Handler   /* 24: EXTI2 */
    .word Default_Handler   /* 25: EXTI3 */
    .word Default_Handler   /* 26: EXTI4 */
    .word Default_Handler   /* 27: DMA1_Stream0 */
    .word Default_Handler   /* 28: DMA1_Stream1 */
    .word Default_Handler   /* 29: DMA1_Stream2 */
    .word Default_Handler   /* 30: DMA1_Stream3 */
    .word Default_Handler   /* 31: DMA1_Stream4 */
    .word Default_Handler   /* 32: DMA1_Stream5 */
    .word Default_Handler   /* 33: DMA1_Stream6 */
    .word Default_Handler   /* 34: ADC1_2_3 */
    .word Default_Handler   /* 35: CAN1_TX */
    .word Default_Handler   /* 36: CAN1_RX0 */
    .word Default_Handler   /* 37: CAN1_RX1 */
    .word Default_Handler   /* 38: CAN1_SCE */
    .word Default_Handler   /* 39: EXTI9_5 */
    .word Default_Handler   /* 40: TIM1_BRK_TIM9 */
    .word Default_Handler   /* 41: TIM1_UP_TIM10 */
    .word Default_Handler   /* 42: TIM1_TRG_COM_TIM11 */
    .word Default_Handler   /* 43: TIM1_CC */
    .word Default_Handler   /* 44: TIM2 */
    .word Default_Handler   /* 45: TIM3 */
    .word Default_Handler   /* 46: TIM4 */
    .word Default_Handler   /* 47: I2C1_EV */
    .word Default_Handler   /* 48: I2C1_ER */
    .word Default_Handler   /* 49: I2C2_EV */
    .word Default_Handler   /* 50: I2C2_ER */
    .word Default_Handler   /* 51: SPI1 */
    .word Default_Handler   /* 52: SPI2 */
    .word Default_Handler   /* 53: USART1 */
    .word Default_Handler   /* 54: USART2 */
    .word Default_Handler   /* 55: USART3 */
    .word Default_Handler   /* 56: EXTI15_10 */
    .word Default_Handler   /* 57: RTC_Alarm */
    .word Default_Handler   /* 58: OTG_FS_WKUP */
    .word Default_Handler   /* 59: TIM8_BRK_TIM12 */
    .word Default_Handler   /* 60: TIM8_UP_TIM13 */
    .word Default_Handler   /* 61: TIM8_TRG_COM_TIM14 */
    .word Default_Handler   /* 62: TIM8_CC */
    .word Default_Handler   /* 63: DMA1_Stream7 */
    .word Default_Handler   /* 64: FSMC */
    .word Default_Handler   /* 65: SDIO */
    .word Default_Handler   /* 66: TIM5 */
    .word Default_Handler   /* 67: SPI3 */
    .word Default_Handler   /* 68: UART4 */
    .word Default_Handler   /* 69: UART5 */
    .word Default_Handler   /* 70: TIM6_DAC */
    .word Default_Handler   /* 71: TIM7 */
    .word Default_Handler   /* 72: DMA2_Stream0 */
    .word Default_Handler   /* 73: DMA2_Stream1 */
    .word Default_Handler   /* 74: DMA2_Stream2 */
    .word Default_Handler   /* 75: DMA2_Stream3 */
    .word Default_Handler   /* 76: DMA2_Stream4 */
    .word Default_Handler   /* 77: ETH */
    .word Default_Handler   /* 78: ETH_WKUP */
    .word Default_Handler   /* 79: CAN2_TX */
    .word Default_Handler   /* 80: CAN2_RX0 */
    .word Default_Handler   /* 81: CAN2_RX1 */
    .word Default_Handler   /* 82: CAN2_SCE */
    .word Default_Handler   /* 83: OTG_FS */
    .word Default_Handler   /* 84: DMA2_Stream5 */
    .word Default_Handler   /* 85: DMA2_Stream6 */
    .word Default_Handler   /* 86: DMA2_Stream7 */
    .word Default_Handler   /* 87: USART6 */
    .word Default_Handler   /* 88: I2C3_EV */
    .word Default_Handler   /* 89: I2C3_ER */
    .word Default_Handler   /* 90: OTG_HS_EP1_OUT */
    .word Default_Handler   /* 91: OTG_HS_EP1_IN */
    .word Default_Handler   /* 92: OTG_HS_WKUP */
    .word Default_Handler   /* 93: OTG_HS */
    .word Default_Handler   /* 94: DCMI */
    .word Default_Handler   /* 95: CRYP */
    .word Default_Handler   /* 96: HASH_RNG */
    .word Default_Handler   /* 97: FPU */

.thumb_func
NMI_Handler:
    b NMI_Handler

.thumb_func
HardFault_Handler:
    
    b HardFault_Handler

.thumb_func
MemManage_Handler:
    b MemManage_Handler

.thumb_func
BusFault_Handler:
    b BusFault_Handler

.thumb_func
UsageFault_Handler:
    b UsageFault_Handler

.thumb_func
SVC_Handler:
    b SVC_Handler

.thumb_func
DebugMon_Handler:
    b DebugMon_Handler

.thumb_func
Default_Handler:
    /* All unhandled interrupts trap here */
    b Default_Handler

.section .text.Reset_Handler
.globl Reset_Handler
.thumb_func
Reset_Handler:
    /* Copy .data from FLASH to RAM */
    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata
copy_loop:
    cmp r1, r2
    bge copy_done
    ldr r3, [r0], #4
    str r3, [r1], #4
    b copy_loop
copy_done:
    /* Zero .bss */
    ldr r0, =_sbss
    ldr r1, =_ebss
    mov r2, #0
zero_loop:
    cmp r0, r1
    bge zero_done
    str r2, [r0], #4
    b zero_loop
zero_done:
    /* Call main */
    bl main
hang_forever:
    b hang_forever

.globl PendSV_Handler
.thumb_func
PendSV_Handler:
    B PendSV_Handler_c

.thumb_func
SysTick_Handler:
    B SysTick_Handler_c