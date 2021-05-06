;///////////////////////////////////////////////////////////////////////////////
;//---------------------------------------------------------------------------//
;//----------H----H--X----X-----CCCCC-----22222----0000-----0000-----11-------//
;//---------H----H----X-X-----C--------------2---0----0---0----0---1-1--------//
;//--------HHHHHH-----X------C----------22222---0----0---0----0-----1---------//
;//-------H----H----X--X----C----------2-------0----0---0----0-----1----------//
;//------H----H---X-----X---CCCCC-----22222----0000-----0000----11111---------//
;//---------------------------------------------------------------------------//
;//----- Contact: hxc2001 at hxc2001.com ----------- https://hxc2001.com -----//
;//----- (c) 2021 Jean-François DEL NERO ----------- http://hxc2001.free.fr --//
;///////////////////////////////////////////////////////////////////////////////
;// File : crc16.asm
;// Contains: crc16 functions
;//
;// Written by: Jean-François DEL NERO
;//
;// Change History (most recent first):
;///////////////////////////////////////////////////////////////////////////////

#include "P18CXXX.INC"
#include "C18_MACRO.INC"
#include "ASM_VAR.INC"

STRING CODE

;///////////////////////////////////////////////////////////////////////////////////
;// unsigned char CRC16();
;///////////////////////////////////////////////////////////////////////////////////

CRC16
	global CRC16

	MOVLB   VARPAGE
;// Little Endian CRC16
	movlw   8
	movwf   CRCCount,1
	movf    CRCData,0,1
	xorwf   CRCLo,1,1
_loop:
	bcf     STATUS,0,0
	rrcf    CRCHi,1,1
	rrcf    CRCLo,1,1
	bnc     _next
	movf    CRCPolyLo,0,1
	xorwf   CRCLo,1,1
	movf    CRCPolyHi,0,1
	xorwf   CRCHi,1,1
_next:
	decfsz  CRCCount,1,1
	bra     _loop
	return

CRC16BE
	global CRC16BE
;unsigned char CRCPolyLo = 0x21, CRCPolyHi = 0x10, CRCLo, CRCHi, CRCData, CRCCount;
; - - - -
; Big Endian CRC16
; - - - -
	MOVLB  VARPAGE

	movlw   8
	movwf   CRCCount,1
	movf    CRCData,0,1
	xorwf   CRCHi,1,1
_loopBE:
	bcf     STATUS,0,0
	rlcf    CRCLo,1,1
	rlcf    CRCHi,1,1
	bnc     _nextBE
	movf    CRCPolyLo,0,1
	xorwf   CRCLo,1,1
	movf    CRCPolyHi,0,1
	xorwf   CRCHi,1,1
_nextBE:
	decfsz  CRCCount,1,1
	bra     _loopBE
	return

 end
