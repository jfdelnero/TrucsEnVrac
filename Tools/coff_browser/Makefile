
override CFLAGS += -Wall

sources := $(wildcard src/*.c)
objects := $(sources:src/%.c=obj/%.o)

all: output_dir coff_browser

coff_browser:  $(objects)
	${CC} -o $@    $^ $(LDFLAGS)

$(objects): obj/%.o: src/%.c
	${CC} -o $@ $^ -c $(CFLAGS)

output_dir:
	@mkdir -p obj

clean:
	rm -Rf  *.o  .*.o  .*.o.* coff_browser obj
