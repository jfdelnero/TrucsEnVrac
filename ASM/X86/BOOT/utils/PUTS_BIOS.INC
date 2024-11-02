; -----------------------------------------------------------------------------
; -----------------------------------------------------------------------------
; 8086 Print string (use bios I/O)
; (Use nasm to assemble it)
; Jan-2024 Jean-François DEL NERO / HxC2001
; -----------------------------------------------------------------------------
; -----------------------------------------------------------------------------

[bits  16]

section .text

;------------------------------------------------------------------------------
; Print null terminated string function
; ds:si : string pointer
global puts_bios
puts_bios:
	push ax
	push si

puts_bios_loop:
	mov al,[ds:si]
	or  al,al
	jz  end_puts_bios

	call putc_bios

	add si,1

	jmp puts_bios_loop

end_puts_bios:
	pop si
	pop ax

	ret
