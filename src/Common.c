#include "Common.h"




void allocation_error(char *file, char *data_structure){
	printf("Allocation error for data structure %s, in file %s", data_structure, file);
	exit(EXIT_FAILURE);
}