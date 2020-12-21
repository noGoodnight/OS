
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               syscall.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                     Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include "sconst.inc"

extern semaphore_v

_NR_get_ticks       equ 0 ; 要跟 global.c 中 sys_call_table 的定义相对应！
INT_VECTOR_SYS_CALL equ 0x90

_NR_print_str equ 1
INT_VECTOR_SYS_PRINT equ 0x91

_NR_sleep equ 2
INT_VECTOR_SYS_SLEEP equ 0x92

_NR_sem_p equ 3
INT_VECTOR_SYS_SEM_P equ 0x93

_NR_sem_v equ 4
INT_VECTOR_SYS_SEM_V equ 0x94

; 导出符号
global	get_ticks
global  print_str
global  sleep
global  sem_p
global  sem_v
global  sys_sem_v

bits 32
[section .text]

; ====================================================================
;                              get_ticks
; ====================================================================
get_ticks:
	mov	eax, _NR_get_ticks
	int	INT_VECTOR_SYS_CALL
	ret

print_str:
    mov eax, _NR_print_str
    int INT_VECTOR_SYS_PRINT
	ret

sleep:
    mov eax, _NR_sleep
	push ebx
	mov ebx, [esp+8]
	int INT_VECTOR_SYS_SLEEP
	pop ebx
	ret

sem_p:
	mov eax, _NR_sem_p
	push ebx
	mov ebx, [esp+8]
	int INT_VECTOR_SYS_SEM_P
	pop ebx
	ret

sem_v:
	mov eax, _NR_sem_v
	push ebx
	mov ebx, [esp+8]
	int INT_VECTOR_SYS_CALL
	pop ebx
	ret

sys_sem_v:
	push ebx
	call semaphore_v
	pop ebx
	ret