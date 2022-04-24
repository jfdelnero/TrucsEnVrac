; -----------------------------------------------------------------------------
; -----------------------------------------------------------------------------
; STAGE 2 PC boot strap
; (Use nasm to assemble it)
; Jan-2022 Jean-François DEL NERO / HxC2001
; -----------------------------------------------------------------------------
; nasm STAGE2.ASM -o STAGE2.BIN
; -----------------------------------------------------------------------------

	%define START_MEMORY_TEST   64*1024*1024
	%define MEMORY_TEST_SIZE    63*1024*1024

	[bits  16]

	org 0x800

;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
; second stage loader code

second_loader:
	; Here we are in real mode.
	mov     si, msg_second_stage_hello
	call    print_string_func

	call detect_cpu

	; Switch to a Flat / protected mode
	jmp switch_to_linear_mode

;------------------------------------------------------------------------------
; print char function
print_char_func:
	push ax
	push bx

	mov ah,0x0E                 ; Teletype output
	mov bh,0x00                 ; Page Number
	mov bl,0x07                 ; Color (only in graphic mode)

	int  0x10                   ; BIOS Videos services.

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

detect_cpu:

	pushf                          ; push original FLAGS
	pop     ax                     ; get original FLAGS
	mov     cx, ax                 ; save original FLAGS
	and     ax, 0xfff              ; clear bits 12-15 in FLAGS
	push    ax                     ; save new FLAGS value on stack
	popf                           ; replace current FLAGS value
	pushf                          ; get new FLAGS
	pop     ax                     ; store new FLAGS in AX
	and     ax, 0xf000             ; if bits 12-15 are set, then
	cmp     ax, 0xf000             ;   processor is an 8086/8088
	mov     byte [cs:cpu_type], 0  ; turn on 8086/8088 flag
	je      end_cpu_type           ; jump if processor is 8086/8088

	or      cx, 0xf000             ; try to set bits 12-15
	push    cx                     ; save new FLAGS value on stack
	popf                           ; replace current FLAGS value
	pushf                          ; get new FLAGS
	pop     ax                     ; store new FLAGS in AX
	and     ax, 0xf000             ; if bits 12-15 are clear
	mov     byte [cs:cpu_type], 2  ; processor=80286, turn on 80286 flag
	jz      end_cpu_type           ; if no bits set, processor is 80286

	pushfd                         ; push original EFLAGS
	pop     eax                    ; get original EFLAGS
	mov     ecx, eax               ; save original EFLAGS
	xor     eax, 0x40000           ; flip AC bit in EFLAGS
	push    eax                    ; save new EFLAGS value on stack
	popfd                          ; replace current EFLAGS value

	pushfd                         ; get new EFLAGS
	pop     eax                    ; store new EFLAGS in EAX
	xor     eax, ecx               ; can't toggle AC bit, processor=80386
	mov     byte [cs:cpu_type], 3  ; turn on 80386 processor flag
	jz      end_cpu_type           ; jump if 80386 processor
	push    ecx
	popfd                          ; restore AC bit in EFLAGS first

	mov     byte [cs:cpu_type], 4  ; turn on 80486 processor flag
	mov     eax, ecx               ; get original EFLAGS
	xor     eax, 200000h           ; flip ID bit in EFLAGS
	push    eax                    ; save new EFLAGS value on stack
	popfd                          ; replace current EFLAGS value
	pushfd                         ; get new EFLAGS
	pop     eax                    ; store new EFLAGS in EAX
	xor     eax, ecx               ; can't toggle ID bit,
	je      end_cpu_type           ; processor=80486

	;Here CPUID is supported !
	mov     byte [cs:cpu_type], 5  ; Pentium or any other CPUID compatible CPU

	mov     eax,0
	cpuid
	mov dword [cs:cpuid_manufacturer], ebx
	mov dword [cs:cpuid_manufacturer+4], edx
	mov dword [cs:cpuid_manufacturer+8], ecx

	mov     eax,1
	cpuid
	mov dword [cs:cpuid_id], eax

end_cpu_type:
	ret

align 8

; -----------------------------------------------------------------------------
; Global Descriptor Table (GDT)
; (Protected mode segments descriptors)

; We define here 2 4GB Code and Data flat segment

gdt_table:
	; First descriptor : Should be null/disable
	dw 0x0000		; segment limit (bits 0-15)
	dw 0			; base (bits 0-15)
	db 0			; base (bits 16-23)
	db 0x00			; Access byte
	db 00000000b	; Flags (4 bits) + segment limit (bits 16-20)
	db 0			; Base (bits 24-31)

; ---- offset 0x8 ----
gdt_code_entry:
	; Second descriptor : Code segment descriptor
	; cs must point to this descriptor in pm mode
	dw 0xffff		; segment limit (bits 0-15)
	dw 0			; base (bits 0-15)
	db 0			; base (bits 16-23)
	db 0x9a			; Access byte
	db 11001111b	; Flags (4 bits) + segment limit (bits 16-20)
	db 0			; Base (bits 24-31)

; offset 0x10
gdt_data_entry:
	; Third descriptor : Data segment descriptor
	; ds, ss, es, fs, and gs should point to this descriptor in pm mode
	dw 0xffff		; segment limit (bits 0-15)
	dw 0			; base (bits 0-15)
	db 0			; base (bits 16-23)
	db 0x92			; Access byte
	db 11001111b	; Flags (4 bits) + segment limit (bits 16-20)
	db 0			; Base (bits 24-31)
gdt_end:

; GDT descriptor entry used by the lgdt instruction
gdt_descriptor:
dw  (gdt_end  - gdt_table)  - 1  ; Global descriptor table size - 1
dq  gdt_table                    ; Global descriptor table offset

; Code and Data segment offset definition
CODE_SEG  equ  gdt_code_entry  - gdt_table
DATA_SEG  equ  gdt_data_entry  - gdt_table

;--------------------------------------------------------------
;--------------------------------------------------------------
;--------------------------------------------------------------

switch_to_linear_mode:

	; Disable all interruptions
	; (Once switched to PM mode real mode interruptions can't be used anymore !)
	cli

	; Load the Global Descriptor Table !
	o32 lgdt [cs:gdt_descriptor]

	; And switch to protected mode !
	mov eax , cr0
	or  eax , 0x1           ; Protected mode flag
	mov cr0 , eax

	; And finaly jump to the new code segment !
	; (This operation should also flush the instructions prefetch & cache)
	jmp CODE_SEG:(code_32bit_mode)

align 8

code_32bit_mode:
[bits  32]
	; Welcome to the Protected Mode ! :)

	; All data related segment should now point to the new
	; data segment defined in the Global Descriptor Table
	mov ax, DATA_SEG
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	; Setup the stack
	mov ebp , 0x50000
	mov esp , ebp

	; Enable the A20 line
	; The address line n°20 is disconnected by default
	; to emulate the pre-386 memory mirror behavior.
	; This is of course not desired here and we need enable it.
	in  al, 0x92      ; Fast Gate A20 (Note : older alternate method :through the keyboard controller)
	or  al, 0x02
	out 0x92, al

	; At this point the memory

	call clear_screen

	mov esi, msg_second_stage_flat
	call write_string

	mov esi, msg_cpuvendor
	call write_string

	mov esi,msg_cpu8086
	mov al,[cs:cpu_type]
	cmp al,0
	je  print_cputype

	mov esi,msg_cpu80286
	mov al,[cs:cpu_type]
	cmp al,2
	je  print_cputype

	mov esi,msg_cpu80386
	mov al,[cs:cpu_type]
	cmp al,3
	je  print_cputype

	mov esi,msg_cpu80486
	mov al,[cs:cpu_type]
	cmp al,4
	je  print_cputype

	mov esi,msg_cpuid
	call write_string
	mov eax, [cs:cpuid_id]
	call print_hex_long
	mov esi,msg_return

print_cputype:
	call write_string

	mov edi,START_MEMORY_TEST
	mov ebx,0
	mov edx,0x32340C
	CLD
loop_0001:

	mov eax,ebx
	mov dword [ds:char_x_pos],0
	mov dword [ds:char_y_pos],5
	call print_hex_long

	mov eax,0x0000FFFF
	mov dword [ds:char_x_pos],0
	mov dword [ds:char_y_pos],4
	call print_hex_long

	mov edi,START_MEMORY_TEST
	mov ecx,(MEMORY_TEST_SIZE)/4
	rep stosd

	;mov eax,edx
	;call random_mem
	;mov edx,eax

	mov eax,0x00000000
	mov dword [ds:char_x_pos],0
	mov dword [ds:char_y_pos],4
	call print_hex_long

	mov edi,START_MEMORY_TEST
	mov ecx,(MEMORY_TEST_SIZE)/4
	rep stosd

	;mov eax,edx
	;call random_mem
	;mov edx,eax

	dec ebx
	cmp ebx,0
	jmp loop_0001

	hlt

	call random_mem

	mov esi, 2*1024*1024

loop_copy:
	call copy_video_memory
	call random_mem

	add esi,320

	cmp esi,(4*1024*1024) - 64000
	jna loop_copy

	mov esi, 2*1024*1024

	jmp loop_copy

	hlt

mainloopscreen:

	mov eax,0xA0000
	mov edi,eax
	mov esi,0

loopscreen:

	mov ebx,ecx
	shl eax,13
	xor eax,ebx

	mov ebx,eax
	shr eax,17
	xor eax,ebx

	mov ebx,eax
	shl eax,5
	xor eax,ebx

	inc ecx

	mov [es:edi],eax

	add edi,4
	cmp edi,(0xA0000+64000)
	jne loopscreen

jmp mainloopscreen

	;hlt
	jmp $

	ret

char_x_pos dd 0
char_y_pos dd 0

; al = char,
write_char:
	push edi
	push edx

	mov edi, 0xB8000

	cmp al, 0x0D ; \r
	jne .l01
	mov dword [ds:char_x_pos],0
	jmp .l03
.l01:

	cmp al, 0x0A ; \n
	jne .l02
	inc dword [ds:char_y_pos]
	jmp .l03
.l02:

	cmp dword [ds:char_y_pos],25
	jae .l03

	push eax

	mov eax,(80*2)
	mul dword [ds:char_y_pos]
	mov edx,[ds:char_x_pos]
	shl edx,1
	add eax,edx
	add edi,eax

	pop eax

	mov byte [ds:edi],al

	inc dword [ds:char_x_pos]
	cmp dword [ds:char_x_pos],80
	jnae .l03
	mov dword [ds:char_x_pos],0
	inc dword [ds:char_y_pos]
.l03:

	pop edx
	pop edi

	ret

write_string:
	push esi
	push eax
.l01:
	mov al,[ds:esi]
	cmp al,0
	je .l02
	call write_char
	inc esi
	jmp .l01
.l02:
	pop eax
	pop esi
	ret

clear_screen:
	mov dword [ds:char_x_pos],0
	mov dword [ds:char_y_pos],0

	mov ecx,80*25
	mov  al,' '
.l01:
	call write_char
	dec ecx
	jnz .l01

	mov dword [ds:char_x_pos],0
	mov dword [ds:char_y_pos],0

	ret


print_hex_quartet:

	push ax

	and al,0x0F

	cmp al,0xA
	jnae .l01

	sub al,0xA

	add al,'A'
	call write_char

	pop ax

	ret

.l01:
	add al,'0'
	call write_char

	pop  ax

	ret

print_hex_byte:

	push ax
	shr al,4
	call print_hex_quartet
	pop ax
	call print_hex_quartet

	ret

print_hex_long:
	push eax
	push eax
	push eax

	shr  eax,24
	call print_hex_byte

	pop eax
	shr  eax,16
	call print_hex_byte

	pop eax
	shr  eax,8
	call print_hex_byte

	pop eax
	call print_hex_byte
	ret

set_color:
	push edx
	push eax

	push eax
	mov dx,0x3C6
	mov al,0xFF
	out dx,al
	pop eax

	mov dx,0x3C8
	out dx,al

	shr eax,8
	mov dx,0x3C9
	out dx,al

	shr eax,8
	mov dx,0x3C9
	out dx,al

	shr eax,8
	mov dx,0x3C9
	out dx,al

	pop eax
	pop edx

	ret

init_pal:
	push eax

	mov eax,0x00000000
set_col_loop:
	call set_color
	add eax,0x01030701

	cmp al,0
	jne set_col_loop

	pop eax

	ret

fill_video_memory:

	push eax
	push edi

	mov edi,0xA0000

loop_fill_screen:

	mov [es:edi],ebx
	add edi,4
	mov [es:edi],ebx
	add edi,4
	mov [es:edi],ebx
	add edi,4
	mov [es:edi],ebx
	add edi,4

	cmp edi,(0xA0000+64000)
	jne loop_fill_screen

	pop edi
	pop eax

	ret

fill_memory:

	push eax
	push edi
	push ecx

	mov     eax,ebx
	rep stosd

	pop ecx
	pop edi
	pop eax

	ret


copy_video_memory: ; esi : src

	push ebx
	push edi
	push esi

	mov edi,0xA0000

loop_copy_video_memory:
	mov ebx,[es:esi]
	mov [es:edi],ebx
	add esi,4
	add edi,4

	cmp edi,(0xA0000+64000)
	jne loop_copy_video_memory

	pop esi
	pop edi
	pop ebx

	ret

random_mem:
	push edi
	push ebx

	mov edi,START_MEMORY_TEST

main_loop_random_mem:

	mov ebx,eax
	shl eax,13
	xor eax,ebx

	mov ebx,eax
	shr eax,17
	xor eax,ebx

	mov ebx,eax
	shl eax,5
	xor eax,ebx

	and eax,0x0000FFFF
	mov [ds:edi],eax

	add edi,4
	cmp edi,START_MEMORY_TEST + MEMORY_TEST_SIZE
	jne main_loop_random_mem

	pop ebx
	pop edi
	ret

msg_second_stage_hello      db "I am the second stage ! :)",13,10,0
msg_second_stage_flat       db "Protected mode enabled !",13,10,0
cpu_type                    db 0
msg_return                  db 13,10,0
msg_cpuid                   db "CPU ID: 0x",0
msg_cpu8086                 db "8086 CPU",13,10,0
msg_cpu80286                db "286 CPU",13,10,0
msg_cpu80386                db "386 CPU",13,10,0
msg_cpu80486                db "486 CPU",13,10,0
msg_cpuvendor               db "Vendor ID: "
cpuid_manufacturer          db "            ",13,10,0
cpuid_id                    dd 0
