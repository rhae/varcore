

all: tools lib examples test

.PHONY: lib
lib:
	$(MAKE) -C lib

.PHONY: test
test: lib
	$(MAKE) -C test

.PHONY: tools
tools: 
	$(MAKE) -C tools

.PHONY: examples
examples: 
	$(MAKE) -C examples


.PHONY: check
check:
	$(MAKE) -C lib check
	$(MAKE) -C tools check

clean:
	$(MAKE) -C tools clean
	$(MAKE) -C lib clean
	$(MAKE) -C examples clean
	$(MAKE) -C test clean
