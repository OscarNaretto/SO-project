#include "Common.h"

void source_signal_actions();
void source_handle_signal(int signum);
void source_set_maps();
void source_call_taxi();

int **source_map;
sigset_t my_mask;

int main(int argc, char *argv[]){
    /*Controllo argomentti passati con possibile exec
        Vedere come sviluppato main
        if(....)...
    */

    source_signal_actions;


    //srand(...);

    source_set_maps;


    raise(SIGALRM);
}

void source_signal_actions(){
    struct sigaction abort_taxi;
    struct sigaction restart_source;
    
    abort_taxi.sa_handler = source_handle_signal;
    abort_taxi.sa_flags = 0;
    
    restart_source.sa_handler = source_handle_signal; 
    restart_source.sa_flags = SA_RESTART;

    sigaddset(&my_mask, SIGALRM);
    sigaddset(&my_mask, SIGINT);

    abort_taxi.sa_mask = my_mask;
    restart_source.sa_mask = my_mask;

    sigaction(SIGINT, &abort_taxi, NULL);
    sigaction(SIGALRM, &restart_source, NULL);

}

void source_handle_signal(int signum){
    sigprocmask(SIG_BLOCK, &my_mask, NULL);
    switch (signum){
        case SIGALRM:
            //Gestire caso alarm
            source_call_taxi();
            break;
        case SIGINT:
            //Gestire caso chiusura simulazione
            //exit();
            break;
        default:
            printf("\nSegnale %d non gestito\n", signum);
            break;
    }
    sigprocmask(SIG_UNBLOCK, &my_mask, NULL);
}

void source_set_maps(){
    int i, j, offset = 0;
    source_map = (int **)malloc(SO_HEIGHT * sizeof(int *));
    
    if (source_map == NULL){allocation_error("Source", "source_map");}
    for (i = 0; i < SO_HEIGHT; i++){
        source_map[i] = malloc(SO_WIDTH * sizeof(int));
        if (source_map[i] == NULL){
            allocation_error("Source", "source_map");
        }else{
            for (i = 0; i < SO_HEIGHT; i++){
                for(j = 0; j < SO_WIDTH; j++){
                    source_map[i][j] = //Finire assegnamento con shdmem
                    offset++;
                }
            }
        }
    }
}

void source_call_taxi(){

}