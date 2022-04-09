
sources := $(wildcard *.ASM)
objects := $(sources:%.ASM=obj/%.BIN)

NASM := nasm
FILEIMAGE := obj/disk_image.img

all: output_dir disk_image

disk_image:  $(objects)
	cat obj/BOOTSECT.BIN obj/STAGE2.BIN > ${FILEIMAGE}
	dd if=/dev/zero of=${FILEIMAGE} bs=1 count=0 seek=737280

$(objects): obj/%.BIN: %.ASM
	${NASM} $^ -o $@  

output_dir:
	@mkdir -p obj

clean:
	rm -Rf ${FILEIMAGE} obj
