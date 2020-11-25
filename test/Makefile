
target ::= tests

sources := $(wildcard *.c)
objects := $(sources:.c=.o)

CC      := clang
AR      := ar

INCLUDE := -Icunit/CUnit -I../lib
LIBS    := -L cunit/local-build/CUnit -lcunit -L../lib -lvarcore

#CFLAGS  := -g -W -Wall -pedantic $(INCLUDE) -lgcc_s -lubsan -fsanitize=undefined
CFLAGS  := -g -W -Wall -pedantic $(INCLUDE)

all: $(target)

cunit:
	git clone https://gitlab.com/cunity/cunit.git

libcunit: cunit
	-cd cunit && mkdir -p local-build && cd local-build && cmake .. && cmake --build . 

vardef.inc: res.csv
	../tools/varpp/varpp $<

varcore_test.o: vardef.inc varcore_test.c

tests: libcunit vardef.inc $(objects)
	$(CC) $(CFLAGS) $(objects) $(LIBS) -o $@
	./$@

clean:
	# $(RM) -rf cunit
	$(RM) vardef.inc vardefs.h
	$(RM) $(objects) $(target)
