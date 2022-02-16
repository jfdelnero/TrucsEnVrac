-------------------------------------------------------------------------------
-----------H----H---X----X----CCCCC----22222-----0000-----0000-----11----------
----------H----H-----X-X----C--------------2---0----0---0----0---1-1-----------
---------HHHHHH------X-----C----------22222---0----0---0----0-----1------------
--------H----H-----X--X---C----------2-------0----0---0----0-----1-------------
-------H----H----X-----X--CCCCC-----222222---0000-----0000----11111------------
-------------------------------------------------------------------------------

ROM Splitter

a ROM file splitter

Written by: Jean-François DEL NERO

Syntax :

rom_split ROMTOSPLIT.ROM [-num_of_bytes:x] [-num_of_banks:x] [-bank_word_size:x (in KB)] [-mirror:x]

Example : Split a ROM file for a 16 bits system with 2*32KB*3 EPROMs (192KB):

rom_split ROMTOSPLIT.ROM -num_of_bytes:2 -num_of_banks:3 -bank_word_size:32
