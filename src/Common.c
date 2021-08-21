#include "Common.h"

void allocation_error(char *file, char *data_structure){
	printf("Allocation error for data structure %s, in file %s", data_structure, file);
	exit(EXIT_FAILURE);
}

void processes_sync(int sem_id){
    struct sembuf sops;
    sops.sem_num = 0; 
    sops.sem_op = -1; 
    sops.sem_flg = 0;
    semop(sem_id, &sops, 1);
    TEST_ERROR;
        
    sops.sem_num = 0;
    sops.sem_op = 0; 
    sops.sem_flg = 0;
    semop(sem_id, &sops, 1);
    TEST_ERROR;
}

void shdmem_return_sem_reserve(int sem_id){
    struct sembuf sops;

    sops.sem_num = 1; 
    sops.sem_op = -1; 
    sops.sem_flg = 0;
    semop(sem_id, &sops, 1);
    TEST_ERROR;
}

void shdmem_return_sem_release(int sem_id){
    struct sembuf sops;

    sops.sem_num = 1;
    sops.sem_op = 1;
    sops.sem_flg = 0;
    semop(sem_id, &sops, 1);
    TEST_ERROR;
}
