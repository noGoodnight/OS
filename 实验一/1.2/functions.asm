SECTION .bss
tmp1 resb 44
tmp2 resb 44
tmp3 resb 44
tmpAdd resb 44

SECTION .text

;------------------------------------------
; void iprint(Integer number)
; Integer printing function (itoa)
iprint:
    push    eax
    push    ecx
    push    edx
    push    esi
    mov     ecx, 0

divideLoop:
    inc     ecx
    mov     edx, 0
    mov     esi, 10
    idiv    esi
    add     edx, 48
    push    edx
    cmp     eax, 0
    jnz     divideLoop

printLoop:
    dec     ecx
    mov     eax, esp
    call    sprint
    pop     eax
    cmp     ecx, 0
    jnz     printLoop

    pop     esi
    pop     edx
    pop     ecx
    pop     eax
    ret
 
 
;------------------------------------------
; void iprintLF(Integer number)
; Integer printing function with linefeed (itoa)
iprintLF:
    call    iprint

    push    eax
    mov     eax, 0Ah
    push    eax
    mov     eax, esp
    call    sprint
    pop     eax
    pop     eax
    ret


;------------------------------------------
; int slen(String message)
; String length calculation function
slen:
    push    ebx
    mov     ebx, eax

nextchar:
    cmp     byte [eax], 0
    jz      finished
    inc     eax
    jmp     nextchar
 
finished:
    sub     eax, ebx
    pop     ebx
    ret                     


;------------------------------------------
; void sprint(String message)
; String printing function
sprint:
    push    edx
    push    ecx
    push    ebx
    push    eax
    call    slen

    mov     edx, eax
    pop     eax

    mov     ecx, eax
    mov     ebx, 1
    mov     eax, 4
    int     80h

    pop     ebx
    pop     ecx
    pop     edx
    ret


;------------------------------------------
; void sprintLF(String message)
; String printing with line feed function
sprintLF:
    call    sprint

    push    eax
    mov     eax, 0Ah
    push    eax
    mov     eax, esp
    call    sprint
    pop     eax
    pop     eax
    ret


;------------------------------------------
; void exit()
; Exit program and restore resources
quit:
    mov     ebx, 0
    mov     eax, 1
    int     80h
    ret


;------------------------------------------
; void splitNum()
; get num1 and num2 from nums
splitNum:
    push eax
    push ebx
    push ecx
    push edx
    
firstChar:
    cmp byte[ecx], 20h
    jz ignoreSpace
    mov edx, [ecx]
    mov [eax], edx
    inc ecx 
    inc eax
    jmp firstChar

ignoreSpace:
    inc ecx
    mov dword[eax], 0

secondChar:
    cmp byte[ecx], 0Ah
    jz finished_1
    mov edx, [ecx]
    mov [ebx], edx
    inc ecx
    inc ebx
    jmp secondChar

finished_1:
    mov dword[ebx], 0

    pop edx
    pop ecx
    pop ebx 
    pop eax
    ret


;------------------------------------------
; void reverseNum()
; ecx == eax
; reverse from ecx to ebx
; copy from ebx to ecx
reverseNum:
    push eax
    push ebx
    push ecx
    push edx

    mov ecx, eax
    mov edx,  ecx
    call slen 
    add ecx, eax
    push eax ;-->>stack 
    dec ecx 
    mov eax, edx 

reverseChar:
    cmp ecx, eax
    jz processed_2
    mov dl, byte[ecx]
    mov byte[ebx], dl
    inc ebx
    dec ecx
    jmp reverseChar
    
processed_2:
    mov dl, byte[ecx]
    mov byte[ebx], dl
    inc ebx
    mov byte[ebx], 0h
    pop eax ;<<--stack
    sub ebx, eax

copyChar:
    cmp byte[ebx], 0h
    jz finished_2
    mov dl, byte[ebx]
    mov byte[ecx], dl
    inc ebx
    inc ecx
    jmp copyChar

 finished_2:
    mov ecx, 0h
    sub ebx, eax
    sub ecx, eax

    pop edx
    pop ecx
    pop ebx
    pop eax
    ret


;------------------------------------------
; int add(int, int)
bigNumAdd:
    push eax
    push ebx
    push ecx
    push edx
    push esi
    push ebp
    mov esi, 0h

twoNum:
    cmp byte[ebx], 0h
    jz num_2
    cmp byte[ecx], 0h
    jz num_1

    mov eax, 0h
    mov al, byte[ebx]
    sub eax, 30h
    ; call iprintLF
    mov edx, 0h
    mov dl, byte[ecx]
    sub edx, 30h

    add eax, edx
    add eax, esi
    ; call iprintLF
    mov edx, 10
    div dl
    mov dl, ah
    mov ah, 0h
    ; call iprintLF
    mov esi, eax
    mov eax, edx
    ; call iprintLF
    ; push eax
    ; mov eax, edx
    ; call iprintLF
    ; pop eax

    add dl, 30h
    mov byte[ebp], dl
    inc ebp
    inc ebx
    inc ecx
    jmp twoNum
    
num_1:
    ; mov eax, num1
    ; call sprintLF
    cmp byte[ebx], 0h
    jz finished_3

    mov eax, 0h
    mov al, byte[ebx]
    sub eax, 30h
    ; call iprintLF
    mov edx, 0h

    add eax,  edx
    add eax,  esi
    ; call iprintLF
    mov edx, 10
    div dl
    mov dl, ah
    mov ah, 0h
    ; call iprintLF
    mov esi, eax
    mov eax, edx
    ; call iprintLF
    add dl, 30h
    mov byte[ebp], dl
    inc ebp
    inc ebx
    jmp num_1

num_2:
    ; mov eax, num2
    ; call sprintLF
    cmp byte[ecx], 0h
    jz finished_3
    
    mov eax, 0h
    ; call iprintLF
    mov edx, 0h
    mov dl, byte[ecx]
    sub edx, 30h

    add eax,  edx
    add eax,  esi
    ; call iprintLF
    mov edx, 10
    div dl
    mov dl, ah
    mov ah, 0h
    ; call iprintLF
    mov esi, eax
    mov eax, edx
    ; call iprintLF
    add dl, 30h
    mov byte[ebp], dl
    inc ebp
    inc ecx
    jmp num_2

finished_3:
    mov eax, esi
    cmp eax, 0h
    jz finished_carry
    ; call iprintLF
    add eax, 30h
    mov byte[ebp], al
    inc ebp

finished_carry:
    mov byte[ebp], 0h
    pop ebp
    pop esi
    pop edx
    pop ecx
    pop ebx
    pop edx
    ret


;------------------------------------------
; int mul(int, int)
bigNumMul:
    push eax
    push ebx
    push ecx
    push edx
    push edi
    push esi

    mov byte[ebp], '0'

    mov eax, ecx
    call slen
    push eax
    mov eax, ebx
    call slen
    push eax

    mov edi, 0h
    mov esi, 0h

first_loop:
    pop eax
    cmp edi, eax
    push eax
    jz finished_4
second_loop:
    pop eax
    pop edx
    cmp esi, edx
    push edx
    push eax
    jz to_outside

    push ebp
    mov eax, tmp1
copy_tmp1:
    cmp byte[ebp], 0h
    jz processed_3
    mov dl, byte[ebp]
    mov byte[eax], dl
    inc ebp
    inc eax
    jmp copy_tmp1
processed_3:
    mov byte[eax], 0h
    pop ebp

    push ebp
    mov ebp, tmp2
    
    add ebx, edi
    add ecx, esi
    mov eax, 0h
    mov al, byte[ebx]
    sub al, 30h
    mov dl, byte[ecx]
    sub dl, 30h
    mul dl
    mov dh, 10
copy_tmp2_1:
    div dh
    mov dl, ah
    mov ah, 0h
    cmp al, 0
    jz copy_tmp2_2
    add dl, 30h
    mov byte[ebp], dl
    inc ebp
    jmp copy_tmp2_1
copy_tmp2_2:
    add dl, 30h
    mov byte[ebp], dl
    inc ebp
    mov byte[ebp], 0h

    push eax
    push ebx
    mov eax, tmp2
    mov ebx, tmp3
    call reverseNum
    pop ebx
    pop eax

    mov eax, edi
    mov edx, esi
    add eax, edx

add_zero: 
    cmp eax, 0
    jz processed_4
    mov byte[ebp], 30h
    inc ebp
    dec eax
    jmp add_zero

processed_4:
    mov byte[ebp], 0h

    pop ebp
    sub ebx, edi
    sub ecx, esi

    push ebx
    push ecx
    push ebp
    mov eax, tmp1
    mov ebx, tmp3
    call reverseNum
    mov eax, tmp2
    mov ebx, tmp3
    call reverseNum
    mov eax, tmp1
    ; call sprintLF
    mov eax, tmp2
    ; call sprintLF
    mov ebx, tmp1
    mov ecx, tmp2
    mov ebp, tmpAdd
    call bigNumAdd
    mov eax, tmpAdd
    mov ebx, tmp3
    call reverseNum
    ; call sprintLF
    pop ebp
    pop ecx
    pop ebx

    mov eax, tmpAdd
    mov edx, eax
    call slen
    ; call iprintLF

copyBack:
    cmp byte[edx], 0
    jz end_loop_2
    mov ah, byte[edx]
    mov byte[ebp], ah
    inc edx
    inc ebp
    jmp copyBack

end_loop_2:
    mov ah, 0h
    sub ebp, eax

    inc esi
    jmp second_loop 
to_outside:
    mov esi, 0h
    inc edi
    jmp first_loop

finished_4:
    pop eax
    pop eax
    pop esi
    pop edi
    pop edx
    pop ecx
    pop ebx
    pop eax
    ret
