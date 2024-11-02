; -----------------------------------------------------------------------------
; -----------------------------------------------------------------------------
; 386 Hexadecimal print functions
; (Use nasm to assemble it)
; Jan-2024 Jean-François DEL NERO / HxC2001
; -----------------------------------------------------------------------------
; -----------------------------------------------------------------------------

[bits  32]

section .text

global prt_hex_quartet_386
prt_hex_quartet_386:

	push ax

	and al,0x0F

	cmp al,0xA
	jnae .l01

	sub al,0xA

	add al,'A'
	call putc_386

	pop ax

	ret

.l01:
	add al,'0'
	call putc_386

	pop  ax

	ret

global prt_hex_byte_386
prt_hex_byte_386:

	push ax
	shr  al, 4
	call prt_hex_quartet_386
	pop  ax
	call prt_hex_quartet_386

	ret

global prt_hex_short_386
prt_hex_short_386:
	push ax

	shr  ax,8
	call prt_hex_byte_386

	pop  ax

	call prt_hex_byte_386

	ret

global prt_hex_long_386
prt_hex_long_386:

print_hex_long:
	push eax
	push eax
	push eax

	shr  eax,24
	call prt_hex_byte_386

	pop eax
	shr  eax,16
	call prt_hex_byte_386

	pop eax
	shr  eax,8
	call prt_hex_byte_386

	pop eax
	call prt_hex_byte_386

	ret
