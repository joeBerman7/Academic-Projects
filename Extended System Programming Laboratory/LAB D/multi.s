section .data
    state: dw 0xACE1
section .bss
    buffer resb 600
section .rodata
    mask: dw 0x002D
    x_struct: dw 5
        x_num: dw 0xaa, 1,2,0x44,0x4f
    y_struct: dw 6
        y_num: dw 0xaa, 1,2,3,0x44,0x4f
    print_format: db "%04hx", 0
    format: db "%04hx", 10, 0
    newline: db 10, 0
section .text
global main
extern printf
extern puts
extern fgets
extern calloc
extern free
extern stdin
extern strlen
main:
    push ebp
    mov ebp, esp
    mov eax, [ebp+8] ; argc
    cmp eax, 1
    je default_print
    mov eax, [ebp+12] ; argv
    add eax, 4          ; argv + 1
    mov eax, [eax]      ; argv[1]
    cmp word[eax], "-I"
    je run_getmulti
    cmp word[eax], "-R"
    je run_prmulti
end_program:
    pop ebp
    ret

default_print:
    push x_struct
    call print_multi
    push y_struct
    call print_multi
    call add_multi
    push eax
    call print_multi
    call free
    add esp, 12
    jmp end_program

run_getmulti:
    call getmulti
    push eax
    call getmulti
    push eax
    call add_multi
    push eax
    call print_multi
    call free
    add esp, 4
    call free
    add esp, 4
    call free
    add esp, 4
    jmp end_program

run_prmulti:
    call PRMulti
    push eax
    call print_multi
    call PRMulti
    push eax
    call print_multi
    call add_multi
    push eax
    call print_multi
    call free
    add esp, 4
    call free
    add esp, 4
    call free
    add esp, 4
    jmp end_program
print_multi:
    push ebp
    mov ebp, esp
    mov edi, [ebp+8]
    mov esi, edi
    add esi, 2
    mov eax, 0
    mov ax, word[edi]
    mov edi, eax

    loop_print:
        push dword[esi+edi*2-2]
        push print_format
        call printf
        add esp, 8
        dec edi
        jne loop_print
    push newline
    call printf
    add esp, 4
    pop ebp
    ret

getmulti:
    push ebp
    mov ebp, esp
    
    ; get input using fgets
    push dword[stdin]
    push 600
    push buffer
    call fgets
    add esp, 12
    ; get sisze of the buffer
    push buffer
    call strlen
    add esp, 4
    mov esi, eax
    ; calculate how many bytes we have in the number
    mov edi, eax
    shr edi, 1
    add edi, 2
    mov edx, 1
    and edx, edi
    cmp edx, 0
    je skip_add
    add edi, 1
    skip_add:
    ;calloc the required amount of bytes
    push 1
    push edi
    call calloc
    add esp, 8
    ; save the size of the struct in the first two bytes
    sub edi, 2
    shr edi, 1
    mov dword[eax], edi
    ; loop over the input string and turn it into bytes
    mov ebx, buffer
    add ebx, esi
    sub ebx, 2 ; ignore newline
    mov edx, eax
    add edx, 2
    dec esi             ;remove new line
    loop_parse:
        mov ecx, 0
        mov cl, [ebx]
        dec ebx
        mov ch, [ebx]
        dec ebx
        cmp cl, '9'
        jg letter1
        sub cl, '0'
        jmp skip_letter1
        letter1:
        sub cl, 'a'
        add cl, 10
        skip_letter1:
        cmp ch, '9'
        jg letter2
        sub ch, '0'
        jmp skip_letter2
        letter2:
        sub ch, 'a'
        add ch, 10
        skip_letter2:
        shl ch, 4
        or cl, ch
        mov byte[edx], cl
        inc edx
        sub esi, 2
        cmp esi, 0
        jg loop_parse
    pop ebp
    ret


Get_MaxMin:
    mov cx, [eax]
    mov dx, [ebx]
    cmp cx, dx
    jge skip_change
    mov ecx, eax
    mov eax, ebx
    mov ebx, ecx
    skip_change:
    ret

add_multi:
    push ebp
    mov ebp, esp
    mov eax, [ebp+8]
    mov ebx, [ebp+12]
    call Get_MaxMin
    mov esi, eax
    mov edi, ebx
    mov eax, 0
    mov ax, [esi]
    shl eax, 1
    add eax, 4
    push 1
    push eax
    call calloc
    pop ebx
    add esp, 4
    sub ebx, 2
    shr ebx, 1
    mov word[eax], bx
    mov ebx, 0
    mov ecx, 0
    mov edx, 0
    push eax
    loop_add:
        add eax, 2
        mov dx, cx
        mov cx, [esi + 2 + ebx * 2]
        add ecx, edx ; add the carry
        mov dx, [edi + 2 + ebx * 2]
        add ecx, edx
        mov word[eax], cx
        shr ecx, 16
        inc bx
        cmp bx, word[edi]
        jl loop_add

    ; check if both have the same length
    cmp bx, word[esi]
    je skip_add_2

    loop_add_2:
        add eax, 2
        mov dx, cx
        mov cx, [esi + 2 + ebx * 2]
        add ecx, edx ; add the carry
        mov word[eax], cx
        shr ecx, 16
        inc bx
        cmp bx, word[esi]
        jl loop_add_2
skip_add_2:

    pop eax
    pop ebp
    ret

rand_num:
    push ebp
    mov ebp, esp

    mov eax, 0
    mov ax, word[state]
    mov ebx, 0
    mov ecx, 0

    and ax, word[mask]
    jnp skip_bit
    mov ebx, 1
    skip_bit:
    shr word[state], 1
    shl ebx, 15
    or word[state], bx
    mov eax, 0
    mov ax, word[state]
    pop ebp
    ret

PRMulti:
    push ebp
    mov ebp, esp
    generate_random:
        call rand_num
        mov ah, 0
        cmp eax, 0
        je generate_random
    mov edi, eax
    shl eax, 1
    add eax, 2
    push 1
    push eax
    call calloc
    add esp, 8
    mov esi, eax
    mov ebx, edi
    mov word[eax], bx
    mov edx, 2
    loop_prmulti:
        call rand_num
        mov word[esi + edx], ax
        add edx, 2
        dec edi
        cmp edi, 0
        jne loop_prmulti
    mov eax, esi
    pop ebp
    ret