; -----------------------------------------------------------------------------
; -----------------------------------------------------------------------------
; Print character / clear screen
; (Use nasm to assemble it)
; Jan-2024 Jean-François DEL NERO / HxC2001
; -----------------------------------------------------------------------------
; -----------------------------------------------------------------------------

[bits  32]

section .text

char_x_pos dd 0
char_y_pos dd 0

;------------------------------------------------------------------------------
; print char function
; al = character to print
global putc_386
putc_386:
; al = char,
	push edi
	push edx

	; Video Text Buffer
	mov edi, 0xB8000

	cmp al, 0x0D ; \r
	jne .l01     ; Carriage Return

	mov dword [ds:char_x_pos],0
	jmp .l03

.l01:

	cmp al, 0x0A ; \n
	jne .l02     ; Line Feed

	inc dword [ds:char_y_pos] ; Next line

	jmp .l03

.l02:

	cmp dword [ds:char_y_pos],25 ; Above last line ?
	jae .l03

	push eax

	; Compute memory offset
	mov eax,(80*2)
	mul dword [ds:char_y_pos] ; y * 80 * 2
	mov edx,[ds:char_x_pos]
	shl edx,1
	add eax,edx
	add edi,eax ; edi = (y * 80 * 2) + (x * 2)

	pop eax

	; write char
	mov byte [ds:edi],al

	; inc char position
	inc dword [ds:char_x_pos]

	cmp dword [ds:char_x_pos],80
	jnae .l03

	mov dword [ds:char_x_pos],0
	inc dword [ds:char_y_pos]

.l03:

	pop edx
	pop edi

	ret

global clear_screen_386
clear_screen_386:
	mov dword [ds:char_x_pos],0
	mov dword [ds:char_y_pos],0

	mov ecx,80*25
	mov  al,' '
.l01:
	call putc_386
	dec ecx
	jnz .l01

	mov dword [ds:char_x_pos],0
	mov dword [ds:char_y_pos],0

	ret
