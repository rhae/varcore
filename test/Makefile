
target ::= tests

sources := testrunner.c varcore_test.c
objects := $(sources:.c=.o)

CC := clang
AR := ar

INCLUDE := -Icunit/CUnit -I../lib
LIBS := -L cunit/local-build/CUnit -lcunit -L../lib -lvarcore

CFLAGS := -g -W -Wall -pedantic $(INCLUDE)

all: $(target)

.PHONY: cunit
cunit:
	# git clone https://gitlab.com/cunity/cunit.git
	cd cunit && mkdir local-build && cd local-build && cmake .. && cmake --build . 

vardef.inc: res.csv
	../tools/varpp/varpp $<

tests: vardef.inc $(objects)
	$(CC) $(CFLAGS) $(objects) $(LIBS) -o $@

clean:
	# $(RM) -rf cunit
	$(RM) vardef.inc vardefs.h
	$(RM) $(objects) $(target)
