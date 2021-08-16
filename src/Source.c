#include "Common.h"

void source_signal_actions();
void source_handle_signal(int signum);
void source_set_maps();
void source_call_taxi();

int **source_map;
sigset_t mask;
source_value_struct *source_shd_mem;
struct msgbuf buf_msg_snd;
int x, y;
int source_msgqueue_id;
int source_sem_sync_id;
int source_shd_mem_to_source_id;
int SO_INIT_REQUESTS = -1;
int SO_INIT_REQUESTS_MIN = -1;
int SO_INIT_REQUESTS_MAX  = -1;

int main(int argc, char *argv[]){
    if(argc != 9){
        printf("ERRORE NUMERO DEI PARAMETRI PASSATI A SOURCE \n");
        exit(EXIT_FAILURE);
    }

    x = atoi(argv[1]);
    y = atoi(argv[2]);
    source_msgqueue_id = atoi(argv[3]);
    source_sem_sync_id = atoi(argv[4]);
    source_shd_mem_to_source_id = atoi(argv[5]);
    SO_INIT_REQUESTS_MIN = atoi(argv[6]);
    SO_INIT_REQUESTS_MAX = atoi(argv[7]);

    source_shd_mem = (source_value_struct *)shmat(source_shd_mem_to_source_id, NULL, 0);
    TEST_ERROR;

    srand(getpid());

    source_signal_actions;

    source_set_maps;

    processes_sync(source_sem_sync_id);
    
    SO_INIT_REQUESTS = rand() % (SO_INIT_REQUESTS_MAX + 1 - SO_INIT_REQUESTS_MIN) + SO_INIT_REQUESTS_MIN;
    while(SO_INIT_REQUESTS > 0){
        raise(SIGALRM);
        SO_INIT_REQUESTS--;
        //pause();
    }
    raise(SIGINT);
}

void source_signal_actions(){
    struct sigaction sa_alarm, sa_int;
    
    sa_alarm.sa_handler = source_handle_signal;
    sa_alarm.sa_flags = 0;
    
    sa_int.sa_handler = source_handle_signal; 
    sa_int.sa_flags = 0;

    sigaddset(&mask, SIGALRM);
    sigaddset(&mask, SIGINT);

    sa_alarm.sa_mask = mask;
    sa_int.sa_mask = mask;

    sigaction(SIGINT, &sa_int, NULL);
    TEST_ERROR;

    sigaction(SIGALRM, &sa_alarm, NULL);
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
            alarm(0);
            exit(0);
            break;
        default:
            printf("\nSegnale %d non gestito\n", signum);
            break;
    }
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

void source_set_maps(){
    int i = 0, j, gap = 0;
    source_map = (int **)malloc(SO_HEIGHT * sizeof(int *));
    
    if (source_map == NULL){allocation_error("Source", "source_map");}
    while (i < SO_HEIGHT){
        source_map[i] = malloc(SO_WIDTH * sizeof(int));
        if (source_map[i] == NULL){
            allocation_error("Source", "source_map");
        }else{
            for (i = 0; i < SO_HEIGHT; i++){
                for(j = 0; j < SO_WIDTH; j++){
                    source_map[i][j] = source_shd_mem[gap].cell_value;
                    gap++;
                }
            }
        }
        i++;
    }
    shmdt(source_shd_mem);
}

void source_call_taxi(){
    int destination_coor_x, destination_coor_y;
    
    destination_coor_x = rand() % SO_HEIGHT;
    destination_coor_y = rand() % SO_WIDTH;
    if(source_map[destination_coor_x][destination_coor_y] == 0 || (x == destination_coor_x && y == destination_coor_y)){

    }

    msgsnd(source_msgqueue_id, &buf_msg_snd, MSG_LEN, IPC_NOWAIT);
}