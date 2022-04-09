; -----------------------------------------------------------------------------
; -----------------------------------------------------------------------------
; 8086 PC Boot sector.
; (Use nasm to assemble it)
; Jan-2022 Jean-François DEL NERO / HxC2001
; -----------------------------------------------------------------------------
; nasm bootsect.asm -o bootsect.img
; -----------------------------------------------------------------------------

	%define SECTOR_SIZE      0x200         ; 0x400 on NEC PC98, 0x200 on standard PC

	%define NUMBER_OF_TRACKS 80
	%define SECTOR_PER_TRACK 9
	%define HEADS_PER_TRACK  2

	%define DISK_TOTAL_SECTORS (NUMBER_OF_TRACKS*SECTOR_PER_TRACK*HEADS_PER_TRACK)

	%define KB_TO_READ       16

	%define TRACKS_TO_READ   ( ((KB_TO_READ*1024) / (SECTOR_PER_TRACK*HEADS_PER_TRACK*SECTOR_SIZE) + 1 ) )
	%define RSVD_SECTORS     SECTOR_PER_TRACK*HEADS_PER_TRACK*TRACKS_TO_READ

	%define BIOS_LOAD_OFFSET   0x7C00      ; BIOS Boot sector location 0x0000:0x7C00
	%define RELOC_BASE_OFFSET   0x600      ; Boot sector relocation to 0x0000:0x0600
	%define LOAD_BASE_OFFSET    0x800      ; Load start address set to 0x0000:0x0800

	[bits  16]

	org BIOS_LOAD_OFFSET

entry_point:

	jmp short boot_sector_code

;------------------------------------------------------------------------------
	times   3-($-$$)      db      0        ; align to offset 3

	; Minimal disk description
	db "HXC2001 "                          ; OEM name and version
	dw SECTOR_SIZE                         ; bytes per sector
	db 1                                   ; sectors per cluster
	dw RSVD_SECTORS                        ; number of reserved sectors (starting at 0)
	db 0                                   ; number of FAT's on disk
	dw 0                                   ; number of root directory entries (directory size)
	dw DISK_TOTAL_SECTORS                  ; number of total sectors
	db 0xF9                                ; media descriptor byte  (see MEDIA DESCRIPTOR)
	dw 0                                   ; sectors per FAT
	dw SECTOR_PER_TRACK                    ; sectors per track  (DOS 3.0+)
	dw HEADS_PER_TRACK                     ; number of heads  (DOS 3.0+)
	dw 0                                   ; number of hidden sectors  (DOS 3.0+)

;------------------------------------------------------------------------------

error_cnt db 5

;------------------------------------------------------------------------------

retry_loop:
	mov al,[cs:(RELOC_BASE_OFFSET + (error_cnt - BIOS_LOAD_OFFSET))]
	dec al
	mov [cs:(RELOC_BASE_OFFSET + (error_cnt - BIOS_LOAD_OFFSET))],al

	jnz short retry

	mov  si, (RELOC_BASE_OFFSET + (msg_error - BIOS_LOAD_OFFSET))
	call print_string_func

	; Stop -> "sleep mode"
looperror:
	hlt
	jmp looperror

;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
;------------------------------------------------------------------------------

boot_sector_code:

	; relocate the boot sector code to 0x0000:0x0600
	mov ax, cs                             ; es = ds = cs = 0x0000
	mov ds, ax
	mov es, ax
	mov di, RELOC_BASE_OFFSET              ; Destination
	mov si, BIOS_LOAD_OFFSET               ; Source
	mov cx, SECTOR_SIZE                    ; Size
	repnz movsb                            ; Copy...

	; Jump to the relocated code !
	jmp 0x0000:(RELOC_BASE_OFFSET + (relocated_code_entry - BIOS_LOAD_OFFSET))

;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
;------------------------------------------------------------------------------

relocated_code_entry:
    ; We are now located in the 0x0000:0x0600 memory block !

	; Print the startup message
	mov  si, RELOC_BASE_OFFSET + (msg_start - BIOS_LOAD_OFFSET)
	call print_string_func

retry:
	; Reset floppy drive
	xor ah,ah
	xor dh,dh
	int 13h
	jc  retry_loop

	; Now Read (TRACKS_TO_READ*SECTOR_PER_TRACK*HEADS_PER_TRACK) - 1 sectors
	; Destination buffer
	mov bx,LOAD_BASE_OFFSET

	xor dx,dx                              ; Head 0 (dh), drive A: (dl)
	xor ch,ch                              ; Start at track 0
	mov cl,2                               ; Sector 2
	mov al,[cs:RELOC_BASE_OFFSET+0x18]     ; SECTOR_PER_TRACK sectors to read for the first track
	dec al                                 ; SECTOR_PER_TRACK - 1
load_loop:
	mov ah,2                               ; Read Disk Sectors
	int 13h                                ; Call the BIOS disks services
	jc  retry_loop

	; Go to next side/track to read
	mov ah,0
	shl ax,9                               ; AX = Sectors read * SECTOR_SIZE
	add bx,ax                              ; Move buffer the offset
	add dh,[cs:RELOC_BASE_OFFSET+0x1A]     ; Next head (HEADS_PER_TRACK-1)
	dec dh
	and dh,1
	jnz same_track
	add ch,1                               ; Next track
	cmp ch,TRACKS_TO_READ                  ; All tracks read ?
	jge load_done
same_track:

	mov al,'.'
	call print_char_func

	mov al,[cs:RELOC_BASE_OFFSET+0x18]     ; SECTOR_PER_TRACK sectors to read

	mov cl,1                               ; Sector 1

	jmp load_loop

load_done:

	mov al,'.'
	call print_char_func

	mov si, RELOC_BASE_OFFSET + (msg_second_stage - BIOS_LOAD_OFFSET)
	call print_string_func

	; And finally jump to the loaded code...
	jmp 0x0000:LOAD_BASE_OFFSET

;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
;------------------------------------------------------------------------------

;------------------------------------------------------------------------------
; print char function
print_char_func:
	push ax
	push bx

	mov ah,0x0E                            ; Teletype output
	mov bh,0x00                            ; Page Number
	mov bl,0x07                            ; Color (only in graphic mode)

	int  0x10                              ; BIOS Videos services.

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
;------------------------------------------------------------------------------
msg_start          db "HxC2001 Boot sector started ! :) Loading : ",0
msg_error          db "Boot error ! :(",13,10,0
msg_second_stage   db 13,10,"Second stage Loaded :) -> Jump !",13,10,0
;------------------------------------------------------------------------------
;   sector padding + bootable sector magic number
	times   (SECTOR_SIZE-2)-($-$$)      db      0
	dw      0xAA55

;   End of the boot sector

;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
