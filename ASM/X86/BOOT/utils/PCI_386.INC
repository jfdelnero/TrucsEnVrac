; -----------------------------------------------------------------------------
; -----------------------------------------------------------------------------
; PCI access functions
; (Use nasm to assemble it)
; Sept-2024 Jean-François DEL NERO / HxC2001
; -----------------------------------------------------------------------------
; -----------------------------------------------------------------------------

[bits  32]

section .text

;------------------------------------------------------------------------------
; print pci devices
global print_pcidev
print_pcidev:

	mov ecx, 0x80000000
	mov ebx, 0

.loop_scan_pci:

	mov eax, ecx

	mov dx, 0xCF8
	out dx, eax

	mov dx, 0xCFC
	in  eax, dx

	cmp eax, 0xFFFFFFFF
	je  .not_valid

	push ecx
	push ebx

	push eax
	call prt_hex_short_386
	mov al,':'
	call putc_386
	pop eax
	shr eax,16

	call prt_hex_short_386

	mov al,' '
	call putc_386
	mov al,'-'
	call putc_386
	mov al,' '
	call putc_386

	mov eax, ecx
	call prt_hex_long_386

	mov esi,msg_return
	call puts_386

	pop ebx
	pop ecx

	add ebx,1

.not_valid:

	add ecx, 0x800
	cmp ecx, 0x80FFF800
	jbe .loop_scan_pci

	mov esi, msg_numberofpci
	call puts_386

	mov eax, ebx
	call prt_hex_long_386

	mov esi,msg_return
	call puts_386

	ret

	; ebx = device / vendor
global find_pcidev
find_pcidev:

	mov ecx, 0x80000000
.loop_scan_pci2:

	mov eax, ecx

	mov dx, 0xCF8
	out dx, eax

	mov dx, 0xCFC
	in  eax, dx

	cmp eax, ebx
	jne  .not_valid2

	ret

.not_valid2:

	add ecx, 0x800
	cmp ecx, 0x80FFF800
	jbe .loop_scan_pci2

	mov ecx, 0xFFFFFFFF

	ret
