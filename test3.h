int init(long unsigned int* data, unsigned int len);

void lib_init(void) __attribute__((constructor));
void lib_fini(void) __attribute__((destructor));
