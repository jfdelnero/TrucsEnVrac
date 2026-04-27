
override CFLAGS += -Wall -O1
CC = $(TGT)gcc
AS = $(TGT)gcc -x assembler-with-cpp

sources := $(wildcard src/*.c)
objects := $(sources:src/%.c=obj/%.o)

sources_asm := $(wildcard src/*.s)
objects_asm := $(sources_asm:src/%.s=obj/%.so)

all: output_dir unzx0

unzx0:  $(objects) $(objects_asm)
	${CC} -o $@    $^ $(LDFLAGS)

$(objects): obj/%.o: src/%.c
	${CC} -o $@ $^ -c $(CFLAGS)

$(objects_asm): obj/%.so: src/%.s
	${AS} -o $@ $^ -c $(CFLAGS)

output_dir:
	@mkdir -p obj

clean:
	rm -Rf  *.o  .*.o  .*.o.* unzx0 obj
