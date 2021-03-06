; -----------------------------------------------------------------------------
; -----------------------------------------------------------------------------
; RadioTrack DOS control software
; Minimum requirements : A 8086/8088 PC and ~1KB of free memory.
; (Use nasm to assemble it: nasm RADIOTRK.ASM -o RADIOTRK.COM)
; July 1999 - Jean-Fran�ois DEL NERO / HxC2001
; -----------------------------------------------------------------------------
; -----------------------------------------------------------------------------
;
; AIMS Lab Pte Ltd Radio Track FM Tuner DOS control software
;
; Main parts :
;
; Sanyo LM7000     : Direct PLL Frequency Synthesizer for Electronic Tuning.
; Philips TEA5712T : AM/FM stereo DTS radio circuit.
; Sanyo LC7534M    : Electronic Volume control.
; Samsung KA2206   : 4.0W Dual Audio Power Amp.
; GAL16V8B         : ISA address decoder.
; 74LS273          : ISA register latch.
;
; ISA Port number  : 30Ch or 20Ch (jumper JP1 set)
;
; Port Usage       : Bit 0 : LM7000 CE
;                  : Bit 1 : Write: LM7000 CL /serial clock line, Read: Tuned
;                  : Bit 2 : LM7000 DATA serial line
;                  : Bit 3 : Enable / Mute
;                  : Bit 4 : LM7000 STRQ
;                  : Bit 5 : RFU
;                  : Bit 6 : /Up volume
;                  : Bit 7 : /Down volume
;
; ISA Port connections :
;   - Data : D0 <> D7
;   - Address : A2 <> A9  (A0 & A1 not connected : The board present the same
;                          register at 30Ch,30Dh,30Eh and 30Fh...)
;   - Control : RESET, AEN, I/O Write, I/O Read.
;   - +5V and +12V (For the power amp)
;
; GAL connections:
; GAL Pin 1  : ISA I/O Read
; GAL Pin 2  : ISA I/O Write
; GAL Pin 3  : ISA A2
; GAL Pin 4  : ISA A3
; GAL Pin 5  : ISA A4
; GAL Pin 6  : ISA A5
; GAL Pin 7  : ISA A6
; GAL Pin 8  : ISA A7
; GAL Pin 9  : ISA A8
; GAL Pin 10 : GND
; GAL Pin 11 : ISA A9
; GAL Pin 12 : LS273 Latch CP pin
; GAL Pin 13 : LM7000 /STOUT (Auto search stop/ "Tuned" status)
; GAL Pin 14 : ISA AEN
; GAL Pin 15 : Jumper (Default set to high : 30Ch-30Fh, Low : 20Ch-20Fh )
; GAL Pin 16 : ISA RESET
; GAL Pin 17 : TEA5712T IFCNT pin (path not populated !)
; GAL Pin 18 : LS273 Latch MR pin
; GAL Pin 19 : ISA D1 -> Used to return the LM7000 "Tuned" status
; GAL Pin 20 : +5V
;
; Latch LS273 output connections :
; Q7 : LC7534 /Down pin (Pin 17)
; Q6 : LC7534 /Up pin   (Pin 19)
; Q5 : NC
; Q4 : LM7000 STRQ      (Pin 2)
; Q3 : LC7534 CE pin    (Pin 4)
; Q2 : LM7000 DATA      (Pin 5)
; Q1 : LM7000 CLOCK     (Pin 4)
; Q0 : LM7000 CE        (Pin 3)
;
; LM7000:
; BO1,BO2,BO3 : Not connected
;

    org 100h

    jmp entry_point

;------------------------------------------------------------------------------
;
; Print a decimal value
; AX = value
;
;------------------------------------------------------------------------------

Print_dec:
    pusha

    mov  cx, 100   ; make it support "000" to "999"

loop_print_dec:
    xor  dx, dx
    div  cx

    add  al, '0'
    mov  ah, 0Eh
    int  10h

    mov  ax, dx    ; div remain

    push dx
    push ax
    xor  dx, dx
    mov  ax, cx
    mov  cx, 10
    div  cx        ; preparing the next divisor
    mov  cx, ax
    pop  ax
    pop  dx

    cmp  cx, 0
    jnz  loop_print_dec

    popa

    ret

;------------------------------------------------------------------------------
;
; Print the radio status bar
;
;------------------------------------------------------------------------------

Display_RadioState:
    pusha

    ; Set cursor position : Line 2, colonn 0
    mov  ah, 2
    mov  bh, 0
    mov  dx, 0200h
    int  10h

    mov  dx, AM_MSG
    mov  al, [ds:BANDE]
    cmp  al, 1
    jne  am_mode
    mov  dx, FM_MSG
am_mode:
    mov  ah, 9
    int  21h

    mov  ax, [ds:FREQUENCY]
    mov  dx, [ds:FREQUENCY+2]
    mov  cx, 1000
    div  cx

    push dx

    call Print_dec

    mov  al, '.';
    mov  ah, 0Eh;
    int  10h

    pop  ax

    call Print_dec

    mov  dx, MHZ_MSG
    mov  ah, 9
    int  21h

    mov  dx, REF_MSG
    mov  ah, 9
    int  21h

    mov  ax, [ds:REFERENCE]
    call Print_dec

    mov  dx, KHZ_MSG
    mov  ah, 9
    int  21h

    mov  dx, NOT_MUTED_MSG
    mov  al, [ds:MUTESTATUS]
    cmp  al, 1
    jne  not_muted
    mov  dx, MUTED_MSG
not_muted:
    mov  ah, 9
    int  21h

    mov  dx, NOT_MUTED_MSG
    mov  ah, 9
    int  21h

    popa
    ret

;------------------------------------------------------------------------------
;
; 20ms delay
;
;------------------------------------------------------------------------------

DELAY:
    pusha

    mov  cx, 00000h
    mov  dx, 04E20h
    mov  ah, 86h
    int  15h

    popa

    ret

;------------------------------------------------------------------------------
;
; Push a bit to the LM7000 PLL
; AL = bit value
;
;------------------------------------------------------------------------------
Send_Bit:

    pusha

    mov  dx, [ds:CARD_PORT]

    mov  cl, 11001001b
    and  al, 1
    shl  al, 2
    or   al, cl
    out  dx, al
    out  dx, al

    or   al, 00000010b
    out  dx, al
    out  dx, al

    popa

    ret

;------------------------------------------------------------------------------
;
; Update the LM7000 Frequency array
; AX = Frequency
;
;------------------------------------------------------------------------------
Update_LM7000_FREQ_ARRAY:
    pusha

    mov  si, LM7000_REG_ARRAY
    mov  bx, 1
    mov  cx, ax

UPDATE_Freq_Loop:

    mov  dl, 0
    and  cx, bx
    jz   SETZERO

    mov  dl, 1

SETZERO:

    mov  [ds:si], dl

    mov  cx, ax
    add  si, 1
    rol  bx, 1
    cmp  bx, 0100000000000000B
    jne  UPDATE_Freq_Loop

    popa
    ret

;------------------------------------------------------------------------------
;
; Update the LM7000 register
;
;------------------------------------------------------------------------------

Update_LM7000_Reg:
    pusha

    mov  dx, [ds:CARD_PORT]
    mov  si, LM7000_REG_ARRAY

Loop_Set_Card:

    mov  al, [ds:si]
    cmp  al, 0ffh
    je   End_Set_Card

    call Send_Bit

    add  si, 1
    jmp  Loop_Set_Card

End_Set_Card:

    mov  al, 11001000b
    out  dx, al

    popa
    ret

;------------------------------------------------------------------------------
;
; Flush the keyboard buffer
;
;------------------------------------------------------------------------------

Flush_keyboard_buffer:
    push bx
    push es
    push di

    mov  bx, 40h
    mov  es, bx
    mov  di, 1ch
    mov  bx, [es:di]
    mov  di, 1ah
    mov  [es:di], bx

    pop  di
    pop  es
    pop  bx

    ret

;------------------------------------------------------------------------------
;
; Update the tune status (TUNESTATUS)
;
;------------------------------------------------------------------------------

Update_Tune_Status:
    pusha

    mov  ah, 2
    mov  bh, 0
    mov  dx, 022Bh
    int  10h

    mov  cl, 11010000B
    mov  dx, [ds:CARD_PORT]
    mov  bl, [ds:MUTESTATUS]
    dec  bl
    jz   Card_Muted

    mov  cl, 11011000B
Card_Muted:

    mov  al, cl
    out  dx, al
    xor  ax, ax
    xor  ax, ax
    xor  ax, ax
    in   al, dx
    in   al, dx
    and  al, 00000010B
    jz   TUNEOK

    ; Not tuned

    xor  al, al
    mov  [ds:TUNESTATUS], al

    mov  dx, NOT_TUNED_MSG
    mov  ah, 9
    int  21h
    popa

    ret

TUNEOK:
    mov  al, 1
    mov  [ds:TUNESTATUS], al

    mov  dx, TUNED_MSG
    mov  ah, 9
    int  21h

    popa

    ret

;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
;
; Entry Point !
;
;------------------------------------------------------------------------------
;------------------------------------------------------------------------------

entry_point:

    ; DS=ES=CS
    push cs
    push cs
    pop  ds
    pop  es

    ; Clear screen
    mov  ax, 0003h
    int  10h

    ; Initial message
    mov  dx, BANNER_MSG
    mov  ah, 9
    int  21h

    mov  cx, 0
    call Change_Frequency

MAIN_LOOP:

    call Update_Tune_Status

    hlt

    ; Get the State of the keyboard buffer
    mov  ah, 1
    int  16h

    JZ   MAIN_LOOP  ; Nothing in the buffer -> Main loop

    call Flush_keyboard_buffer

    cmp  ax, 4800h       ; Up key
    jne  cont01
    mov  al, 1
    call Change_Volume
cont01:

    cmp  ax, 5000h       ; Down key
    jne  cont02
    mov  al, 0
    call Change_Volume
cont02:

    cmp  ax, 4f00h       ; mute
    jne  cont03
    call Toggle_Mute
cont03:

    cmp  ax, 4D00h       ; Right key
    jne  cont04
    mov  cx, [ds:REFERENCE]
    call Change_Frequency
cont04:

    cmp  ax, 4B00h        ; Left key
    jne  cont05
    mov  cx, [ds:REFERENCE]
    neg  cx
    call Change_Frequency
cont05:

    cmp  ax, 4d36h        ; Shift + Right key
    jne  cont06

    mov  cx, 1000
    call Change_Frequency

cont06:

    cmp  ax, 4b34h        ; Shift + Left key
    jne  cont07

    mov  cx, -1000
    call Change_Frequency

cont07:

    cmp  ax, 5100h        ; pgdn
    jne  cont08
    call Toggle_BAND
cont08:

    cmp  ax, 4900h        ; pgup
    jne  cont09
    call Change_STEP
cont09:

    cmp  ax, 011Bh        ; Escape
    je   Return_to_DOS

    jmp  MAIN_LOOP

Return_to_DOS:

    mov  ax, 4c00h
    int  21h

    ret

;------------------------------------------------------------------------------
;
; Select another LM7000 configuration
;
;------------------------------------------------------------------------------

Change_STEP:

    pusha
    mov  ah, 2
    mov  bh, 0
    mov  dx, 0400h
    int  10h
    popa

    mov  di, [ds:REF_CFG_INDEX]
    add  di, 4
    mov  bl, [ds:di]
    cmp  bl, 0ffh
    jne  end_table_not_reached
    mov  di, REF_CFG_ARRAY
end_table_not_reached:
    mov  [ds:REF_CFG_INDEX], di

    xor  bx, bx
    mov  bl, [ds:di]
    mov  [ds:REFERENCE], bx
    xor  bh, bh

    call Display_RadioState

    ; Update LM7000 reference frequency field
    inc  di
    mov  si, stp1
loop_update_ref:
    mov  al, [cs:di]
    mov  [cs:si], al
    inc  si
    inc  di
    cmp  si, stp1 + 3
    jne  loop_update_ref

    call Update_LM7000_Reg

    xor  al,al

    ret

;------------------------------------------------------------------------------
;
; Toggle FM / AM mode
;
;------------------------------------------------------------------------------

Toggle_BAND:

    mov  al, [ds:BANDE]
    xor  al, 1
    and  al, 1
    mov  [ds:BANDE],al

    call Update_LM7000_Reg

    call Display_RadioState

    xor  al, al

    ret

;------------------------------------------------------------------------------

; Frequency calculation helper (addition)
; cx -> count
; dx -> high
; ax -> low
; bx
AddFreq:
    cmp  cx, 0
    jnl  notneg
    neg  cx
    sub  ax, cx
    jnc  end_incn
    dec  dx
end_incn:
    ret

notneg:
    add  ax, cx
    jnc  end_inc
    inc  dx
end_inc:
    ret

;------------------------------------------------------------------------------
;
; Change Frequency
; cx = delta
;
;------------------------------------------------------------------------------

Change_Frequency:

    mov  dx, [ds:FREQUENCY+2]
    mov  ax, [ds:FREQUENCY]
    call AddFreq               ; Frequency = Frequency +/- Offset
    mov  [ds:FREQUENCY+2], dx
    mov  [ds:FREQUENCY], ax
    call Display_RadioState

    mov  cx, [ds:IF_FREQ]
    call AddFreq               ; (Frequency + IF_Freq)

    mov  cx, [ds:REFERENCE]

    div  cx                    ; AX = (Frequency + IF_Freq) / Ref

    call Update_LM7000_FREQ_ARRAY
    call Update_LM7000_Reg

    xor  al, al

    ret

;------------------------------------------------------------------------------
;
; Change Volume
; al = 0 -> down , 1 -> up
;
;------------------------------------------------------------------------------

Change_Volume:

    mov  dx, [ds:CARD_PORT]

    mov  cl, 01000000B
    and  al, 1
    jz   dec_vol
    rol  cl, 1
dec_vol:

    mov  al, 00001000B
    or   al, cl
    out  dx, al

    call DELAY

    mov  al, 11001000B
    out  dx, al

    xor  al,al

    ret

;------------------------------------------------------------------------------
;
; Toggle Mute state
;
;------------------------------------------------------------------------------

Toggle_Mute:

    mov  bl, [ds:MUTESTATUS]
    xor  bl, 1
    and  bl, 1
    mov  [ds:MUTESTATUS],bl

    mov  dx, [ds:CARD_PORT]
    mov  al, 11000000B
    shl  bl, 3             ; Bit 3 -> Mute
    or   al, bl
    out  dx, al

    call Display_RadioState

    xor  al,al

    ret

;------------------------------------------------------------------------------
;-------------------------------DATA-------------------------------------------
;------------------------------------------------------------------------------

BANNER_MSG         db "RadioTrack v0.6 (c) 1998-2020 Jeff / HxC2001",10,13
                   db "Left/Right:Frequency, Up/Down:Volume, PgDn:AM/FM,"
                   db " End:Mute, PgUp:mode, Esc:Quit",10,13,"$"

CARD_PORT          dw 30Ch

TUNED_MSG          db "TUNED ","$"
NOT_TUNED_MSG      db "      ","$"
MUTED_MSG          db "MUTED | ","$"
NOT_MUTED_MSG      db "      | ","$"
MHZ_MSG            db " MHz | ","$"
KHZ_MSG            db " KHz | ","$"
AM_MSG             db "| AM | ","$"
FM_MSG             db "| FM | ","$"
REF_MSG            db "Ref ","$"

MUTESTATUS         db 0                                     ; 1 = muted

;------------------------------------------------------------------------------
; LM7000 Serial buffer
LM7000_REG_ARRAY   db 1,0,1,1 ,1,1,1,1 ,1,1,0,0 ,0,0        ;divisor data
                   db 0,0                                   ;test
                   db 0,0,0,0                               ;band
stp1               db 0,1,0                                 ;reference
BANDE              db 1                                     ;am/fm
                   db 0FFh
;------------------------------------------------------------------------------

;Default station: 91MHz (Radio Flotteurs/Flotteurs FM at Clamecy / France ;) )
FREQUENCY          dd 91000                                 ;KHz
REFERENCE          dw    25                                 ;KHz
IF_FREQ            dw 10700                                 ;KHz / (10.7 MHz)
TUNESTATUS         db 0

;                     ref,ref setting
REF_CFG_ARRAY      db 100,0,0,0,
                   db 50 ,0,0,1,
                   db 25 ,0,1,0,
                   db 5  ,0,1,1,
                   db 10 ,1,0,0,
                   db 9  ,1,0,1,
                   db 1  ,1,1,0,
                   db 5  ,1,1,1,
                   db 0ffh

REF_CFG_INDEX      dw REF_CFG_ARRAY
;------------------------------------------------------------------------------
