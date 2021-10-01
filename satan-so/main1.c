#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

void good(void);

int main(int argc, char *argv[])
{
	void * a = dlopen(NULL, RTLD_NOW);
	void *handle = dlopen("./bin/libbad.so", RTLD_LAZY);
	if (!handle) {
		fprintf(stderr, "%s\n", dlerror());
		return -1;
	}
	printf("good(): %x\n", (int)good);	
	good();
	
	dlclose(handle);
	return 0;
}
