.syntax unified
.cpu cortex-m4
.thumb

.section .vector_table, "a"
.globl g_vector_table
g_vector_table:
  .word _estack          
  .word Reset_Handler    
  .word NMI_Handler     
  .word HardFault_Handler     
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word PendSV_Handler
  .word SysTick_Handler
.thumb_func
NMI_Handler:
  b NMI_Handler  
.section .text.Reset_Handler
.globl Reset_Handler
.thumb_func
Reset_Handler:
  
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

  
  ldr r0, =_sbss
  ldr r1, =_ebss
  mov r2, #0
zero_loop:
  cmp r0, r1
  bge zero_done
  str r2, [r0], #4
  b zero_loop
zero_done:
  bl main
hang_forever:
  b hang_forever  
.global PendSV_Handler
.thumb_func
PendSV_Handler:
  B PendSV_Handler_c  
.thumb_func
HardFault_Handler:
  b HardFault_Handler
