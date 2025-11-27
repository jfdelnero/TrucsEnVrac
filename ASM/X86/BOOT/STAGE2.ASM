; -----------------------------------------------------------------------------
; -----------------------------------------------------------------------------
; STAGE 2 PC boot strap
; (Use nasm to assemble it)
; Jan-2022 Jean-François DEL NERO / HxC2001
; -----------------------------------------------------------------------------
; nasm STAGE2.ASM -o STAGE2.BIN
; -----------------------------------------------------------------------------

	[bits  16]

	org 0x1000

;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
; second stage loader code entry point

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
	call    puts_bios

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
	dw 0x0000       ; segment limit (bits 0-15)
	dw 0            ; base (bits 0-15)
	db 0            ; base (bits 16-23)
	db 0x00         ; Access byte
	db 00000000b    ; Flags (4 bits) + segment limit (bits 16-20)
	db 0            ; Base (bits 24-31)

; ---- offset 0x8 ----
gdt_code_entry:
	; Second descriptor : Code segment descriptor
	; cs must point to this descriptor in pm mode
	dw 0xffff       ; segment limit (bits 0-15)
	dw 0            ; base (bits 0-15)
	db 0            ; base (bits 16-23)
	db 0x9a         ; Access byte
	db 11001111b    ; Flags (4 bits) + segment limit (bits 16-20)
	db 0            ; Base (bits 24-31)

; offset 0x10
gdt_data_entry:
	; Third descriptor : Data segment descriptor
	; ds, ss, es, fs, and gs should point to this descriptor in pm mode
	dw 0xffff       ; segment limit (bits 0-15)
	dw 0            ; base (bits 0-15)
	db 0            ; base (bits 16-23)
	db 0x92         ; Access byte
	db 11001111b    ; Flags (4 bits) + segment limit (bits 16-20)
	db 0            ; Base (bits 24-31)
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

;
; |--------------------------------------------------------|
; |PC Memory MAP (32 bits)                                 |
; |--------------------------------------------------------|
; |0x00000000 - 0x0009FFFF ; 640 KB  - RAM   (160 pages)   |
; |0x000A0000 - 0x000BFFFF ; 128 KB  - Video (32 pages)    | 1 MB
; |0x000C0000 - 0x000FFFFF ; 256 KB  - ROMs  (64 pages)    |
; |--------------------------------------------------------|
; |0x00100000 - 0x00EFFFFF ; 14 MB   - RAM   (3584 pages)  |
; |0x00F00000 - 0x00FFFFFF ; 1 MB    - ISA hole (256 pages)|
; |0x01000000 - 0xBFFFFFFF ; 3056 MB - RAM   (782336 pages)|
; |0xC0000000 - 0xFFFFFFFF ; 1 GB    - PCI   (262144 pages)|
; |--------------------------------------------------------|
;

%define PAGE_PRESENT      (0x1<<0)
%define PAGE_RW           (0x1<<1)
%define PAGE_USER         (0x1<<2)
%define PAGE_WRITETHROUGH (0x1<<3)
%define PAGE_DISABLECACHE (0x1<<4)

%define TEST_MEM_BASE     (2*1024*1024)
%define TEST_MEM_KBSIZE   (8)

switch_to_linear_mode:

	; Disable all interruptions
	; (Once switched to PM mode real mode interruptions can't be used anymore !)
	cli

%ifdef NEXGEN_SUPPORT
	; Nexgen IDOn (486 mode)
	mov si,0xB00E  ; Function E
	mov al,0x03    ; sub function 3
	mov ah,0x4C
	db 0x0F,0x55

	mov si,0xB001 ; switch cache
	mov al,0x01   ; on
	db 0x0F,0x55

	mov si,0xB002 ; switch L2 cache
	mov al,0x01   ; on
	db 0x0F,0x55
%endif

	; Enable the A20 line
	; The address line n°20 is disconnected by default
	; to emulate the pre-386 memory mirror behavior.
	; This is of course not desired here and we need enable it.
	in  al, 0x92      ; Fast Gate A20 (Note : older alternate method :through the keyboard controller)
	or  al, 0x02
	out 0x92, al

	; Load the Global Descriptor Table !
	o32 lgdt [cs:gdt_descriptor]

	;mov eax, cr4 ; Enable Page Size Extension (PSE) (>= Pentium)
	;or eax, 0x00000010
	;mov cr4, eax

	; [31:PG Paging][30:CD Cache disable][29:NW Not-write through][18:AM Alignment mask][16:WP Write protect][5:NE Numeric error][4:ET Extension type][3:TS Task switched][2:EM x87 FPU Emulation][1:MP Monitor co-processor][0:PE Protected Mode Enable]

	; And switch to protected mode !
	mov eax , cr0
	or  eax , 0x00000001    ; Protected mode flag
%ifdef NO_CACHE
	or  eax , 0x40000000    ; disable cache
%else
	and eax , 0xBFFFFFFF    ; enable cache
%endif
	mov cr0 , eax

	; And finaly jump to the new code segment !
	; (This operation should also flush the instructions prefetch & cache)
	jmp CODE_SEG:(code_32bit_protected_mode)

align 8

[bits  32]
code_32bit_protected_mode:
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

	mov eax , cr0
	call prt_hex_long_386

;------------------------------------------------------------------------
; Fill the page directory buffer

	mov esi, page_directory
	; [31-12:Address] [11-8:Available][7:Page Size][6:Available][5:Accessed][4:Cache Disable][3:Write Throught][2:User/Supervisor][1:Read/write][0:Present]

	mov eax, page_table
%ifdef NO_CACHE
	or  eax, ( PAGE_USER | PAGE_RW | PAGE_PRESENT ) | (PAGE_DISABLECACHE|PAGE_WRITETHROUGH)
%else
	or  eax, ( PAGE_USER | PAGE_RW | PAGE_PRESENT )
%endif

	mov ecx, 4 ; 4096
.fill_dir:
	mov ds:[esi], eax
	add eax, 4096; (1024*4096) ; 1024 entries * 4096kB : 4MB
	add esi,4
	dec ecx
	jnz .fill_dir

;------------------------------------------------------------------------
; Fill the page table buffer

	mov esi, page_table
%ifdef NO_CACHE
	mov eax, ( PAGE_USER | PAGE_RW | PAGE_PRESENT ) | (PAGE_DISABLECACHE|PAGE_WRITETHROUGH)
%else
	mov eax, ( PAGE_USER | PAGE_RW | PAGE_PRESENT )
%endif

	mov ecx, 1024*4
.fill_page:
	mov ds:[esi], eax
	add eax, 4096  ; 4kB per page...
	add esi,4
	dec ecx
	jnz .fill_page

;------------------------------------------------------------------------

	; Tell where is the page directory
	mov eax, page_directory
	and eax, 0xFFFFF000
	or  eax, 0x00000000
	mov cr3, eax

	; And switch to page mode !
	mov eax , cr0
	or  eax , 0x80000000    ; Set the Page mode flag
	and eax , 0xBFFFFFFF    ; Enable cache
	mov cr0 , eax

	jmp CODE_SEG:code_32bit_paging_mode

align 8
;-------------------------------------------------------------------------

code_32bit_paging_mode:
[bits  32]
	; Welcome to the Page Mode ! :)

	call clear_screen_386

	call init_timer

	mov esi, msg_second_stage_flat
	call puts_386

;   mov esi, page_directory
;   mov ecx, 512
;   call prt_hexbuf_386
;haltloop1:
;   hlt
;   jmp haltloop1

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

	mov eax , cr0
	call prt_hex_long_386

	mov esi,msg_return
	call puts_386

.print_cputype_done:
;-------------------------------------------------------------------

;   %include 'PRG.INC'

;   mov esi,msg_second_stage_halted
;   call puts_386

;.loop_halt_cpu:
;   hlt
;   jmp .loop_halt_cpu


; Xorshift32 memory filling and checking loop

	xor edx, edx
	xor ecx, ecx  ; loops counter
loop_memtest:

	; Get the timer current tick value
	call get_curtick
	mov ds:[str_tick],ax

	; xorshift32 pattern filling
	mov edi, TEST_MEM_BASE
	mov eax, 0xAC392812
	add eax, ecx

.loop_fill:
	mov ebx,eax
	shl eax,13
	xor eax,ebx

	mov ebx,eax
	shr eax,17
	xor eax,ebx

	mov ebx,eax
	shl eax,5
	xor eax,ebx

	mov [ds:edi],eax

	add edi, 4

	cmp edi,  (TEST_MEM_BASE) + (TEST_MEM_KBSIZE*1024)
	jne .loop_fill

	; xorshift32 pattern cheching

	mov edi, TEST_MEM_BASE
	mov eax, 0xAC392812
	add eax, ecx

.loop_check:
	mov ebx,eax
	shl eax,13
	xor eax,ebx

	mov ebx,eax
	shr eax,17
	xor eax,ebx

	mov ebx,eax
	shl eax,5
	xor eax,ebx

	mov ebx,[ds:edi]

	cmp ebx,eax
	je .noerror
	add edx,1
.noerror:
	add edi, 4

	cmp edi, (TEST_MEM_BASE) + (TEST_MEM_KBSIZE*1024)
	jne .loop_check

	; print duration
	xor eax, eax
	mov ds:[char_x_pos], eax
	mov eax, 20
	mov ds:[char_y_pos], eax

	xor eax,eax
	mov ax,ds:[str_tick]
	call get_duration
	call print_hex_long

	; print loop counter
	xor eax, eax
	mov ds:[char_x_pos], eax
	mov eax, 18
	mov ds:[char_y_pos], eax

	add ecx,1

	mov eax,ecx
	call print_hex_long

	; print errors counter
	xor eax, eax
	mov ds:[char_x_pos], eax
	mov eax, 19
	mov ds:[char_y_pos], eax
	mov eax,edx
	call print_hex_long

	jmp loop_memtest

;-------------------------------------------------------------------

	%include 'PUTC_386.INC'
	%include 'PUTS_386.INC'
	%include 'PRINT_HEX_386.INC'
	%include 'TIMER_386.INC'

msg_cpuvendor               db "Vendor ID: ",0
msg_cpuid                   db "CPU ID: 0x",0
cpuid_manufacturer          db "            ",13,10,0

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

align 4
cpuid_id                    dd 0
str_tick                    dw 0
cpu_type                    db 0


;pages_config dd 0x00000000 | ( PAGE_USER | PAGE_RW | PAGE_PRESENT ), 160,
;            dd 0x000A0000 | ( PAGE_DISABLECACHE | PAGE_WRITETHROUGH | PAGE_USER | PAGE_RW | PAGE_PRESENT ), 32,
;            dd 0x000C0000 | ( PAGE_WRITETHROUGH | PAGE_USER | PAGE_RW | PAGE_PRESENT ), 64,
;            dd 0x00100000 | ( PAGE_USER | PAGE_RW | PAGE_PRESENT ), 3584,
;            dd 0x00F00000 | ( PAGE_DISABLECACHE | PAGE_USER | PAGE_RW | PAGE_PRESENT ), 256,
;            dd 0x01000000 | ( PAGE_USER | PAGE_RW | PAGE_PRESENT ), 12288


align 4096

; CR3 : [31-12:Page Directory Address] [4:Cache Disable][3:Write Throught]
;
; CR3 -> Page Directory : 1024 * 4 :
;       [31-12:Page table address] [11-8:Available][7:Page Size][6:Available][5:Accessed][4:Cache Disable][3:Write Throught][2:User/Supervisor][1:Read/write][0:Present]
;
; Page table entry:
;       [21-12:Address] [11-8:Available][7:Page Size][6:Available][5:Accessed][4:Cache Disable][3:Write Throught][2:User/Supervisor][1:Read/write][0:Present]
;
;
;  Virtual address : [31-22][21-12][11-0]
;                       |      |      |
;      Offset in the page      |      |
;          directory           |      |
;        (1024 index)          |      |
;                              |      |
;             Offset in the page      |
;                   table             |
;               (1024 index)          |
;                                     |
;                             Offset in the page
;
; CR3 target the page directory
; The page directory contains pages tables addresses
; The page table contains physical addresses (top 20/[31-12] most significant bits)
; bits 11-0 is the offset in the page.
;

page_directory:

db 4096 dup(0)

align 4096

page_table:

db 16384 dup(0)

dd 0xEFBEADDE
