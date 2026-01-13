#include <stdio.h>
#include "test.h"


int hello(void) {
	return printf("Hello World\n");
}


int sum(int a, int b) {
	int s = a + b;
	printf("%d + %d = %d\n", a,b,s);
	return s;
}

