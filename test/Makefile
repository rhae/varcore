
target ::= tests

sources := testrunner.c varcore_test.c
objects := $(sources:.c=.o)

CC      := clang
AR      := ar

INCLUDE := -Icunit/CUnit -I../lib
LIBS    := -L cunit/local-build/CUnit -lcunit -L../lib -lvarcore

CFLAGS  := -g -W -Wall -pedantic $(INCLUDE)

all: $(target)

cunit:
	# git clone https://gitlab.com/cunity/cunit.git
	if [[ -d cunit ]]; then \
		cd cunit && git checkout; \
	else \
		git clone https://gitlab.com/rhae/cunit.git \
	fi

libcunit: cunit
	-cd cunit && mkdir -p local-build && cd local-build && cmake .. && cmake --build . 

vardef.inc: res.csv
	../tools/varpp/varpp $<

tests: libcunit vardef.inc $(objects)
	$(CC) $(CFLAGS) $(objects) $(LIBS) -o $@
	./$@

clean:
	# $(RM) -rf cunit
	$(RM) vardef.inc vardefs.h
	$(RM) $(objects) $(target)
