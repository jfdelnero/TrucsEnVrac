; -----------------------------------------------------------------------------
; -----------------------------------------------------------------------------
; Timer functions
; (Use nasm to assemble it)
; Sept-2024 Jean-François DEL NERO / HxC2001
; -----------------------------------------------------------------------------
; -----------------------------------------------------------------------------

[bits  32]

section .text

;------------------------------------------------------------------------------
; timer init
global init_timer
init_timer:
	push ax
	push cx

	countdown  equ	8000h ; approx 36 interrupts per second

	mov	al,00110110b    ; bit 7,6 = (00) timer counter 0
						; bit 5,4 = (11) write LSB then MSB
						; bit 3-1 = (011) generate square wave
						; bit 0 = (0) binary counter
	out	43h,al	        ; prep PIT, counter 0, square wave&init count
	nop
	mov	cx,countdown    ; default is 0x0000 (65536) (18.2 per sec)

	; interrupts when counter decrements to 0
	mov	al,cl	        ; send LSB of timer count
	out	40h,al
	nop

	mov	al,ch	        ; send MSB of timer count
	out	0x40,al
	nop

	pop cx
	pop ax

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
	out	    43h,al       ; freeze timer 0 - latch timer 0
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