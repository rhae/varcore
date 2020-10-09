
target ::= varpp

sources := utils.c log.c loc.c defs.c strpool.c varpp.c
objects := $(sources:.c=.o)

CC := clang

CFLAGS := -g -W -Wall

all: $(target)

varpp: $(objects)

.PHONY: check
check:
	cppcheck $(sources)

clean:
	$(RM) $(objects) $(target)
