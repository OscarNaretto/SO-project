#include "Common.h"
#define EXIT_FAILURE_CUSTOM -1

int **taxi_map;
long int **TAXI_TIMENSEC_MAP;

int main(int argc, char const *argv[])
{
    /* code */
}

void taxi_signal_actions(){
    /* code */
}

void signal_handler(int signal){
    /* code */
}

void set_maps(){
    int i, j;
    int offset = 0;

    taxi_map = (int **)malloc(SO_HEIGHT*sizeof(int*));
    if(taxi_map = NULL){
        allocation_error("Taxi", "taxi_map");
    }
    for(i = 0; i < SO_HEIGHT; i++){
        taxi_map[i] = malloc(SO_WIDTH * sizeof(int));
        if(taxi_map[i] == NULL){
           allocation_error("Taxi", "taxi_map"); 
        }
    }

    TAXI_TIMENSEC_MAP = (long int **)malloc(SO_HEIGHT*sizeof(long int *));
    if(TAXI_TIMENSEC_MAP = NULL){
        allocation_error("Taxi", "TAXI_TIMENSEC_MAP");
    }
    for ( i = 0; i < SO_HEIGHT; i++){
       TAXI_TIMENSEC_MAP[i] = malloc(SO_WIDTH*sizeof(long int));
       if(TAXI_TIMENSEC_MAP == NULL){
           allocation_error("Taxi","TAXI_TIMENSEC_MAP");
       }
    }

    for ( i = 0; i < SO_HEIGHT i++){
       for ( j = 0; i < SO_WIDTH; j++){
           taxi_map[i][j] = (/*shd_mem_taxi*/ + offset)-> cell_map_value; //guardare i nomi
           TAXI_TIMENSEC_MAP[i][j] = (/*shd_mem_taxi*/ + offset)->cell_timensec_map_value;// guardare i nomi
           offset++;
       }    
    }
    
    shmdt(/*shd_mem_taxi*/);
}