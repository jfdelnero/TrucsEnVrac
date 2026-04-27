///////////////////////////////////////////////////////////////////////////////////
// File : unzx0_x64.s (x86_64 version - Work in progress !)
// Contains: ZX0 unpacker
//
// Written by: Jean-François DEL NERO
///////////////////////////////////////////////////////////////////////////////////

.intel_syntax noprefix
#.section        .note.GNU-stack,"",@progbits

.global zx0_unpack

#
# int zx0_unpack(unsigned char * output,unsigned char * packed);
# Calling convention : Arguments 1-6 are passed via registers RDI, RSI, RDX, RCX, R8, R9 respectively;
#
# RDI = Out buffer
# RSI = packed data
#

# -----------------------------

_get_next_bit:
	shr bl, 1
	jz get_bit_next_byte

	test ch, bl
	ret

get_bit_next_byte:
	mov bl, 0x80
	mov ch, ds:[rsi]
	mov cl, ch
	inc rsi

	test ch, bl
	ret
# -----------------------------

_read_interlaced_elias_gamma_backtrace:
	# rax = value = 1
	xor rax,rax
	inc rax

	test cl, 0x01
	jz _read_interlaced_elias_gamma_cont

	ret

_read_interlaced_elias_gamma:

	# rax = value = 1
	xor rax,rax
	inc rax

loop_elias_gamma:

	call _get_next_bit
	jnz end_loop_elias_gamma

_read_interlaced_elias_gamma_cont:

	add rax, rax #shl rax, 1

	call _get_next_bit

	jz   loop_elias_gamma_0

	or    al,  1

loop_elias_gamma_0:
	xor   al, r9b
	jmp   loop_elias_gamma

end_loop_elias_gamma:
	ret


# ----------------------------------------

zx0_unpack:
	# rax
	# rbx # bit_mask
	# rcx # cl = last_byte ch = last_byte_2
	# rdx = last_offset
	# rsi = packed data ptr
	# rdi = out ptr

#	push r11
#	push r10
#	push r9
#	push rsi
#	push rdi


	xor rdx,rdx
	inc rdx
	xor bl,  bl # bit_mask
	xor rcx, rcx # cl = last_byte
				 # ch = last_byte_2
	mov r10,rdi

copy_literals_loop:

#_copy_literals:

	xor  r9b,r9b
	call _read_interlaced_elias_gamma

	push rcx
	cld
	mov rcx,rax
	rep movsb
	pop rcx

#

	call _get_next_bit
	jnz  skip_001

#_copy_from_last_offset

	xor  r9b,r9b
	call _read_interlaced_elias_gamma

	mov  r11, rsi

	mov rsi, rdi
	sub rsi, rdx

	push rcx
	cld
	mov rcx,rax
	rep movsb
	pop rcx

	mov  rsi, r11

#

	call _get_next_bit

skip_001:
	jz  copy_literals_loop

copy_from_new_offset_loop:

# ----------------------------------------------
# _copy_from_new_offset:

	mov  r9b,1
	call _read_interlaced_elias_gamma

	cmp rax, 256
	je  _zx0_unpack_exit

	# offset = (offset<<7) - (get_next_byte(ctx)>>1);
	shl rax, 7

	xor r9,r9
	mov r9b, ds:[rsi]
	mov cl, r9b
	inc rsi

	shr r9, 1
	sub rax, r9
	mov r11,rax

	xor  r9b, r9b
	call _read_interlaced_elias_gamma_backtrace
	inc rax

	mov rdx,rcx
	mov rcx,rax

	mov rax, rsi

	mov rsi, rdi
	sub rsi, r11

	cld
	rep movsb

	mov rsi, rax

	mov rcx,rdx

	mov rdx,r11

# ----------------------------------------------

	# get_bit

	call _get_next_bit
	jnz  copy_from_new_offset_loop

	jmp copy_literals_loop

_zx0_unpack_exit:
	mov rax, rdi
	sub rax, r10

#	pop rdi
#	pop rsi
#	pop r9
#	pop r10
#	pop r11
	ret
