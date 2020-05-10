; -------------------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------------------
; Quick & Dirty PCI Access functions
; (Use nasm to assemble it)
; May-2020 Jean-François DEL NERO / HxC2001
; -------------------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------------------
    org 100h

    mov     ah, 09
    mov     dx, message
    int     21h                   ; Fonction DOS 9h : Affichage d'un message

    mov     ax,01022h             ; VENDOR ID
    mov     bx,02000h             ; DEVICE ID

    call    find_PCI_device       ; Recherche du periph PCI et récupération de sa position
    jc      not_found

    push    bx

    mov     ah, 09
    mov     dx, message_found
    int     21h                   ; Fonction DOS 9h : Affichage d'un message

    pop     ax

    ; Lecture et affichage de quelques registres PCI

    push    ax
    mov     ah, 09
    mov     dx, message_read
    int     21h                   ; Fonction DOS 9h : Affichage d'un message
    pop     ax

    mov     bl,10h                ; premier registre

    call    read_pci_reg
    call    affiche_hex
    inc     bl
    call    read_pci_reg
    call    affiche_hex
    inc     bl
    call    read_pci_reg
    call    affiche_hex
    inc     bl
    call    read_pci_reg
    call    affiche_hex


    ; Ecriture de quelques registres PCI

    mov     bl,10h                ; premier registre

    mov     cl,31h
    call    write_pci_reg
    inc     bl
    mov     cl,32h
    call    write_pci_reg
    inc     bl
    mov     cl,33h
    call    write_pci_reg
    inc     bl
    mov     cl,34h
    call    write_pci_reg

    push    ax
    mov     ah, 09
    mov     dx, message_modified
    int     21h                   ; Fonction DOS 9h : Affichage d'un message
    pop     ax

    ; Lecture et affichage de quelques registres PCI

    mov     bl,10h                ; premier registre

    call    read_pci_reg
    call    affiche_hex
    inc     bl
    call    read_pci_reg
    call    affiche_hex
    inc     bl
    call    read_pci_reg
    call    affiche_hex
    inc     bl
    call    read_pci_reg
    call    affiche_hex

    ret                           ; Fin du programme

not_found:
    mov     ah, 09h
    mov     dx, message_not_found
    int     21h                   ; Fonction DOS 9h : Affichage d'un message

    ret                           ; Fin du programme

; -------------------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------------------

; -------------------------------------------------------------------------------------------------
; affiche_hex
; Fonction d'affichage hexa
; CL = Valeur a afficher
; Pas de registre modifié
; -------------------------------------------------------------------------------------------------

affiche_hex:

    pusha

    push    cx

    mov     ax,cx

    shr     al,4

    cmp     al,09h
    jna     underA

    sub     al,0AH
    add     al,'A'-'0'

underA:

    add     al,'0'
    mov     dl,al
    mov     ah,2
    int     21h                   ; Fonction DOS 2h : Affichage d'un caractère

    pop     ax

    and     al,0Fh

    cmp     al,09h
    jna     underA2

    sub     al,0AH
    add     al,'A'-'0'

underA2:

    add     al,'0'
    mov     dl,al
    mov     ah,2
    int     21h                   ; Fonction DOS 2h : Affichage d'un caractère

    popa

    ret

; -------------------------------------------------------------------------------------------------
; delay
; Fonction de delay
; CX = nb de loop
; Pas de registre modifié
; -------------------------------------------------------------------------------------------------
delay:

    pusha
    mov ax,0

loop_delay:

    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    add     ax,1
    jnz     loop_delay
    dec     cx
    jnz     loop_delay

    popa
    ret

; -------------------------------------------------------------------------------------------------
; affiche_car
; Fonction d'affichage caractère
; CL = caractère a afficher
; Pas de registre modifié
; -------------------------------------------------------------------------------------------------
affiche_car:

    pusha

    mov dl,cl
    mov ah,2
    int 21h                       ; Fonction DOS 2h : Affichage d'un caractère

    popa

    ret

; -------------------------------------------------------------------------------------------------
; read_pci_reg
; Fonction de lecture d'un registre PCI
; AX = bus/device
; BL = registre
; Retour -> CL
; -------------------------------------------------------------------------------------------------

read_pci_reg:
    push    ax
    push    dx
    push    bx
    and     eax, 0FFFFh
    shl     eax, 8
    or      eax, 80000000h
    or      al, bl
    mov     bh, 0
    and     bl, 03h
    mov     dx, 0CF8h
    out     dx, eax
    mov     dx, 0CFCh
    or      dx, bx
    in      al, dx
    mov     cl, al
    pop     bx
    pop     dx
    pop     ax

    ret

; -------------------------------------------------------------------------------------------------
; read32bits_pci_reg
; Fonction de lecture d'un registre PCI (32bits)
; BX = bus/device
; AL = registre
; Retour -> EAX
; -------------------------------------------------------------------------------------------------

read32bits_pci_reg:
    mov     cl, al
    and     cl, 11b               ; store offset within double word
    and     al, ~0x3              ; align address to double word
    mov     dx, bx
    shl     edx, 8
    mov     dl, al
    mov     eax, 80000000h
    or      eax, edx
    mov     dx, 0CF8h
    out     dx, eax               ; map double word at 0CFCh
    mov     dx, 0CFCh
    add     dl, cl                ; calculate address of mapped register
    in      eax, dx
    ret

; -------------------------------------------------------------------------------------------------
; write_pci_reg
; Fonction d'écriture d'un registre PCI
; AX = bus/device
; BL = registre
; CL = valeur
; -------------------------------------------------------------------------------------------------

write_pci_reg:
    push    ax
    push    dx
    push    bx
    push    bx

    and     eax, 0FFFFh
    shl     eax, 8
    or      eax, 80000000h

    or      al, bl
    mov     bh, 0
    and     bl, 3h
    mov     dx, 0CF8h
    out     dx, eax
    mov     dx, 0CFCh
    or      dx, bx
    pop     ax

    mov     al, cl
    out     dx, al

    pop     bx
    pop     dx
    pop     ax

    ret

; -------------------------------------------------------------------------------------------------
; find_PCI_device
; Fonction de recherche d'un périph PCI
; AX = vendor ID
; BX = device ID
; Retour -> BH = PCI bus, BL = PCI device/function
; Si pas trouvé carry flag set
; -------------------------------------------------------------------------------------------------

find_PCI_device:
    shl     ebx,16
    mov     bx,ax
    mov     ebp,ebx
    mov     bh,0                  ; scan bus 0 first
scan_PCI_bus:
    mov     bl,0                  ; begin from device 0, function 0
try_PCI_device:
    mov     ax,0
    call    read32bits_pci_reg
    cmp     eax,ebp
    je      PCI_device_found
    inc     bl
    jnz     try_PCI_device
    inc     bh
    cmp     bh,16                 ; try 16 buses (change if needed)
    jne     scan_PCI_bus
    stc                           ; device not found
    ret
PCI_device_found:
    clc
    ret

message           db "PCI access test",10,13,'$'
message_found     db "device found ! :)",10,13,'$'
message_not_found db "device not found :( !",10,13,'$'
message_read      db 10,13,"Read     regs: ",'$'
message_modified  db 10,13,"Modified regs: ",'$'
