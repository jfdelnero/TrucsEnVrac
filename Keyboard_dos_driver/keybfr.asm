; -----------------------------------------------------------------------------
; -----------------------------------------------------------------------------
; French Keyboard DOS driver / Driver clavier franÁais
; Minimum requirements : A 8086/8088 PC and ~3KB of free memory.
; (Use nasm to assemble it:
;  nasm -f bin keybfr.asm -o keybfr.com)
;
; Original release : SMT Goupil 28 June 88
;
; October 2020 -  Disassembled source code (WIP)
;
; Jean-FranÁois DEL NERO / HxC2001
; -----------------------------------------------------------------------------
; -----------------------------------------------------------------------------

    org 100h
    bits 16

BIOS_KEYBOARD_SHIFT_STATUS                equ 17h
BIOS_KEYBOARD_EXTENDED_SHIFT_STATUS       equ 18h
BIOS_ALTERNATE_KEYBOARD_ENTRY             equ 19h
BIOS_KEYBOARD_BUFFER_HEAD_POINTER         equ 1Ah
BIOS_KEYBOARD_BUFFER_TAIL_POINTER         equ 1Ch
BIOS_BREAK_KEY_STATE                      equ 71h
BIOS_RESET_STATE_FLAG                     equ 72h
BIOS_KEYBOARD_BUFFER_START_OFFSET_POINTER equ 80h
BIOS_KEYBOARD_BUFFER_END_OFFSET_POINTER   equ 82h
BIOS_KEYBOARD_STATUS_AND_TYPE             equ 96h
BIOS_KEYBOARD_LED_STATUS                  equ 97h

start:
    jmp entry_point

; -----------------------------------------------------------------------------
;
; Keyboard ISR
;
; -----------------------------------------------------------------------------

isr_keyboard:
    sti

    push    ax
    push    cx
    push    dx
    push    bx
    push    bp
    push    si
    push    di
    push    ds

    ; ds = Bios data structure segment (0040)
    mov ax, 40h
    mov ds, ax

    mov ah, 0ADh    ; Disable Keyboard Interface
    call    Send_8042_command

    in      al, 60h         ; AT Keyboard controller 8042.
    mov     dl, al
    in      al, 61h         ; PC/XT PPI port B bits:
    mov     ah, al
    or      al, 80h         ; 7: 1=disable kbrd
    out     61h, al         ; PC/XT PPI port B bits:
    xchg    al, ah
    out     61h, al         ; PC/XT PPI port B bits:

    mov     al, dl          ; al <- scan code

    stc
    mov ah, 4Fh
    int 15h     ; OS HOOK - KEYBOARD INTERCEPT
                ; AL = scan code, CF set
                ; Return: CF set - AL = scan code
                ; CF clear - scan code should be ignored

    jnb     jmp0_enable_kb_leave_kb_isr   ; If CF = 0 -> scan code removed -> exit

    cmp     al, 0FAh            ; Keyboard acknowledge to keyboard commands ?
    jz      loc_148+1

    cmp     al, 0FEh            ; Resend ? Keyboard requests the system to resend the last command.
    jz      set_resend_flag_and_leave_isr

    cmp     al, 0FFh            ; Keyboard reset ?
    jz      loc_152

    cmp     al, 0E0h            ; E0 scancode ?
    jz      e0_e1_scancode

    cmp     al, 0E1h            ; E1 scancode ?
    jz      e0_e1_scancode

    jmp     loc_174

    nop

set_resend_flag_and_leave_isr:
    mov al, 20h      ; Resend receive flag

loc_148:
    cmp ax, 10B0h
    or  byte [BIOS_KEYBOARD_LED_STATUS],    al

jmp0_enable_kb_leave_kb_isr:
    jmp enable_kb_leave_kb_isr

; -----------------------------------------------------------------------------

loc_152:
    and byte [BIOS_KEYBOARD_SHIFT_STATUS], 0F0h          ; CTRL pressed, ALT pressed flags cleared.
    and byte [BIOS_KEYBOARD_EXTENDED_SHIFT_STATUS], 0FCh ; Right CTRL pressed, Left ALT pressed flags cleared.
    and byte [BIOS_KEYBOARD_STATUS_AND_TYPE], 0F3h       ; Right CTRL pressed, Right ALT key pressed flags cleared.
    jmp loc_200
; -----------------------------------------------------------------------------

e0_e1_scancode:
    mov ah, 12h
    and al, 1
    sub ah, al
    or  [BIOS_KEYBOARD_STATUS_AND_TYPE],    ah           ; Set the E0 or E1 flag

loc_16E:
    call    enable_keyboard
    jmp immediat_leave_kb_isr
; -----------------------------------------------------------------------------

loc_174:
    and ax, 7Fh
    jz  short loc_17D            ; -> scancode 0 -> leave
                                 ; (Key detection error or internal buffer overrun)

    cmp al, 58h                  ; scancode <= 0x58 key press event.
    jbe short key_pressed_scancode

loc_17D:
    jmp loc_200

; -----------------------------------------------------------------------------

key_pressed_scancode:
        ; ax < 0x58
    push    sp
    pop bx
    cmp bx, sp
    jz  short loc_19E

    test    byte [BIOS_BREAK_KEY_STATE], 40h
    jz  short loc_19E

    test    byte [BIOS_KEYBOARD_STATUS_AND_TYPE], 10h
    jnz short loc_19E

    cmp al, 29h        ; ` (back tick) pressed
    jz  short loc_19C

    cmp al, 2Bh        ; back slash pressed
    jnz short loc_19E

loc_19C:
    xor al, 2          ; exchange back tick and back slash key

loc_19E:
    mov di, ax        ; ax < 0x58
    shl di, 1         ; di = ax * 2
    test    byte [BIOS_BREAK_KEY_STATE], 40h

    jz  short loc_1AD
    add di, array_2_710_1A2 - (scancode_to_function_array - 2)

loc_1AD:
    mov bx, [cs:di+scancode_to_function_array - 2]
    xchg    ah, bh             ; bh <> ah = (bh = 0 ah=bh)
    mov si, func_pointer_array
    shl bl, 1                  ; bl = function number
    jb  short loc_1DD          ; bl = bl * 2 -> bx = function pointer address.
    or  dl, dl
    js  short enable_kb_leave_kb_isr
    test    byte [BIOS_KEYBOARD_EXTENDED_SHIFT_STATUS], 8
    jnz short loc_1E6

loc_1C6:
    add  si, bx
    mov  dx, [BIOS_KEYBOARD_SHIFT_STATUS]
    mov  bl, dl
    and  bl, 0Fh
    mov  cl, bl
    mov  bl, [cs:bx + array_0afa]
    xchg al, ah               ; al = scan code
                              ; ah = function param
                              ; bl = <0 - 3>
                              ;      1 = shift key pressed
                              ;      2 = shift + ctrl
                              ;      3 = shift + ctrl + alt
                              ; cl = Right SHIFT,Left SHIFT pressed
                              ;      CTRL pressed, ALT pressed status
                              ; dl = Keyboard Shift Status
                              ; dh = Keyboard Extended Shift Status
    jmp word [cs:si]
; -----------------------------------------------------------------------------

loc_1DD:
    or  dl, dl
    jns short loc_1C6
    mov si, func_pointer_array_2
    jmp short loc_1C6
; -----------------------------------------------------------------------------

loc_1E6:
    cmp al, 45h
    jz  short enable_kb_leave_kb_isr
    and byte [BIOS_KEYBOARD_EXTENDED_SHIFT_STATUS], 0F7h

pfunc_enable_kb_leave_kb_isr:
enable_kb_leave_kb_isr:
    call    enable_keyboard

clear_e0e1_leave_kb_isr:
    and byte [BIOS_KEYBOARD_STATUS_AND_TYPE], 0FCh ; Clear E0 & E1 flags

immediat_leave_kb_isr:
    pop ds
    pop di
    pop si
    pop bp
    pop bx
    pop dx
    pop cx
    pop ax
    iret
; -----------------------------------------------------------------------------

loc_200:
    mov al, 20h
    out 20h, al     ; Interrupt controller, 8259A.
    call    Speaker_Beep
    cli
    call    Send_8042_Enable_Keyboard_command
    jmp short clear_e0e1_leave_kb_isr
; -----------------------------------------------------------------------------

loc_20D:
    cli
    mov al, 20h
    out 20h, al     ; Interrupt controller, 8259A.
    push    sp
    pop ax
    cmp ax, sp
    jnz short loc_21E
    call    loc_62C
    call    Send_8042_Enable_Keyboard_command

loc_21E:
    jmp short clear_e0e1_leave_kb_isr
; -----------------------------------------------------------------------------

pfunc_loc_220:
    test    dh, al
    jnz short enable_kb_leave_kb_isr
    cmp cl, 0Ch
    jnb short loc_22E
    xor bl, bl
    jmp short loc_244

    nop

loc_22E:
    xor [BIOS_KEYBOARD_EXTENDED_SHIFT_STATUS],  al
    call    enable_keyboard
    pop ds
    pop di
    pop si
    pop bp
    pop bx
    pop dx
    pop cx
    pop ax
    jmp 0F000h:0E000h
; -----------------------------------------------------------------------------
pfunc_loc_242:
    mov bl, 1
loc_244:
    xor [BIOS_KEYBOARD_EXTENDED_SHIFT_STATUS],  al
    mov bh, 85h
    jmp short loc_277
; -----------------------------------------------------------------------------
    nop
; -----------------------------------------------------------------------------
data_array_24D:
    db 0FFh
    db 0DFh ; ﬂ
    db  1Fh
    db    0
; -----------------------------------------------------------------------------
pfunc_loc_251:
loc_251:
    test    dl, 40h
    jz  short loc_25E
    cmp bl, 1
    ja  short loc_25E
    xor bl, 1

loc_25E:
    and al, [cs:bx+data_array_24D]

Push_scancode:
    mov bl, [BIOS_BREAK_KEY_STATE]
    and bx, 3Fh
    jz  short loc_26F

    call    loc_58E

loc_26F:
    call    Push_scancode_to_keyboard_buffer
    jz  short loc_200
    mov bx, 9102h

loc_277:
    push    sp
    pop ax
    cmp ax, sp
    jz  short loc_27F
    jmp short loc_20D
; -----------------------------------------------------------------------------

loc_27F:
    call    enable_keyboard
    mov ax, bx
    int 15h
    jmp clear_e0e1_leave_kb_isr
; -----------------------------------------------------------------------------
data_array_289:
    db    0
    db  19h
    db  23h ; #
    db  2Dh ; -
unk_28D     db  2Eh ; .
; -----------------------------------------------------------------------------
pfunc_loc_28E:
    cmp cl, 0Ch
    jb  short loc_29F
    and byte [BIOS_BREAK_KEY_STATE], 0BFh
    or  [BIOS_BREAK_KEY_STATE], al
    jmp enable_kb_leave_kb_isr
; -----------------------------------------------------------------------------
pfunc_loc_29F:
loc_29F:
    add ah, [cs:bx+data_array_289]
    jmp loc_32C
; -----------------------------------------------------------------------------
pfunc_loc_2A7:
    shl bx, 1
    add ah, byte [cs:unk_28D]
    add ah, bl
    jmp short loc_32C
; -----------------------------------------------------------------------------
    nop
pfunc_loc_2B3:
    cmp cl, 0Ch
    jnb short loc_305

    cmp bl, 2            ; shift + ctrl ?
    jb  short loc_305    ; no (shift or nothing)

    jnz short loc_2C4    ; shift + ctrl + alt

    ; shift + ctrl
    mov ax, 94F0h
    jmp short Push_scancode
; -----------------------------------------------------------------------------

loc_2C4:
    mov ax, 0A5F0h
    jmp short Push_scancode
; -----------------------------------------------------------------------------
pfunc_loc_2C9:
    test    byte [BIOS_KEYBOARD_STATUS_AND_TYPE], 10h
    jz  short loc_2DD
    test    dl, 40h
    jz  short loc_2DD
    cmp bl, 1
    ja  short loc_2DD
    xor bl, 1

pfunc_loc_2DD:
loc_2DD:
    call    loc_5C7
    js  short loc_2EE

    test    byte [BIOS_KEYBOARD_STATUS_AND_TYPE], 2    ; E0 flag set ?
    jz  short loc_2EB

    mov ah, 0E0h

loc_2EB:
    jmp Push_scancode
; -----------------------------------------------------------------------------

loc_2EE:
    cmp al, 0FFh
    jz  short loc_331
    cmp al, 0FEh
    jz  short loc_32C
    cmp al, 0F0h
    jz  short loc_2EB
    jmp short loc_316
; -----------------------------------------------------------------------------
    nop

loc_2FD:
    test    dl, 20h
    jz  short loc_305
    xor bl, 1

pfunc_loc_305:
loc_305:
    call    loc_5C7
    jns short loc_2EB

loc_30A:
    cmp al, 0FFh
    jz  short loc_331
    cmp al, 0FEh
    jz  short loc_323
    cmp al, 0F0h
    jz  short loc_2EB

loc_316:
    cmp dl, 2
    jb  short loc_2EB
    pushf
    add al, 0E4h
    mov ah, al
    popf
    jnz short loc_32C

loc_323:
    mov al, 0E0h
    test    byte [BIOS_KEYBOARD_STATUS_AND_TYPE], 2   ; E0 flag set ?
    jnz short loc_32E

loc_32C:
    xor al, al

loc_32E:
    jmp Push_scancode
; -----------------------------------------------------------------------------

loc_331:
    jmp enable_kb_leave_kb_isr
; -----------------------------------------------------------------------------
data_array_334:
    db  2Ah ; *
    db    0
    db  8Eh ; é
    db 0F0h ; 
; -----------------------------------------------------------------------------

pfunc_loc_338:
    test    byte [BIOS_KEYBOARD_STATUS_AND_TYPE], 10h
    jnz short loc_34C
    mov al, [cs:bx+data_array_334]
    or  al, al
    jz  short loc_353
    mov dl, bl
    jmp short loc_2EE

; -----------------------------------------------------------------------------

loc_34C:
    call    loc_5C7
    js  short loc_2EE
    jnz short loc_32E

loc_353:
    call    enable_keyboard
    xor bx, bx
    int 5       ;  - PRINT-SCREEN KEY
                ; automatically called by keyboard scanner when print-screen
                ; key is pressed
    jmp clear_e0e1_leave_kb_isr

; -----------------------------------------------------------------------------

pfunc_reboot:
    cmp bl, 2            ; shift + ctrl ?
    jz  short loc_305

    jb  short loc_2FD    ; shift or nothing

    ; shift + ctrl + alt
    test    dl, 4
    jz  short loc_305    ; CTRL pressed ?

    mov word [BIOS_RESET_STATE_FLAG], 1234h ; (skip memory test)
    call    enable_keyboard
    jmp 0F000h:0E05Bh
; -----------------------------------------------------------------------------
pfunc_loc_377:

    cmp bl, 2
    jb  loc_2FD

loc_37C:
    jz  short loc_305
    call    loc_5C7
    js  short loc_30A
    mov ah, [BIOS_ALTERNATE_KEYBOARD_ENTRY]
    aad
    mov [BIOS_ALTERNATE_KEYBOARD_ENTRY],    al

loc_38C:
    jmp short loc_331

; -----------------------------------------------------------------------------

pfunc_loc_38E:
    cmp bl, 2
    jnb short loc_37C
    rol dl, 1
    rol dl, 1
    rol dl, 1
    xor dl, bl
    mov ch, [BIOS_KEYBOARD_STATUS_AND_TYPE]
    shr ch, 1
    not ch
    and dl, ch
    shr dl, 1
    mov al, 30h
    jb  loc_32E
    mov al, 80h
    test    dh, al
    jnz short loc_38C
    call    loc_443
    jmp loc_323

; -----------------------------------------------------------------------------

pfunc_loc_3B7:

    test    byte [BIOS_KEYBOARD_STATUS_AND_TYPE], 1       ; E1 flag set ?
    jnz short loc_3CF
    test    cl, 4
    jz  short loc_41F
    cmp bl, 2
    jnz loc_440
    test    byte [BIOS_KEYBOARD_STATUS_AND_TYPE], 10h
    jnz short loc_440

loc_3CF:
    or  byte [BIOS_KEYBOARD_EXTENDED_SHIFT_STATUS], 8
    call    enable_keyboard
    sti

loc_3D8:
    test    byte [BIOS_KEYBOARD_EXTENDED_SHIFT_STATUS], 8
    jnz short loc_3D8
    jmp clear_e0e1_leave_kb_isr

; -----------------------------------------------------------------------------

pfunc_loc_3E2:
    test    cl, 4
    jz  short loc_41F
    cmp bl, 2
    jnz short loc_440
    mov al, [BIOS_KEYBOARD_STATUS_AND_TYPE]
    aam 10h
    and al, 2
    shr al, 1
    xor ah, al
    jnz short loc_440
    or  byte [BIOS_BREAK_KEY_STATE], 80h
    mov ax, [BIOS_KEYBOARD_BUFFER_START_OFFSET_POINTER]
    mov [BIOS_KEYBOARD_BUFFER_HEAD_POINTER],    ax
    mov [BIOS_KEYBOARD_BUFFER_TAIL_POINTER],    ax
    call    Send_8042_Enable_Keyboard_command
    int 1Bh     ; CTRL-BREAK KEY
    xor ax, ax
    jmp Push_scancode

; -----------------------------------------------------------------------------

pfunc_loc_411:
    not al
    and [BIOS_KEYBOARD_EXTENDED_SHIFT_STATUS],  al

loc_417:
    jmp short loc_440

; -----------------------------------------------------------------------------

    nop
pfunc_loc_41A:
    test    cl, 4
    jnz short loc_440

loc_41F:
    test    dh, al
    jnz short loc_440
    call    loc_443
    jmp loc_20D

; -----------------------------------------------------------------------------

pfunc_loc_429:
    test    cl, 4
    jnz short loc_440
    test    byte [BIOS_KEYBOARD_STATUS_AND_TYPE], 10h
    jz  short loc_41F
    or  [BIOS_KEYBOARD_EXTENDED_SHIFT_STATUS],  al
    or  [BIOS_KEYBOARD_SHIFT_STATUS],   al
    jmp loc_20D

; -----------------------------------------------------------------------------

loc_440:
    jmp enable_kb_leave_kb_isr
; -----------------------------------------------------------------------------

loc_443:
    or  [BIOS_KEYBOARD_EXTENDED_SHIFT_STATUS],  al
    xor [BIOS_KEYBOARD_SHIFT_STATUS],   al
    retn

; -----------------------------------------------------------------------------

pfunc_loc_44C:
    test    byte [BIOS_KEYBOARD_STATUS_AND_TYPE], 2         ; E0 flag set ?
    jnz short loc_440
    not al
    and [BIOS_KEYBOARD_SHIFT_STATUS],   al
    jmp short loc_440

; -----------------------------------------------------------------------------

pfunc_loc_45B:
    not al
    test    byte [BIOS_KEYBOARD_STATUS_AND_TYPE], 1         ; E1 flag set ?
    jnz loc_4D1
    and [BIOS_KEYBOARD_SHIFT_STATUS],   al
    test    byte [BIOS_KEYBOARD_STATUS_AND_TYPE], 2         ; E0 flag set ?
    jz  short loc_47D
    and [BIOS_KEYBOARD_STATUS_AND_TYPE],    al
    mov al, [BIOS_KEYBOARD_EXTENDED_SHIFT_STATUS]
    shl al, 1
    shl al, 1
    jmp short loc_488

; -----------------------------------------------------------------------------

    nop

loc_47D:
    sar al, 1
    sar al, 1
    and [BIOS_KEYBOARD_EXTENDED_SHIFT_STATUS],  al
    mov al, [BIOS_KEYBOARD_STATUS_AND_TYPE]

loc_488:
    and al, 0Ch
    or  [BIOS_KEYBOARD_SHIFT_STATUS],   al
    cmp ah, 38h
    jnz short loc_440
    xor ax, ax
    xchg    al, [BIOS_ALTERNATE_KEYBOARD_ENTRY]
    or  al, al
    jz  short loc_440
    jmp Push_scancode

; -----------------------------------------------------------------------------

pfunc_loc_4A0:
    test    byte [BIOS_KEYBOARD_STATUS_AND_TYPE], 3   ; E1 or E0 flags set ?
    jnz short loc_440
    or  [BIOS_KEYBOARD_SHIFT_STATUS],   al
    test    byte [BIOS_KEYBOARD_STATUS_AND_TYPE], 10h
    jz  loc_440
    test    byte [BIOS_KEYBOARD_SHIFT_STATUS], 40h
    jz  loc_440
    test    byte [BIOS_BREAK_KEY_STATE], 40h
    jz  loc_440
    and byte [BIOS_KEYBOARD_SHIFT_STATUS], 0BFh
    jmp loc_20D

pfunc_loc_4C8:
    mov ah, [BIOS_KEYBOARD_STATUS_AND_TYPE]
    test    ah, 1                                     ; E1 flag set ?
    jz  short loc_4D4

loc_4D1:
    jmp loc_16E
; -----------------------------------------------------------------------------

loc_4D4:
    or  [BIOS_KEYBOARD_SHIFT_STATUS],   al
    test    ah, 2
    jz  short loc_4E4
    or  [BIOS_KEYBOARD_STATUS_AND_TYPE],    al
    jmp loc_440
; -----------------------------------------------------------------------------

loc_4E4:
    shr al, 1
    shr al, 1
    or  [BIOS_KEYBOARD_EXTENDED_SHIFT_STATUS],  al
    jmp loc_440
; -----------------------------------------------------------------------------
pfunc_loc_4EF:
    cmp cl, 0Ch
    jb  short loc_4F8

loc_4F4:
    inc bx

loc_4F5:
    jmp loc_305
; -----------------------------------------------------------------------------

loc_4F8:
    test    byte [BIOS_KEYBOARD_STATUS_AND_TYPE], 8 ; Right ALT key is pressed ?
    jnz short loc_4F4

pfunc_loc_4FF:
loc_4FF:
    cmp ah, 1Bh
    jz  short loc_4F5
    cmp ah, 29h
    jz  short loc_4F5
    cmp ah, 0Dh
    jbe short loc_515
    test    byte [BIOS_KEYBOARD_STATUS_AND_TYPE], 10h
    jz  short loc_4F5

loc_515:
    test    dl, 40h
    jz  short loc_4F5
    cmp bl, 1
    ja  short loc_4F5
    xor bl, 1
    jmp short loc_4F5
; -----------------------------------------------------------------------------
pfunc_loc_524:
    call    loc_5C7
    jns short loc_53F

    cmp al, 0FFh
    jz  short loc_53C

    cmp al, 0F0h
    jz  short loc_53F

    and al, 3Fh
    and byte [BIOS_BREAK_KEY_STATE], 0C0h
    or  [BIOS_BREAK_KEY_STATE], al

loc_53C:
    jmp enable_kb_leave_kb_isr
; -----------------------------------------------------------------------------

loc_53F:
    jmp loc_26F
; -----------------------------------------------------------------------------
data_array_542:
    db  61h ; a
    db  7Ah ; z
    db  71h ; q
    db  77h ; w
    db  6Dh ; m
    db  63h ; c
    db  1Eh
    db  2Ch ; ,
    db  10h
    db  11h
    db  32h ; 2
    db  27h ; '
; -----------------------------------------------------------------------------
pfunc_loc_54E:
    cmp bl, 3
    jnz short loc_560
    ; shift + ctrl + alt pressed

    mov di, data_array_542
    mov cx, 6
    call    scan_buffer
    mov ah, [cs:di+5]

loc_560:
    cmp al, 63h        ; 'c'
    nop
    nop
    jz  short loc_4FF
    jmp loc_251
; -----------------------------------------------------------------------------
data_array_569:
    db  5Eh ; ^
    db 0F9h ; ˘
data_array_56B:
    db  61h ; a
    db  65h ; e
    db  69h ; i
    db  6Fh ; o
    db  75h ; u
    db  79h ; y
    db  41h ; A
    db  4Fh ; O
    db  55h ; U

    db  83h ; É
    db  88h ; à
    db  8Ch ; å
    db  93h ; ì
    db  96h ; ñ
    db 0FFh
    db 0FFh
    db 0FFh
    db 0FFh

    db  84h ; Ñ
    db  89h ; â
    db  8Bh ; ã
    db  94h ; î
    db  81h ; Å
    db  98h ; ò
    db  8Eh ; é
    db  99h ; ô
    db  9Ah ; ö
; -----------------------------------------------------------------------------

scan_buffer:
    push    es
    push    cs
    pop es
    ;assume es:seg000
    cld
    repne scasb
    pop es
    ;assume es:nothing
    retn
; -----------------------------------------------------------------------------

loc_58E:
    push    ax
    and byte [BIOS_BREAK_KEY_STATE], 0C0h
    mov di, data_array_56B
    mov cx, 9
    call    scan_buffer
    jnz short loc_5B2
    mov cx, bx

loc_5A1:
    add di, 9
    loop    loc_5A1
    mov al, [cs:di-1]
    cmp al, 0FFh
    jz  short loc_5B2
    xor ah, ah
    pop bx
    retn
; -----------------------------------------------------------------------------

loc_5B2:
    xor ah, ah
    cmp al, 20h
    mov al, [cs:bx + data_array_569 - 1]
    jnz short loc_5BF
    pop bx
    retn
; -----------------------------------------------------------------------------

loc_5BF:
    call    Push_scancode_to_keyboard_buffer
    call    Speaker_Beep
    pop ax
    retn
; -----------------------------------------------------------------------------

loc_5C7:
    mov dl, bl
    test    byte [BIOS_KEYBOARD_STATUS_AND_TYPE], 2      ; E0 flag set ?
    jz  short loc_5E1
    mov bl, 5
    call    loc_5E1
    mov bl, al
    add bl, dl
    mov al, [cs:bx+data_array_880]
    or  al, al
    retn
; -----------------------------------------------------------------------------

loc_5E1:
    add bl, al
    test    byte [BIOS_BREAK_KEY_STATE], 40h
    jnz short loc_5F2
    mov al, [cs:bx+data_array_7C2]
    or  al, al
    retn
; -----------------------------------------------------------------------------

loc_5F2:
    test byte [BIOS_KEYBOARD_STATUS_AND_TYPE], 10h   ; 101/102-key keyboard installed ?
    jnz short loc_601
    mov al, [cs:bx+data_array_964]
    or  al, al
    retn
; -----------------------------------------------------------------------------

loc_601:
    mov al, [cs:bx+data_array_A2F]
    or  al, al
    retn

; -----------------------------------------------------------------------------

Push_scancode_to_keyboard_buffer:
    mov si, [BIOS_KEYBOARD_BUFFER_TAIL_POINTER]
    mov bx, si
    call    loc_61F
    cmp bx, [BIOS_KEYBOARD_BUFFER_HEAD_POINTER]
    jz  short locret_61E
    mov [si], ax
    mov [BIOS_KEYBOARD_BUFFER_TAIL_POINTER],    bx

locret_61E:
    retn
; -----------------------------------------------------------------------------

loc_61F:
    inc bx
    inc bx
    cmp bx, [BIOS_KEYBOARD_BUFFER_END_OFFSET_POINTER]
    jnz short locret_62B
    mov bx, [BIOS_KEYBOARD_BUFFER_START_OFFSET_POINTER]

locret_62B:
    retn
; -----------------------------------------------------------------------------

loc_62C:
    test    byte [BIOS_KEYBOARD_LED_STATUS], 40h
    jnz short locret_65F
    or  byte [BIOS_KEYBOARD_LED_STATUS], 40h
    mov ah, 0EDh
    call    loc_67C
    jnz short loc_65A
    mov al, [BIOS_KEYBOARD_SHIFT_STATUS]
    and al, 70h
    shr al, 1
    shr al, 1
    shr al, 1
    shr al, 1
    and byte [BIOS_KEYBOARD_LED_STATUS], 0F8h
    or  [BIOS_KEYBOARD_LED_STATUS], al
    mov ah, al
    call    loc_67C

loc_65A:
    and byte [BIOS_KEYBOARD_LED_STATUS], 0BFh

locret_65F:
    retn
; -----------------------------------------------------------------------------

enable_keyboard:

    cli
    mov al, 20h
    out 20h, al     ; Interrupt controller, 8259A.

Send_8042_Enable_Keyboard_command:
    mov ah, 0AEh

Send_8042_command:
    push    sp
    pop dx
    cmp dx, sp
    jnz short locret_67B
    mov dx, 64h

Send_Command:
    xor cx, cx

loc_672:
    in  al, 64h     ; AT Keyboard controller 8042.
    test    al, 2
    loopne  loc_672
    mov al, ah
    out dx, al      ; AT Keyboard controller 8042.

locret_67B:
    retn
; -----------------------------------------------------------------------------

loc_67C:
    push    sp
    pop bx
    cmp bx, sp
    jnz short locret_67B
    mov bx, 2
    mov dx, 60h

loc_688:
    and byte [BIOS_KEYBOARD_LED_STATUS], 0CFh
    call    Send_Command
    jnz short locret_6A8
    xor cx, cx
    sti

loc_695:
    mov al, [BIOS_KEYBOARD_LED_STATUS]
    and al, 30h
    loope   loc_695
    cli
    inc al
    jcxz    locret_6A8
    test    al, 20h
    jz  short locret_6A8
    dec bx
    jns short loc_688

locret_6A8:
    retn
; -----------------------------------------------------------------------------
Speaker_Beep:
    push    cx
    cli
    mov al, 0B6h
    out 43h, al     ; Timer 8253-5 (AT: 8254.2).
    mov al, 0A5h
    out 42h, al     ; Timer 8253-5 (AT: 8254.2).
    mov al, 2
    out 42h, al     ; Timer 8253-5 (AT: 8254.2).
    in  al, 61h     ; PC/XT PPI port B bits:

    mov ah, al
    or  al, 3
    out 61h, al     ; PC/XT PPI port B bits:

    sti
    mov al, 2
    xor cx, cx

loc_6C4:
    loop    loc_6C4
    dec al
    jnz short loc_6C4
    mov al, ah
    out 61h, al     ; PC/XT PPI port B bits:

loc_6CE:
    loop    loc_6CE
    pop cx
    retn
; -----------------------------------------------------------------------------
func_pointer_array:

    dw pfunc_loc_220     ; Function 0x00
    dw pfunc_loc_4C8     ; Function 0x01
    dw pfunc_loc_4A0     ; Function 0x02
    dw pfunc_loc_41A     ; Function 0x03
    dw pfunc_loc_3B7     ; Function 0x04
    dw pfunc_loc_3E2     ; Function 0x05
    dw pfunc_loc_429     ; Function 0x06
    dw pfunc_loc_38E     ; Function 0x07
    dw pfunc_reboot      ; Function 0x08
    dw pfunc_loc_377     ; Function 0x09
    dw pfunc_loc_251     ; Function 0x0A
    dw pfunc_loc_29F     ; Function 0x0B
    dw pfunc_loc_28E     ; Function 0x0C
    dw pfunc_loc_2A7     ; Function 0x0D
    dw pfunc_loc_338     ; Function 0x0E
    dw pfunc_loc_305     ; Function 0x0F
    dw pfunc_loc_2B3     ; Function 0x10
    dw pfunc_loc_2C9     ; Function 0x11
    dw pfunc_loc_2DD     ; Function 0x12
    dw pfunc_loc_4FF     ; Function 0x13
    dw pfunc_loc_4EF     ; Function 0x14
    dw pfunc_loc_524     ; Function 0x15
    dw pfunc_loc_54E     ; Function 0x16
    dw pfunc_enable_kb_leave_kb_isr      ; Function 0x17

func_pointer_array_2:
    dw pfunc_loc_242
    dw pfunc_loc_45B
    dw pfunc_loc_44C
    dw pfunc_loc_411
    dw pfunc_loc_411
    dw pfunc_loc_411
    dw pfunc_loc_411
    dw pfunc_loc_411

scancode_to_function_array:
    ; Scancode to functions handle
    ; Low byte = function number                                       ; Scancodes
    ;   esc      '1'     '2'     '3'     '4'     '5'     '6'     '7'
    dw 0000Fh, 0040Fh, 0080Fh, 00C0Fh, 0100Fh, 0140Fh, 0180Fh, 01C0Fh, ; 0x01 - 0x08
    ;    '8'     '9'     '0'     '-'     '='  backspace  tab     'q'
    dw 0200Fh, 0240Fh, 0280Fh, 02C0Fh, 0300Fh, 0340Fh, 03810h, 0710Ah, ; 0x09 - 0x10
    ;    'w'     'e'     'r'     't'     'y'     'u'     'i'     'o'
    dw 0770Ah, 0650Ah, 0720Ah, 0740Ah, 0790Ah, 0750Ah, 0690Ah, 06F0Ah, ; 0x11 - 0x18
    ;    'p' bracketL bracketR  enter controlL   'a'     's'     'd'
    dw 0700Ah, 03C0Fh, 0400Fh, 04412h, 00481h, 0610Ah, 0730Ah, 0640Ah, ; 0x19 - 0x20
    ;    'f'     'g'     'h'     'j'     'k'     'l' semicolon apostrophe
    dw 0660Ah, 0670Ah, 0680Ah, 06A0Ah, 06B0Ah, 06C0Ah, 04A0Fh, 04E0Fh, ; 0x21 - 0x28
    ;  grave   shiftL backslash  'z'     'x'     'c'     'v'     'b'
    dw 0520Fh, 00282h, 0560Fh, 07A0Ah, 0780Ah, 0630Ah, 0760Ah, 0620Ah, ; 0x29 - 0x30
    ;    'n'     'm'   comma   period  slash   shiftR  NUM *    AltL
    dw 06E0Ah, 06D0Ah, 05A0Fh, 05E0Fh, 06212h, 00182h, 0680Eh, 00881h, ; 0x31 - 0x38
    ;   Space capsLock   F1      F2      F3      F4      F5      F6
    dw 06E0Fh, 04083h, 0000Ch, 0400Ch, 0000Bh, 0000Bh, 0000Bh, 0000Bh, ; 0x39 - 0x40
    ;    F7      F8      F9      F10   NUMLCK SCROLLCK PAD 7   PAD 8
    dw 0000Bh, 0000Bh, 0000Bh, 0000Bh, 02084h, 01085h, 07209h, 07809h, ; 0x41 - 0x48
    ;  PAD 9   PAD -   PAD 4   PAD 5   PAD 6   PAD +   PAD 1   PAD 2
    dw 07E09h, 08409h, 08809h, 08E09h, 09209h, 09809h, 09C09h, 0A209h, ; 0x49 - 0x50
    ;  PAD 3   PAD 0   PAD .  AltPrnt             <>    F11     F12
    dw 0A809h, 0AE87h, 0B408h, 00480h, 00017h, 0BA0Fh, 0000Dh, 0000Dh  ; 0x51 - 0x58

data_array_7C2:
    db 01Bh, 01Bh, 01Bh, 0F0h, 031h, 021h, 0FFh, 094h, 032h, 040h, 0FEh, 095h
    db 033h, 023h, 0FFh, 096h, 034h, 024h, 0FFh, 097h, 035h, 025h, 0FFh, 098h
    db 036h, 05Eh, 01Eh, 099h, 037h, 026h, 0FFh, 09Ah, 038h, 02Ah, 0FFh, 09Bh
    db 039h, 028h, 0FFh, 09Ch, 030h, 029h, 0FFh, 09Dh, 02Dh, 05Fh, 01Fh, 09Eh
    db 03Dh, 02Bh, 0FFh, 09Fh, 008h, 008h, 07Fh, 0F0h, 009h, 0FEh, 0FFh, 0F0h
    db 05Bh, 07Bh, 01Bh, 0F0h, 05Dh, 07Dh, 01Dh, 0F0h, 00Dh, 00Dh, 00Ah, 0F0h
    db 0F0h, 000h, 03Bh, 03Ah, 0FFh, 0F0h, 027h, 022h, 0FFh, 0F0h, 060h, 07Eh
    db 0FFh, 0F0h, 05Ch, 07Ch, 01Ch, 0F0h, 02Ch, 03Ch, 0FFh, 0F0h, 02Eh, 03Eh
    db 0FFh, 0F0h, 02Fh, 03Fh, 0FFh, 0F0h, 0F0h, 004h, 02Ah, 02Ah, 0B2h, 0F0h
    db 0F0h, 008h, 020h, 020h, 020h, 020h, 0FEh, 037h, 093h, 007h, 007h, 00Ch
    db 0FEh, 038h, 0A9h, 008h, 008h, 010h, 0FEh, 039h, 0A0h, 009h, 009h, 014h
    db 02Dh, 02Dh, 0AAh, 0F0h, 0FEh, 034h, 08Fh, 004h, 004h, 018h, 0F0h, 035h
    db 0ABh, 005h, 0FEh, 036h, 090h, 006h, 006h, 01Ch, 02Bh, 02Bh, 0ACh, 0F0h
    db 0FEh, 031h, 091h, 001h, 001h, 020h, 0FEh, 032h, 0ADh, 002h, 002h, 024h
    db 0FEh, 033h, 092h, 003h, 003h, 028h, 0FEh, 030h, 0AEh, 000h, 000h, 02Ch
    db 0FEh, 02Eh, 0AFh, 0FFh, 0FFh, 030h, 05Ch, 07Ch, 0FFh, 0F0h

data_array_880:
    db 00Dh, 00Dh, 00Ah, 0C2h, 02Fh, 02Fh, 0B1h, 0C0h, 000h, 000h, 08Eh, 0FFh
    db 0FEh, 0FEh, 093h, 0B3h, 0FEh, 0FEh, 0A9h, 0B4h, 0FEh, 0FEh, 0A0h, 0B5h
    db 0FEh, 0FEh, 08Fh, 0B7h, 0FEh, 0FEh, 090h, 0B9h, 0FEh, 0FEh, 091h, 0BBh
    db 0FEh, 0FEh, 0ADh, 0BCh, 0FEh, 0FEh, 092h, 0BDh, 0FEh, 0FEh, 0AEh, 0BEh
    db 0FEh, 0FEh

array_2_710_1A2:
    db 0AFh, 0BFh, 00Fh, 000h, 013h, 004h, 014h, 008h, 014h, 00Dh, 014h, 012h
    db 014h, 017h, 014h, 01Ch, 014h, 021h, 014h, 026h, 014h, 02Bh, 014h, 030h
    db 014h, 035h, 014h, 03Ah, 00Fh, 03Fh, 010h, 043h, 016h, 061h, 016h, 07Ah
    db 00Ah, 065h, 00Ah, 072h, 00Ah, 074h, 00Ah, 079h, 00Ah, 075h, 00Ah, 069h
    db 00Ah, 06Fh, 00Ah, 070h, 015h, 047h, 014h, 04Bh, 012h, 050h, 081h, 004h
    db 016h, 071h, 00Ah, 073h, 00Ah, 064h, 00Ah, 066h, 00Ah, 067h, 00Ah, 068h
    db 00Ah, 06Ah, 00Ah, 06Bh, 00Ah, 06Ch, 016h, 06Dh, 013h, 056h, 014h, 05Ah
    db 082h, 002h, 013h, 05Fh, 016h, 077h, 00Ah, 078h, 00Ah, 063h, 00Ah, 076h
    db 00Ah, 062h, 00Ah, 06Eh, 016h, 063h, 013h, 067h, 013h, 06Bh, 011h, 06Fh
    db 082h, 001h, 00Eh, 075h, 081h, 008h, 013h, 07Bh, 086h, 040h, 00Ch, 000h
    db 00Ch, 040h, 00Bh, 000h, 00Bh, 000h, 00Bh, 000h, 00Bh, 000h, 00Bh, 000h
    db 00Bh, 000h, 00Bh, 000h, 00Bh, 000h, 084h, 020h, 085h, 010h, 009h, 07Fh
    db 009h, 085h, 009h, 08Bh, 009h, 091h, 009h, 095h, 009h, 09Bh, 009h, 09Fh
    db 009h, 0A5h, 009h, 0A9h, 009h, 0AFh, 009h, 0B5h, 087h, 0BBh, 008h, 0C1h
    db 080h, 004h, 017h, 000h, 013h, 0C7h, 00Dh, 000h, 00Dh, 000h

data_array_964:
    db 01Bh, 01Bh, 01Bh, 0F0h, 026h, 031h, 0FFh, 094h, 082h, 032h, 0FEh, 095h
    db 040h, 022h, 033h, 0FFh, 096h, 023h, 027h, 034h, 0FFh, 097h, 097h, 028h
    db 035h, 0FFh, 098h, 07Bh, 015h, 036h, 01Eh, 099h, 05Eh, 08Ah, 037h, 0FFh
    db 09Ah, 09Ah, 021h, 038h, 0FFh, 09Bh, 07Ch, 087h, 039h, 0FFh, 09Ch, 09Ch
    db 085h, 030h, 0FFh, 09Dh, 09Dh, 029h, 0F8h, 0FFh, 09Eh, 07Dh, 02Dh, 05Fh
    db 01Fh, 09Fh, 09Fh, 008h, 008h, 07Fh, 0F0h, 009h, 0FEh, 0FFh, 0F0h, 081h
    db 082h, 01Bh, 05Bh, 024h, 02Ah, 01Dh, 0F0h, 05Dh, 00Dh, 00Dh, 00Ah, 0F0h
    db 0F0h, 000h, 097h, 025h, 0FFh, 0F0h, 03Ch, 03Eh, 0FFh, 0F0h, 05Ch, 0E6h
    db 09Ch, 01Ch, 0F0h, 02Ch, 03Fh, 0FFh, 0F0h, 03Bh, 02Eh, 0FFh, 0F0h, 03Ah
    db 02Fh, 0FFh, 0F0h, 03Dh, 02Bh, 0FFh, 0F0h, 0F0h, 004h, 02Ah, 000h, 08Eh
    db 0F0h, 0F0h, 008h, 020h, 020h, 020h, 020h, 0FEh, 037h, 093h, 007h, 007h
    db 00Ch, 0FEh, 038h, 0A9h, 008h, 008h, 010h, 0FEh, 039h, 0A0h, 009h, 009h
    db 014h, 02Dh, 02Dh, 0AAh, 0F0h, 0FEh, 034h, 08Fh, 004h, 004h, 018h, 0F0h
    db 035h, 0ABh, 005h, 0FEh, 036h, 090h, 006h, 006h, 01Ch, 02Bh, 02Bh, 0ACh
    db 0F0h, 0FEh, 031h, 091h, 001h, 001h, 020h, 0FEh, 032h, 0ADh, 002h, 002h
    db 024h, 0FEh, 033h, 092h, 003h, 003h, 028h, 0FEh, 030h, 0AEh, 000h, 000h
    db 02Ch, 0FEh, 02Eh, 0AFh, 0FFh, 0FFh, 030h, 03Ch, 03Eh, 0FFh, 0F0h

data_array_A2F:
    db 01Bh, 01Bh, 01Bh, 0F0h, 026h, 031h, 0FFh, 094h, 082h, 032h, 0FEh, 095h
    db 07Eh, 022h, 033h, 0FFh, 096h, 023h, 027h, 034h, 0FFh, 097h, 07Bh, 028h
    db 035h, 0FFh, 098h, 05Bh, 02Dh, 036h, 01Eh, 099h, 07Ch, 08Ah, 037h, 0FFh
    db 09Ah, 060h, 05Fh, 038h, 01Ch, 09Bh, 05Ch, 087h, 039h, 0FFh, 09Ch, 05Eh
    db 085h, 030h, 0FFh, 09Dh, 040h, 029h, 0F8h, 0FFh, 09Eh, 05Dh, 03Dh, 02Bh
    db 01Fh, 09Fh, 07Dh, 008h, 008h, 07Fh, 0F0h, 009h, 0FEh, 0B0h, 0F0h, 081h
    db 082h, 01Bh, 0F0h, 024h, 09Ch, 01Dh, 0F0h, 024h, 00Dh, 00Dh, 00Ah, 0F0h
    db 0F0h, 000h, 097h, 025h, 0FFh, 0F0h, 0FDh, 0FFh, 0FFh, 0F0h, 0F0h, 02Ah
    db 0E6h, 01Ch, 0F0h, 02Ch, 03Fh, 0FFh, 0F0h, 03Bh, 02Eh, 0FFh, 0F0h, 03Ah
    db 02Fh, 0FFh, 0F0h, 021h, 015h, 0FFh, 0F0h, 0F0h, 004h, 02Ah, 02Ah, 0B2h
    db 0F0h, 0F0h, 008h, 020h, 020h, 020h, 020h, 0FEh, 037h, 093h, 007h, 007h
    db 00Ch, 0FEh, 038h, 0A9h, 008h, 008h, 010h, 0FEh, 039h, 0A0h, 009h, 009h
    db 014h, 02Dh, 02Dh, 0AAh, 0F0h, 0FEh, 034h, 08Fh, 004h, 004h, 018h, 0F0h
    db 035h, 0ABh, 005h, 0FEh, 036h, 090h, 006h, 006h, 01Ch, 02Bh, 02Bh, 0ACh
    db 0F0h, 0FEh, 031h, 091h, 001h, 001h, 020h, 0FEh, 032h, 0ADh, 002h, 002h
    db 024h, 0FEh, 033h, 092h, 003h, 003h, 028h, 0FEh, 030h, 0AEh, 000h, 000h
    db 02Ch, 0FEh, 02Eh, 0AFh, 0FFh, 0FFh, 030h, 03Ch, 03Eh, 0FFh, 0FFh

array_0afa: ; 1 = shift key pressed, 2 = shift + ctrl, 3 = shift + ctrl + alt
    db 0,1,1,1 ,2,2,2,2 ,3,3,3,3 ,3,3,3,3

; -----------------------------------------------------------------------------

entry_point:

    push    cs
    pop ds

    mov dx, INSTALL_MSG
    mov ah, 9
    int 21h     ; DOS - PRINT STRING


    mov dx, BANNER_MSG
    mov ah, 9
    int 21h     ; DOS - PRINT STRING

    mov ax, 40h
    mov ds, ax

    mov byte [BIOS_BREAK_KEY_STATE], 40h

isr_keyboard_installer:

    ; Install the new keyboard isr vector
    cli
    xor ax, ax
    mov ds, ax
    mov word [24h], isr_keyboard
    mov word [26h], cs
    sti

    mov dx, isr_keyboard_installer   ; Program size
    shr dx, 1
    shr dx, 1
    shr dx, 1
    shr dx, 1                        ; dx / 16
    mov ax, 3103h
    int 21h                          ; (dos 2+ - Terminate but stay resident)

;------------------------------------------------------------------------------
;-------------------------------DATA-------------------------------------------
;------------------------------------------------------------------------------

BANNER_MSG          db "(SMT-Goupil - 28/6/88)",13,10,"$"
INSTALL_MSG         db "Installation du clavier franÁais",13,10,"$"
