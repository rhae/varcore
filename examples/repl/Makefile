
target ::= repl

sources := $(wildcard *.c)
objects := $(sources:.c=.o)

CC      ?= clang
AR      := ar

INCLUDE := -I../../lib
LIBS    := -L../../lib -lvarcore

#CFLAGS  := -g -W -Wall -pedantic $(INCLUDE) -lgcc_s -lubsan -fsanitize=undefined
CFLAGS  := -g -W -Wall -pedantic $(INCLUDE)

all: $(target)


vardef.inc: res.csv
	../../tools/varpp/varpp $<

vars.o: vardef.inc vars.c

repl: $(objects)
	$(CC) $(CFLAGS) $(objects) $(LIBS) -o $@

clean:
	# $(RM) -rf cunit
	$(RM) vardef.inc vardefs.h
	$(RM) $(objects) $(target)
