WRITE EQU 4
READ EQU 3
EXIT EQU 1
OPEN_CREATE EQU 8
OPEN EQU 5
STDOUT EQU 1
global main
global _start
extern strlen
section .data
    newline db 0x0A
    outfile dd 1
    infile dd 0
    d_string db ""
section .text
global _start
global system_call
_start:
    pop    dword ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    ;; lea eax, [esi+4*ecx+4] ; eax = envp = (4*ecx)+esi+4
    mov     eax,ecx     ; put the number of arguments into eax
    shl     eax,2       ; compute the size of argv in bytes
    add     eax,esi     ; add the size to the address of argv 
    add     eax,4       ; skip NULL at the end of argv
    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc

    call    main        ; int main( int argc, char *argv[], char *envp[] )

    mov     ebx,eax
    mov     eax,1
    int     0x80
    nop
        
system_call:
    push    ebp             ; Save caller state
    mov     ebp, esp
    sub     esp, 4          ; Leave space for local var on stack
    pushad                  ; Save some more caller state

    mov     eax, [ebp+8]    ; Copy function args to registers: leftmost...        
    mov     ebx, [ebp+12]   ; Next argument...
    mov     ecx, [ebp+16]   ; Next argument...
    mov     edx, [ebp+20]   ; Next argument...
    int     0x80            ; Transfer control to operating system
    mov     [ebp-4], eax    ; Save returned value...
    popad                   ; Restore caller state (registers)
    mov     eax, [ebp-4]    ; place returned value where caller can see it
    add     esp, 4          ; Restore caller state
    pop     ebp             ; Restore caller state
    ret                     ; Back to caller


main: push ebp
    mov ebp, esp
    push ebx
    mov ebx, 0
    Next: 
        mov esi, [ebp+12]
        mov esi, [esi+ebx*4]
        push esi
        call strlen
        add esp, 4
        mov edx, eax
        push ebx
        mov eax, WRITE
        mov ebx, STDOUT
        mov ecx, esi
        int 0x80
        mov eax, WRITE
        mov ecx, newline
        mov edx, 1
        int 0x80
        cmp word [esi], "-i"
        jnz go_on1
        add esi, 2
        ;mov infile, 
        mov ebx, esi
        mov ecx, 0
        mov eax, OPEN
        int 0x80
        mov [infile], eax
        go_on1:
            cmp word [esi], "-o"
            jnz go_on2
            add esi, 2
            mov ebx, esi
            mov ecx, 1
            or ecx, 64
            mov eax, OPEN
            push edx
            mov edx, 0777
            int 0x80
            mov [outfile], eax
            pop edx
            go_on2:
                pop ebx
                mov ecx, [ebp+12]
                inc ebx
                mov edx, [ebp+8]
                cmp ebx, [ebp+8]
                jnz Next

    call encode
    mov eax, EXIT
    mov ebx, 0
    int 0x80

encode:

    mov eax, READ
    mov ecx, d_string
    mov ebx, [infile]
    mov edx, 1
    int 0x80
    
    cmp dword [ecx], 0
    jz go_on
    Next2:
        
        cmp byte [ecx], 122
        jg continue1
        cmp byte [ecx], 97
        jge continue2
        cmp byte [ecx], 90
        jg continue1
        cmp byte [ecx], 65
        jl continue1

        continue2:
            add byte [ecx], 1

        continue1:
            
            mov edx, eax
            mov eax, WRITE
            mov ebx, [outfile]
            int 0x80

            mov eax, READ
            mov ebx, [infile]
            mov edx, 1
            mov eax, READ
            mov dword [d_string], 0
            mov ecx, d_string
            int 0x80
            cmp dword [ecx], ""
            jnz Next2
    
    go_on: ret