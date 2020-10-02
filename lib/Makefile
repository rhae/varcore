
target ::= libvarcore.a

sources := varcore.c
objects := $(sources:.c=.o)

CC := clang
AR := ar

CFLAGS := -g -W -Wall -pedantic

all: $(target)

libvarcore.a: $(objects)
	$(AR) cr $@ $<

clean:
	$(RM) $(objects) $(target)

