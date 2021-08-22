#include "Common.h"

//parameters
int SO_INIT_REQUESTS;

//map
int **source_map;

//coordinates
int x, y;
int x_to_go, y_to_go;

//message queue
int source_msgqueue_id;
struct msgbuf my_msgbuf;

//semaphores
int source_sem_sync_id;

//shared memory
source_value_struct *source_shd_mem;
int source_shd_mem_id;

void source_set_maps();
void source_signal_actions();
void source_handle_signal(int signum);
void source_call_taxi();
int check_message_for_exit();
void source_map_free();

int main(int argc, char *argv[]){
    if(argc != 7){
        printf("ERRORE NUMERO DEI PARAMETRI PASSATI A SOURCE \n");
        exit(EXIT_FAILURE);
    }
    x = atoi(argv[1]);
    y = atoi(argv[2]);
    source_msgqueue_id = atoi(argv[3]);
    source_sem_sync_id = atoi(argv[4]);
    source_shd_mem_id = atoi(argv[5]);
    SO_INIT_REQUESTS = atoi(argv[6]);

    source_shd_mem = (source_value_struct *)shmat(source_shd_mem_id, NULL, 0);
    TEST_ERROR;

    source_set_maps;
    source_signal_actions;
    processes_sync(source_sem_sync_id);
    
    for (int i = 0; i < SO_INIT_REQUESTS; i++){
        source_call_taxi();
    }
    
    raise(SIGALRM);
    while (1) {
        pause();
    }
}

void source_signal_actions(){
    struct sigaction sa_alarm, sa_int;
    sigset_t mask;

    sigemptyset(&mask); 
    sigaddset(&mask, SIGALRM);
    sigaddset(&mask, SIGINT);

    sigprocmask(SIG_BLOCK, &mask, NULL);
    
    sa_alarm.sa_mask = mask;
    sa_int.sa_mask = mask;
    
    sa_alarm.sa_handler = source_handle_signal;
    sa_alarm.sa_flags =  SA_RESTART;
    
    sa_int.sa_handler = source_handle_signal; 
    sa_int.sa_flags = 0;

    sigprocmask(SIG_UNBLOCK, &mask, NULL);
    
    sigaction(SIGINT, &sa_int, NULL);
    sigaction(SIGALRM, &sa_alarm, NULL);
}

void source_handle_signal(int signum){
    switch (signum){
        case SIGALRM:
            if(check_message_for_exit()){
                alarm(5);
            } else {
                raise(SIGINT);
            }
            break;
        case SIGINT:
            source_map_free();
            exit(EXIT_SUCCESS);
            break;
        default:
            printf("\nSegnale %d non gestito\n", signum);
            break;
    }
}

void source_set_maps(){
    int i, j, offset = 0;
    
    source_map = (int **)malloc(SO_HEIGHT * sizeof(int *));
    if (source_map == NULL){ allocation_error("Source", "source_map"); }
    for (i = 0; i < SO_HEIGHT; i++){
        source_map[i] = malloc(SO_WIDTH * sizeof(int));
        if (source_map[i] == NULL){
            allocation_error("Source", "source_map");
        } else {
            for (j = 0; j < SO_WIDTH; j++){
                source_map[i][j] = source_shd_mem[offset].cell_value;
                offset++;
            }
        }
    }
    shmdt(source_shd_mem);
}

void source_call_taxi(){
    int X, y_to_go, acceptable = 0;
    srand(getpid());

    while (acceptable){
        X = rand() % SO_HEIGHT;
        y_to_go = rand() % SO_WIDTH;

        if(source_map[X][y_to_go] != 0 && (x != X || y != y_to_go)){
            acceptable = 1;
        }
    }
    
    my_msgbuf.mtype = (x * SO_WIDTH) + y + 1;
    sprintf(my_msgbuf.mtext, "%d", (X * SO_WIDTH) + y_to_go);
    msgsnd(source_msgqueue_id, &my_msgbuf, MSG_MAX_SIZE, 0);
    TEST_ERROR;
}

int check_message_for_exit(){
    int num_bytes;

    num_bytes = msgrcv(source_msgqueue_id, &my_msgbuf, MSG_MAX_SIZE, ((x * SO_WIDTH) + y) + 1, IPC_NOWAIT);
    if (num_bytes > 0){
        //reading msg
        x_to_go = atoi(my_msgbuf.mtext) / SO_WIDTH;
        y_to_go = atoi(my_msgbuf.mtext) % SO_WIDTH;
        
        //resending msg
        my_msgbuf.mtype = (x * SO_WIDTH) + y + 1;
        sprintf(my_msgbuf.mtext, "%d", (x_to_go * SO_WIDTH) + y_to_go);
        msgsnd(source_msgqueue_id, &my_msgbuf, MSG_MAX_SIZE, IPC_NOWAIT);
        TEST_ERROR;

        return 1;
    } else if (num_bytes <= 0 && errno!=ENOMSG){
        printf("Errore durante la lettura del messaggio: %d", errno);
    }
    return 0;
}

void source_map_free(){
    for (int i = 0; i < SO_HEIGHT; i++){
        free(source_map[i]);
    }
    free(source_map);
}