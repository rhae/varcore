

all: tools lib

.PHONY: lib
lib:
	$(MAKE) -C lib

.PHONY: test
test: lib
	$(MAKE) -C test

.PHONY: tools
tools: 
	$(MAKE) -C tools