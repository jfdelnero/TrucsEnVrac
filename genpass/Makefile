
override CFLAGS += -Wall -O3
override LDFLAGS += -lm

sources := $(wildcard src/*.c)
objects := $(sources:src/%.c=obj/%.o)

all: output_dir genpass

genpass:  $(objects) 
	${CC} -o $@    $^ $(LDFLAGS)

$(objects): obj/%.o: src/%.c
	${CC} -o $@ $^ -c $(CFLAGS)

output_dir:
	@mkdir -p obj

clean:
	rm -Rf  *.o  .*.o  .*.o.* genpass obj
