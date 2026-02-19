iqdtest: iqd_test.c iqd_test.h
	gcc -Wall -shared -fPIC -ggdb -o lib$@.so $<

testbits: iqdtest test_bits.c
	gcc -Wall -L./ -l$< -o $@ test_bits.c

all: iqdtest testbits

clean: 
	rm *.so testbits
