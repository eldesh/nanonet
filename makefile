
LIB = libnanonet.a

.PHONY: $(LIB)
$(LIB):
	$(MAKE) -C src

.PHONY: test
test:
	$(MAKE) -C test

.PHONY: clean
clean:
	$(MAKE) clean -C src
	$(MAKE) clean -C test


