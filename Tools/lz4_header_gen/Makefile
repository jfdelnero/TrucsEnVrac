
override CFLAGS += -Wall -O3

sources := $(wildcard src/*.c)
objects := $(sources:src/%.c=obj/%.o)

all: output_dir lz4_header_gen

lz4_header_gen:  $(objects) 
	${CC} -o $@    $^ $(LDFLAGS)

$(objects): obj/%.o: src/%.c
	${CC} -o $@ $^ -c $(CFLAGS)

output_dir:
	@mkdir -p obj

clean:
	rm -Rf  *.o  .*.o  .*.o.* *.bin *.lz4 lz4_header_gen obj
