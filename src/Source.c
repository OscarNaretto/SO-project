#include "Common.h"

void source_signal_actions();
void source_handle_signal(int signum);
void source_set_maps();
void source_call_taxi();
int source_check_message();

int **source_map;
sigset_t mask;
source_value_struct *source_shd_mem;
int x, y;
int X, Y;
int source_msgqueue_id;
int source_sem_sync_id;
int source_shd_mem_to_source_id;
int requests_ended = 0;
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
    for(int i = 0; i < SO_INIT_REQUESTS; i++){
        source_call_taxi();
    }
    
    raise(SIGALRM);

    /*while (1)
    {
        pause();
    }*/
}

void source_signal_actions(){
    struct sigaction sa_alarm, sa_int;

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
            if(source_check_message()){
                
            }
            //Gestire caso alarm
            /*if(SO_INIT_REQUESTS == 0){
                raise(SIGINT);
            }else{
                //Inserimento requests_ended per dirgli quando deve finire
                SO_INIT_REQUESTS--;
            }*/
            break;
        case SIGINT:
            //Gestire caso chiusura simulazione
            alarm(0);
            exit(SO_INIT_REQUESTS);
            break;
        default:
            printf("\nSegnale %d non gestito\n", signum);
            break;
    }
}

void source_set_maps(){
    int i = 0, j, offset = 0;
    source_map = (int **)malloc(SO_HEIGHT * sizeof(int *));
    
    if (source_map == NULL){allocation_error("Source", "source_map");}
    while (i < SO_HEIGHT){
        source_map[i] = malloc(SO_WIDTH * sizeof(int));
        if (source_map[i] == NULL){
            allocation_error("Source", "source_map");
        }else{
            for(j = 0; j < SO_WIDTH; j++){
                source_map[i][j] = source_shd_mem[offset].cell_value;
                offset++;
            }
        }
        i++;
    }
    shmdt(source_shd_mem);
}

void source_call_taxi(){
    int X, Y, acceptable = 0;
    int type_msg;
    while (acceptable){
        X = rand() % SO_HEIGHT;
        Y = rand() % SO_WIDTH;

        if(source_map[X][Y] != 0 && (x != X || y != Y)){
            acceptable = 1;
        }
    }
    
    //copia msg nel buffer
    if((type_msg = (x * SO_WIDTH) + y + 1) > 0){
        buf_msg_snd.mtype = type_msg;
    }else{
        fprintf(stderr,"ERRORE VALORE PARAMETRO MTYPE, DEVE ESSERE POSITIVO\n");
        return;
    }
    sprintf(buf_msg_snd.mtext, "%d", (X * SO_WIDTH) + Y);
    msgsnd(source_msgqueue_id, &buf_msg_snd, MSG_LEN, IPC_NOWAIT);
    TEST_ERROR;
}

int source_check_message(){
    int num_bytes;

    //DA RIVEDERE E COMPLETARE
    num_bytes = msgrcv(source_msgqueue_id, &buf_msg_snd, MSG_LEN, ((x * SO_WIDTH) + y) + 1, IPC_NOWAIT);
    if (num_bytes > 0){
        X = atoi(buf_msg_snd.mtext) / SO_WIDTH;
        Y = atoi(buf_msg_snd.mtext) % SO_WIDTH;
        return 1;
    } else if (num_bytes <= 0 && errno!=ENOMSG){
        printf("Errore durante la lettura del messaggio: %d", errno);
    }
    return 0;
}
