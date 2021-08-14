#include "Common.h"

void source_signal_actions();
void source_handle_signal(int signum);
void source_set_maps();
void source_call_taxi();

int **source_map;
sigset_t mask;

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
    struct sigaction sa_alarm, sa_int;
    
    sa_alarm.sa_handler = source_handle_signal;
    sa_alarm.sa_flags = 0;
    
    sa_int.sa_handler = source_handle_signal; 
    sa_int.sa_flags = SA_RESTART;

    sigaddset(&mask, SIGALRM);
    sigaddset(&mask, SIGINT);

    sa_alarm.sa_mask = mask;
    sa_int.sa_mask = mask;

    sigaction(SIGINT, &sa_alarm, NULL);
    TEST_ERROR;

    sigaction(SIGALRM, &sa_int, NULL);
    TEST_ERROR;

}

void source_handle_signal(int signum){
    sigprocmask(SIG_BLOCK, &mask, NULL);
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
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
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