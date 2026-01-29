int init(char* bitfile, char* resource);

void lib_init(void) __attribute__((constructor));
void lib_fini(void) __attribute__((destructor));
