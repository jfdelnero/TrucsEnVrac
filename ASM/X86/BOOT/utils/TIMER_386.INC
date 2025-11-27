; -----------------------------------------------------------------------------
; -----------------------------------------------------------------------------
; Timer functions
; (Use nasm to assemble it)
; Sept-2024 Jean-François DEL NERO / HxC2001
; -----------------------------------------------------------------------------
; -----------------------------------------------------------------------------

[bits  32]

section .text

;
; PIT Clock : 1193181.6 Hz  (14318179,2/12)
; Min tick rate :
; 1/ ( (1/1193182) * 65536 ) = 18,206506204 Hz
;

;
; PC PIT Wiring :
;
; Channel 0 -> Connected to IRQ 0.
; Channel 1 -> DRAM Refresh.
; Channel 2 -> PC Speaker - port 61h bit 1 : speaker enable. bit 0 : timer gate output (enable out change).
;                                    bit 5 : PIT output read back (! On XT Machines this bit is in the port 62h !)
;
;Bits         Usage
;6 and 7      Select channel :
;                0 0 = Channel 0
;                0 1 = Channel 1
;                1 0 = Channel 2
;                1 1 = Read-back command (8254 only)
;4 and 5      Access mode :
;                0 0 = Latch count value command
;                0 1 = Access mode: lobyte only
;                1 0 = Access mode: hibyte only
;                1 1 = Access mode: lobyte/hibyte
;1 to 3       Operating mode :
;                0 0 0 = Mode 0 (interrupt on terminal count)
;                0 0 1 = Mode 1 (hardware re-triggerable one-shot)
;                0 1 0 = Mode 2 (rate generator)
;                0 1 1 = Mode 3 (square wave generator)
;                1 0 0 = Mode 4 (software triggered strobe)
;                1 0 1 = Mode 5 (hardware triggered strobe)
;                1 1 0 = Mode 2 (rate generator, same as 010b)
;                1 1 1 = Mode 3 (square wave generator, same as 011b)
;0            BCD/Binary mode: 0 = 16-bit binary, 1 = four-digit BCD
;

;------------------------------------------------------------------------------
; timer init
global init_timer
init_timer:
	push ax
	push cx

	countdown  equ  0000h ; approx 36 interrupts per second

	mov al,00110110b    ; bit 7,6 = (00) timer counter 0
						; bit 5,4 = (11) write LSB then MSB
						; bit 3-1 = (011) generate square wave
						; bit 0 = (0) binary counter
	out 43h,al          ; prep PIT, counter 0, square wave&init count
	nop
	mov cx,countdown    ; default is 0x0000 (65536) (18.2 per sec)

	; interrupts when counter decrements to 0
	mov al,cl           ; send LSB of timer count
	out 40h,al
	nop

	mov al,ch           ; send MSB of timer count
	out 0x40,al
	nop

	pop cx
	pop ax

	ret

global get_duration
get_duration:

	push dx

	push ax

	mov     dx,40h
	in      al,dx
	xchg    al,ah
	nop
	in      al,dx
	xchg    al,ah

	mov dx, ax

	pop ax

	; AX = prev tick
	cmp dx,ax
	ja rollback

	sub ax,dx

	pop dx

	ret

rollback:

	push cx
	; |      o                  n     |
	mov cx, 0xFFFF
	sub cx, dx
	add cx, ax
	mov ax, cx

	pop cx

	pop dx

	ret

global get_curtick
get_curtick:

	push dx

	mov     dx,40h
	in      al,dx
	xchg    al,ah
	nop
	in      al,dx
	xchg    al,ah
	nop
	pop dx

	ret

;-------------------------------------------------------------------
global wait_delay
wait_delay:
	push ax
	push dx

	mov     dx,40h
	in      al,dx
	xchg    al,ah
	nop
	in      al,dx
	xchg    al,ah
	nop
	xchg    cx,ax
	add     cx,18                              ; Current timer + the delay we want
											   ; in ticks/second. 1 sec := 18.2 ticks
.pollTimer:
	in      al,dx
	xchg    al,ah
	nop
	in      al,dx
	xchg    al,ah
	nop
	cmp     ax,cx
	jb      .pollTimer                          ; until n tick pass

	pop dx
	pop ax

	ret

	mov     al, 0x14
	out     43h,al       ; freeze timer 0 - latch timer 0
	nop

.waittimerloop_1:
	in      al, 0x40     ; read latched timer 0 hi-byte
	cmp     al , 1
	jne     .waittimerloop_1

.waittimerloop_2:
	in      al, 0x40     ; read latched timer 0 hi-byte
	cmp     al , 1
	je     .waittimerloop_2

	pop ax
	ret

;-------------------------------------------------------------------
global delay
delay:
	push ax

.delay_loop:
	call wait_delay
	dec ax
	jnz .delay_loop

	pop ax
	ret
