; -----------------------------------------------------------------------------
; -----------------------------------------------------------------------------
; x86 CPU identification routine.
; Original code from Intel Application Note AP-485.
; "Intel Processor Identification With the CPUID Instruction"
; (Use nasm to assemble it)
; Jan-2024 Jean-François DEL NERO / HxC2001
; -----------------------------------------------------------------------------
; -----------------------------------------------------------------------------

[bits  16]

section .text

;------------------------------------------------------------------------------
; get cpu type
global detect_cpu
detect_cpu:
	push ax
	push cx

	; Intel 8086 processor check
	; Bits 12-15 of the FLAGS register are always set on the
	; 8086 processor.
	;.8086
.check_8086:
	pushf                          ; push original FLAGS
	pop     ax                     ; get original FLAGS
	mov     cx, ax                 ; save original FLAGS
	and     ax, 0xfff              ; clear bits 12-15 in FLAGS
	push    ax                     ; save new FLAGS value on stack
	popf                           ; replace current FLAGS value
	pushf                          ; get new FLAGS
	pop     ax                     ; store new FLAGS in AX
	and     ax, 0xf000             ; if bits 12-15 are set, then
	cmp     ax, 0xf000             ; processor is an 8086/8088
	mov     byte [cs:cpu_type], 0  ; turn on 8086/8088 flag
	je      .end_cpu_type          ; jump if processor is 8086/8088

	; Intel 286 processor check
	; Bits 12-15 of the FLAGS register are always clear on the
	; Intel 286 processor in real-address mode.
	;.286
.check_80286:
	or      cx, 0xf000             ; try to set bits 12-15
	push    cx                     ; save new FLAGS value on stack
	popf                           ; replace current FLAGS value
	pushf                          ; get new FLAGS
	pop     ax                     ; store new FLAGS in AX
	and     ax, 0xf000             ; if bits 12-15 are clear
	mov     byte [cs:cpu_type], 2  ; processor=80286, turn on 80286 flag
	jz      .end_cpu_type          ; if no bits set, processor is 80286

	; Intel386 processor check
	; The AC bit, bit #18, is a new bit introduced in the EFLAGS
	; register on the Intel486 processor to generate alignment
	; faults.
	; This bit cannot be set on the Intel386 processor.
	;.386 ; it is safe to use 386 instructions
.check_80386:
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
	jz      .end_cpu_type          ; jump if 80386 processor
	push    ecx
	popfd                          ; restore AC bit in EFLAGS first

	; Intel486 processor check
	; Checking for ability to set/clear ID flag (Bit 21) in EFLAGS
	; which indicates the presence of a processor with the CPUID
	; instruction
	;.486
.check_80486:
	mov     byte [cs:cpu_type], 4  ; turn on 80486 processor flag
	mov     eax, ecx               ; get original EFLAGS
	xor     eax, 200000h           ; flip ID bit in EFLAGS
	push    eax                    ; save new EFLAGS value on stack
	popfd                          ; replace current EFLAGS value
	pushfd                         ; get new EFLAGS
	pop     eax                    ; store new EFLAGS in EAX
	xor     eax, ecx               ; can't toggle ID bit,
	je      .end_cpu_type          ; processor=80486

	; Here CPUID is supported !
	mov     byte [cs:cpu_type], 5  ; Pentium or any other CPUID compatible CPU

	mov     eax,0
	cpuid
	mov dword [cs:cpuid_manufacturer], ebx
	mov dword [cs:cpuid_manufacturer+4], edx
	mov dword [cs:cpuid_manufacturer+8], ecx

	mov     eax,1
	cpuid
	mov dword [cs:cpuid_id], eax

.end_cpu_type:

	pop cx
	pop ax

	ret
