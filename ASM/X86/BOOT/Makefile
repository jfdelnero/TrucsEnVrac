
sources := $(wildcard *.ASM)
sources_utils := $(wildcard utils/*.ASM)
objects := $(sources:%.ASM=obj/%.BIN)
objects_utils := $(sources_utils:utils/%.ASM=obj/%.o)

NASM := nasm
FILEIMAGE := obj/disk_image.img

all: output_dir disk_image

disk_image:  $(objects)
	cat obj/BOOTSECT.BIN obj/STAGE2.BIN > ${FILEIMAGE}
	dd if=/dev/zero of=${FILEIMAGE} bs=1 count=0 seek=737280

$(objects): obj/%.BIN: %.ASM
	${NASM} $^ -o $@  -I ./utils

$(objects_utils): obj/%.o: utils/%.ASM
	${NASM} $^ -o $@  -f elf

output_dir:
	@mkdir -p obj

clean:
	rm -Rf ${FILEIMAGE} obj
