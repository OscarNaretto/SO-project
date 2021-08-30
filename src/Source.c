#include "Common.h"

//parameters
int SO_INIT_REQUESTS = 0;

//coordinates
int x = 0, y = 0;
int x_to_go = 0, y_to_go = 0;

//message queue
int msgqueue_id = 0;

//semaphores
int sem_sync_id = 0;

//shared memory
source_value_struct *source_shd_mem = 0;
int source_shd_mem_id = 0;

void source_signal_actions();
void source_handle_signal(int signum);
void source_send_request();
int check_message_for_exit();
void source_cleanup();

int main(int argc, char *argv[]){
    if(argc != 7){
        printf("ERRORE NUMERO DEI PARAMETRI PASSATI A SOURCE \n");
        exit(EXIT_FAILURE);
    }
    x = atoi(argv[1]);
    y = atoi(argv[2]);

    msgqueue_id = atoi(argv[3]);
    sem_sync_id = atoi(argv[4]);
    source_shd_mem_id = atoi(argv[5]);

    SO_INIT_REQUESTS = atoi(argv[6]);
    source_shd_mem = (source_value_struct *)shmat(source_shd_mem_id, NULL, 0);
    TEST_ERROR;

    source_signal_actions();
    processes_sync(sem_sync_id);
    
    for (int i = 0; i < SO_INIT_REQUESTS; i++){
        source_send_request();
    }
    
    raise(SIGALRM);

    while (1) {
        
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
            /*if (check_message_for_exit()){
                alarm(2);
            } else {
                alarm(0);
                atexit(source_cleanup);
                exit(EXIT_SUCCESS);
            }*/
            source_send_request();
            alarm(rand() % 5 + 1);
            break;
        case SIGINT:
            alarm(0);
            atexit(source_cleanup);
            exit(EXIT_SUCCESS);
            break;
        default:
            printf("\nSegnale %d non gestito\n", signum);
            break;
    }
}

void source_send_request(){
    int x_to_go, y_to_go, acceptable = 0;
    srand(getpid());
    while (!acceptable){
        x_to_go = rand() % SO_HEIGHT;
        y_to_go = rand() % SO_WIDTH;
        if((source_shd_mem + x * SO_WIDTH + y)->cell_value != 0){
            acceptable = 1;
        }
    }
    my_msgbuf.mtype = (x * SO_WIDTH) + y + 1;
    sprintf(my_msgbuf.mtext, "%d", (x_to_go * SO_WIDTH) + y_to_go);
    msgsnd(msgqueue_id, &my_msgbuf, MSG_LEN, 0);
    TEST_ERROR;
}

int check_message_for_exit(){
    int num_bytes;

    num_bytes = msgrcv(msgqueue_id, &my_msgbuf, MSG_LEN, ((x * SO_WIDTH) + y) + 1, IPC_NOWAIT);
    if (num_bytes > 0){
        //reading msg
        x_to_go = atoi(my_msgbuf.mtext) / SO_WIDTH;
        y_to_go = atoi(my_msgbuf.mtext) % SO_WIDTH;
        //resending msg
        my_msgbuf.mtype = (x * SO_WIDTH) + y + 1;
        sprintf(my_msgbuf.mtext, "%d", (x_to_go * SO_WIDTH) + y_to_go);
        msgsnd(msgqueue_id, &my_msgbuf, MSG_LEN, 0);
        TEST_ERROR;
        return 1;
    } else if (num_bytes <= 0 && errno!=ENOMSG){
        printf("Errore durante la lettura del messaggio: %d", errno);
    }
    return 0;
}

void source_cleanup(){
    struct sembuf sops;

    sops.sem_num = 0; 
    sops.sem_op = 1;
    semop(sem_sync_id, &sops, 1);
    //TEST_ERROR;

    if (shmdt(source_shd_mem) == -1) {
        fprintf(stderr, "%s: %d. Errore in shmdt #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
}