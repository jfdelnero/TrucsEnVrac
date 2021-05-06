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
;// File : clearblock.asm
;// Contains: fast clear block function
;//
;// Written by: Jean-François DEL NERO
;//
;// Change History (most recent first):
;///////////////////////////////////////////////////////////////////////////////

#include "P18CXXX.INC"
#include "C18_MACRO.INC"

STRING CODE

;------------------------------------------------------
clearblock
  global clearblock

	Stk1CpyToReg -1,FSR0H ; put bank to FSR0H

	CLRF FSR0L,ACCESS
NEXT:
	CLRF   POSTINC0,ACCESS ; Clear INDF
	CLRF   POSTINC0,ACCESS ; Clear INDF
	CLRF   POSTINC0,ACCESS ; Clear INDF
	CLRF   POSTINC0,ACCESS ; Clear INDF
	CLRF   POSTINC0,ACCESS ; Clear INDF
	CLRF   POSTINC0,ACCESS ; Clear INDF
	CLRF   POSTINC0,ACCESS ; Clear INDF
	CLRF   POSTINC0,ACCESS ; Clear INDF
	TSTFSZ FSR0L,ACCESS
	BRA    NEXT
	
	return

  end
