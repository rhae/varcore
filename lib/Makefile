
target ::= vc.lib

sources := varcore.c
objects := $(sources:.c=.o)

CC := clang
AR := ar

CFLAGS := -g -W -Wall -pedantic

all: $(target)

libvarcore.a: $(objects)
	$(AR) cr $@ $<

.PHONY: vc.test
vc.test:
	$(CC) $(CFLAGS) -DTEST $(sources) -o $@

clean:
	$(RM) $(objects) $(target)