; -----------------------------------------------------------------------------
; -----------------------------------------------------------------------------
; STAGE 2 PC boot strap
; (Use nasm to assemble it)
; Jan-2022 Jean-François DEL NERO / HxC2001
; -----------------------------------------------------------------------------
; nasm STAGE2.ASM -o STAGE2.BIN
; -----------------------------------------------------------------------------

	[bits  16]

	org 0x800

;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
; second stage loader code

second_loader:
	; Here we are in real mode.
	mov     si, msg_second_stage_hello
	call    puts_bios

	call detect_cpu

	; Running on a 386 or up ?
	cmp byte cs:[cpu_type], 3
	; Switch to a Flat / protected mode
	jae switch_to_linear_mode

	mov     si, msg_second_stage_386
	call  	puts_bios

.haltloop:
	hlt
	jmp .haltloop

	%include 'PUTC_BIOS.INC'
	%include 'PUTS_BIOS.INC'
	%include 'CPU_ID.INC'

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

	call clear_screen_386

	call init_timer

	mov esi, msg_second_stage_flat
	call puts_386

;--------------------------------------
; Print CPU type
;--------------------------------------
	xor eax,eax
	mov al,[cs:cpu_type]

	cmp al, 5
	jae .print_cpuid

	mov esi, msg_cpu
	call puts_386

	shl eax,2
	mov esi, cpu_str_list
	add esi,eax
	mov esi,[cs:esi]
	call puts_386

	jmp  .print_cputype_done

.print_cpuid:

	mov esi, msg_cpuvendor
	call puts_386

	mov esi, cpuid_manufacturer
	call puts_386

	mov esi,msg_cpuid
	call puts_386

	mov eax, [cs:cpuid_id]
	call prt_hex_long_386

	mov esi,msg_return
	call puts_386

.print_cputype_done:
;-------------------------------------------------------------------

	;%include 'PRG.INC'

	mov esi,msg_second_stage_halted
	call puts_386

.loop_halt_cpu:
	hlt
	jmp .loop_halt_cpu
;-------------------------------------------------------------------

	%include 'PUTC_386.INC'
	%include 'PUTS_386.INC'
	%include 'PRINT_HEX_386.INC'
	%include 'TIMER_386.INC'

msg_second_stage_hello      db "I am the second stage ! :)",13,10,0
msg_second_stage_386        db "386 CPU or up required ! - halted...",13,10,0
msg_second_stage_halted     db "- Halted -",13,10,0
msg_second_stage_flat       db "Protected mode enabled !",13,10,0
msg_return                  db 13,10,0

msg_cpu                     db "CPU : ",0
msg_cpu8086                 db "8086 CPU",13,10,0
msg_cpu80186                db "186 CPU",13,10,0
msg_cpu80286                db "286 CPU",13,10,0
msg_cpu80386                db "386 CPU",13,10,0
msg_cpu80486                db "486 CPU",13,10,0
msg_cpu80586                db "586 CPU",13,10,0

align 4
cpu_str_list                dd msg_cpu8086
							dd msg_cpu80186
							dd msg_cpu80286
							dd msg_cpu80386
							dd msg_cpu80486
							dd msg_cpu80586
							dd 0

msg_cpuvendor               db "Vendor ID: ",0
msg_cpuid                   db "CPU ID: 0x",0
cpuid_manufacturer          db "            ",13,10,0

cpu_type                    db 0
cpuid_id                    dd 0
