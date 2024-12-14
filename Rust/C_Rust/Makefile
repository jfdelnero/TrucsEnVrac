
override CFLAGS += -Wall -O3
RUSTC = rustc

sources_c := $(wildcard src_c/*.c)
objects_c := $(sources_c:src_c/%.c=obj/%.o)

sources_rust := $(wildcard src_rust/*.rs)
objects_rust := $(sources_rust:src_rust/%.rs=obj/%.o)

all: output_dir c_rust

c_rust: $(objects_c) $(objects_rust) 
	${CC} -o $@    $^ $(LDFLAGS)
	strip -s $@

$(objects_c): obj/%.o: src_c/%.c
	${CC} -o $@ $^ -c $(CFLAGS)

$(objects_rust): obj/%.o: src_rust/%.rs
	${RUSTC} -o $@ $^ --crate-type staticlib

output_dir:
	@mkdir -p obj obj

clean:
	rm -Rf  *.o  .*.o  .*.o.* c_rust obj
