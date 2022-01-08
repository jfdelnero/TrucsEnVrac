; -------------------------------------------------------------------------------------------------
; -------------------------------------------------------------------------------------------------
; Minimal bootable PC disk image
; (Use nasm to assemble it)
; Jan-2022 Jean-François DEL NERO / HxC2001
; -------------------------------------------------------------------------------------------------
; nasm boot.asm -o boot.img
; -------------------------------------------------------------------------------------------------
	org 0x7c00

	; Disable all interruptions
	cli

	; Print the startup message
	mov     si, msg_start
	call    print_string_func

    ; Stop -> "sleep mode"
loopwait:
	hlt
	jmp loopwait

;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
;------------------------------------------------------------------------------

;------------------------------------------------------------------------------
; print char function
print_char_func:
	push ax
	push bx

	mov  ah, 0x0E  ; Teletype output
	mov  bh, 0x00  ; Page Number
	mov  bl, 0x07  ; Color (only in graphic mode)

	int  0x10      ; Real mode videos services.

	pop bx
	pop ax

	ret

;------------------------------------------------------------------------------
; print string function
print_string_func:

	mov al,[cs:si]
	or  al,al
	jz  end_print_string_func

	call print_char_func

	add si,1

	jmp print_string_func

end_print_string_func:

	ret
;------------------------------------------------------------------------------

msg_start   db "Boot sector Started ! :)",13,10,0

;------------------------------------------------------------------------------
;   sector padding + bootable sector magic number
	times   510-($-$$)      db      0
	dw      0xAA55

;   Floppy image padding
	times   737280-($-$$)   db      0xF6