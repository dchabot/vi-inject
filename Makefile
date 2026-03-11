GIT_HASH := $(shell git rev-parse --short HEAD)

iqdtest: iqd_test.c iqd_test.h version.h
	gcc -Wall -shared -fPIC -ggdb -o lib$@.so $<

testbits: iqdtest test_bits.c
	gcc -Wall -ggdb -L./ -l$< -o $@ test_bits.c

version.h:
	@echo "#ifndef __VERSION_H__" > $@
	@echo "#define __VERSION_H__" >> $@
	@echo "#define GIT_HASH 0x$(GIT_HASH)u" >> $@
	@echo "#endif" >> $@

version: version.h

all: iqdtest testbits

clean: 
	rm -vf *.so testbits version.h
