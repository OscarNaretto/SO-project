#include "Common.h"

void source_signal_actions();
void source_handle_signal(int signum);
void source_set_maps();


sigset_t mask;

int main(int argc, char *argv[]){
    /*Controllo argomentti passati con possibile exec
        Vedere come sviluppato main
        if(....)...
    */

    source_signal_actions;


    //srand(...);

    source_set_maps;
}

void source_signal_actions(){
    struct sigaction abort;

    
    
}

void source_handle_signal(int signum){
    switch (signum){
        case SIGALRM:
            //Gestire caso alarm
            break;
        case SIGQUIT:
            //Gestire caso chiusura simulazione
            break;
        case SIGINT:
            raise(SIGQUIT);
            break;
        default:
            printf("\nSegnale %d non gestito\n", signum);
            break;
    }
}

void source_set_maps(){
    int i, j, offset = 0;
    map = (int **)malloc(SO_HEIGHT * sizeof(int *));
    if (map == NULL){
        allocation_error("Source", "map");
    }
    for (i = 0; i < SO_HEIGHT; i++){
        map[i] = malloc(SO_WIDTH * sizeof(int));
        if (map[i] == NULL){
            allocation_error("Source", "map");
        }
    }

    for (i = 0; i < SO_HEIGHT; i++){
        for(j = 0; j < SO_WIDTH; j++){
            map[i][j] = //shd_mem_map[offset].cell_map_value; //controllare cell_map_value 
            offset++;
        }
    }
    //shmdt(shd_mem_map);
}