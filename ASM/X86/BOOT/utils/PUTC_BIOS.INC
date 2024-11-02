; -----------------------------------------------------------------------------
; -----------------------------------------------------------------------------
; 8086 Print character (use bios I/O)
; (Use nasm to assemble it)
; Jan-2024 Jean-François DEL NERO / HxC2001
; -----------------------------------------------------------------------------
; -----------------------------------------------------------------------------

[bits  16]

section .text

;------------------------------------------------------------------------------
; print char function
; al = character to print
global putc_bios
putc_bios:
	push ax
	push bx

	mov ah,0x0E                            ; Teletype output
	mov bh,0x00                            ; Page Number
	mov bl,0x07                            ; Color (only in graphic mode)

	int  0x10                              ; BIOS Videos services.

	pop bx
	pop ax

	ret
