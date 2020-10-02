
target ::= varpp

sources := utils.c log.c loc.c defs.c varpp.c
objects := $(sources:.c=.o)

CC := clang

CFLAGS := -g -W -Wall

all: $(target)

varpp: $(objects)

clean:
	$(RM) $(objects) $(target)
