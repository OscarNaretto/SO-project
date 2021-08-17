#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/sem.h>

#define SO_WIDTH 20
#define SO_HEIGHT 10

#define TEST_ERROR    if (errno) {fprintf(stderr, \
					   "%s:%d: PID=%5d: Error %d (%s)\n",\
					   __FILE__,\
					   __LINE__,\
					   getpid(),\
					   errno,\
					   strerror(errno));exit(-1);}

#define MSG_LEN 128 /* 128 - sizeof(long) */

//Struttura per i messaggi
struct msgbuf{
	long mtype;
	char mtext[MSG_LEN];
}buf_msg_snd;

//cell value passed to source process
typedef struct {
    int cell_value;
} source_value_struct;

//cell value and timensec value passed to taxi process
typedef struct{
	int cell_value; 
	long int cell_timensec_value; 
} taxi_value_struct;

//returned stats printed by Master
typedef struct{
	int trips_completed;
	int longest_trip;
	int slowest_trip;
	int max_trips_completed;
	int top_cells_map[SO_HEIGHT][SO_WIDTH];    
	pid_t pid_longest_trip;
	pid_t pid_slowest_trip;
	pid_t pid_max_trips_completed;
} returned_stats;
returned_stats *shd_mem_returned_stats;

void allocation_error(char *file, char *data_structure);

//semaphores
void processes_sync(int sem_id){
void shdmem_return_sem_reserve(int sem_id){
void shdmem_return_sem_release(int sem_id){

#endif