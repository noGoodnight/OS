%include 'functions.asm'

SECTION .data
msgInput db 'Please input x and y: ', 0h
msgRes db 'The result of num1 multiplier num2 is: ', 0h
space db ' ', 0h

SECTION .bss
nums resb 44
num1 resb 22
num2 resb 22
tmp resb 22
sum resb 44

SECTION .text
global _start


;------------------------------------------
_start:
	mov eax, msgInput
	call sprint

	mov edx, 44
	mov ecx, nums
	mov ebx, 0
	mov eax, 3
	int 80h

	mov eax, num1
	mov ebx, num2
	call splitNum

	cmp byte[num1], 30h
	jz ZERO
	cmp byte[num2], 30h
	jz ZERO

	mov eax, num1
	mov ebx, tmp
	call reverseNum
	mov eax, num2
	mov ebx, tmp
	call reverseNum

	; mov eax, num1
	; call sprint
	; mov eax, space
	; call sprint
	; mov eax, num2
	; call sprintLF

	mov ebx, num1
	mov ecx, num2
	mov ebp, sum
	call bigNumMul
	
	; mov eax, sum
	; call reverseNum

	mov eax, msgRes
	call sprint
	mov eax, sum
	call sprintLF

	call quit

ZERO:
	mov eax, msgRes
	call sprint
	mov eax, 0
	call iprintLF

	call quit
