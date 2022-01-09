; -----------------------------------------------------------------------------
; -----------------------------------------------------------------------------
; Minimal bootable PC disk image
; (Use nasm to assemble it)
; Jan-2022 Jean-François DEL NERO / HxC2001
; -----------------------------------------------------------------------------
; nasm boot.asm -o boot.img
; -----------------------------------------------------------------------------

	%define SECTOR_PER_TRACK 9
	%define HEADS_PER_TRACK  2
	%define TRACKS_TO_READ   3
	%define RSVD_SECTORS     SECTOR_PER_TRACK*HEADS_PER_TRACK*TRACKS_TO_READ

	org 0x7c00

	jmp short boot_sector_code

;------------------------------------------------------------------------------
	times   3-($-$$)      db      0  ; align to offset 3

	; Minimal disk description
	db "HXC2001 "        ; OEM name and version
	dw 0x200             ; bytes per sector
	db 1                 ; sectors per cluster
	dw RSVD_SECTORS      ; number of reserved sectors (starting at 0)
	db 0                 ; number of FAT's on disk
	dw 0                 ; number of root directory entries (directory size)
	dw 0                 ; number of total sectors
	db 0xF9              ; media descriptor byte  (see MEDIA DESCRIPTOR)
	dw 0                 ; sectors per FAT
	dw 0                 ; sectors per track  (DOS 3.0+)
	dw HEADS_PER_TRACK   ; number of heads  (DOS 3.0+)
	dw 0                 ; number of hidden sectors  (DOS 3.0+)

;------------------------------------------------------------------------------

error_cnt db 5

;------------------------------------------------------------------------------

retry_loop:
	mov al,[cs:error_cnt]
	dec al
	mov [cs:error_cnt],al

	jnz retry

	mov  si, msg_error
	call print_string_func

	; Stop -> "sleep mode"
looperror:
	hlt
	jmp looperror

;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
;------------------------------------------------------------------------------

boot_sector_code:

	; Print the startup message
	mov  si, msg_start
	call print_string_func

retry:

	; Reset floppy drive
	xor ah,ah
	xor dh,dh
	int 13h
	jc  retry_loop

	; Now Read (TRACKS_TO_READ*SECTOR_PER_TRACK*HEADS_PER_TRACK) - 1 sectors
	; Destination buffer
	mov bx,second_loader
	mov ax,cs
	mov es,ax

	xor dx,dx                   ; Head 0 (dh), drive A: (dl)
	xor ch,ch                   ; Start at track 0
	mov cl,2                    ; Sector 2
	mov al,(SECTOR_PER_TRACK-1) ; SECTOR_PER_TRACK-1 sectors to read for the first track
load_loop:
	mov ah,2                    ; Read Disk Sectors
	int 13h                     ; Call the BIOS disks services
	jc  retry_loop

	; Go to next side/track to read
	mov ah,0
	shl ax,9                    ; AX = Sectors read * 512
	add bx,ax                   ; Move buffer the offset
	add dh,(HEADS_PER_TRACK-1)  ; Next head
	and dh,1
	jnz same_track
	add ch,1                    ; Next track
	cmp ch,TRACKS_TO_READ       ; All tracks read ?
	jge load_done
same_track:

	mov al,'.'
	call print_char_func

	mov al,SECTOR_PER_TRACK     ; SECTOR_PER_TRACK sectors to read
	mov cl,1                    ; Sector 1
	jmp load_loop

load_done:

	mov al,'.'
	call print_char_func

	mov si, msg_second_stage
	call print_string_func

	jmp second_loader

;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
;------------------------------------------------------------------------------

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
;------------------------------------------------------------------------------
msg_start          db "HxC2001 Boot sector started ! :) Loading : ",0
msg_error          db "Boot error ! :(",13,10,0
msg_second_stage   db 13,10,"Second stage Loaded :) -> Jump !",13,10,0
;------------------------------------------------------------------------------
;   sector padding + bootable sector magic number
	times   510-($-$$)      db      0
	dw      0xAA55

;   End of the boot sector

;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
;------------------------------------------------------------------------------
; second stage loader code

second_loader:

	mov     si, msg_second_stage_hello
	call    print_string_func

	; Disable all interruptions
	cli
loopwait:
	hlt
	jmp loopwait

msg_second_stage_hello      db "I am the second stage ! :)",13,10,0

;   Floppy image padding
	times   737280-($-$$)   db      0xF6