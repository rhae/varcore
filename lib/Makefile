
target ::= libvarcore.a

sources := varcore.c
objects := $(sources:.c=.o)

CC      ?= clang
AR      := ar

# CFLAGS  := -g -W -Wall -pedantic -fsanitize=undefined
CFLAGS  := -g -W -Wall -pedantic

all: $(target)

libvarcore.a: $(objects)
	$(AR) cr $@ $<

.PHONY: check
check:
	cppcheck -I. --suppressions-list=cppcheck_suppress.txt --enable=all $(sources)

clean:
	$(RM) $(objects) $(target)

