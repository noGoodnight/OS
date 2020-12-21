
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               syscall.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                     Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include "sconst.inc"

extern semaphore_p
extern semaphore_v
extern p_sleep

_NR_get_ticks       equ 0 ; 要跟 global.c 中 sys_call_table 的定义相对应！
INT_VECTOR_SYS_CALL equ 0x90

_NR_print_str equ 1
INT_VECTOR_SYS_PRINT equ 0x91

_NR_sem_p equ 2
_NR_sem_v equ 3
_NR_sleep equ 4

; 导出符号
global	get_ticks
global print_str
global sem_p
global sem_v
global sys_sem_p
global sys_sem_v
global sleep
global sys_sleep

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

sem_p:
    mov eax, _NR_sem_p
    push ebx
    mov ebx, [esp+8]
    int INT_VECTOR_SYS_CALL
    pop ebx
    ret

sem_v:
    mov eax, _NR_sem_v
    push ebx
    mov ebx, [esp+8]
    int INT_VECTOR_SYS_CALL
    pop ebx
    ret

sys_sem_p:
    push ebx
    call semaphore_p
    pop ebx
    ret

sys_sem_v:
    push ebx
    call semaphore_v
    pop ebx
    ret

sleep:
    mov eax, _NR_sleep
    push ebx
    mov ebx, [esp+8]
    int INT_VECTOR_SYS_CALL
    pop ebx
    ret

sys_sleep:
    push ebx
	call p_sleep
	pop ebx
	ret
