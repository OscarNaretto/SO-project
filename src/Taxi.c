#include "Common.h"

//parameters
struct timespec timeout;

//coordinates
int x = 0, y = 0;
int x_to_go = 0, y_to_go = 0;

//message queue
int msgqueue_id = 0;

//semaphores
int sem_sync_id = 0;
int sem_cells_cap_id = 0;
struct sembuf sops[2];

//shared memory
taxi_value_struct *taxi_shd_mem = 0;
returned_stats *shd_mem_returned_stats = 0;

//stats
int taxi_completed_trips = 0;

//movement timer
struct timespec timer;

//flag SIGINT
int flag_sigint = 0;


void taxi_signal_actions();
void taxi_signal_handler(int signum);
void taxi_cleanup();
void customer_research();
void request_check();
int in_bounds(int x_check, int y_check);
int choose_direction();
int taxi_ride();

int main(int argc, char *argv[]){
    if(argc != 9){
        fprintf(stderr, "\n%s: %d. NUMERO DI PARAMETRI ERRATO\n", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
    }

    x = atoi(argv[1]); 
    y = atoi(argv[2]);
    
    timeout.tv_sec = atoi(argv[3]);

    msgqueue_id = atoi(argv[4]);
    sem_sync_id = atoi(argv[5]);
    sem_cells_cap_id = atoi(argv[6]);

    taxi_shd_mem = shmat(atoi(argv[7]), NULL, 0);
    TEST_ERROR;
    shd_mem_returned_stats = shmat(atoi(argv[8]), NULL, 0);
    TEST_ERROR;

    taxi_signal_actions();
    processes_sync(sem_sync_id);
    
    while (1) {
        customer_research();
    }
}

void taxi_signal_actions(){
    struct sigaction sa_int;
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask,SIGINT);
    sigaddset(&mask,SIGQUIT);

    sigprocmask(SIG_BLOCK, &mask, NULL);

    sa_int.sa_mask = mask;
    sa_int.sa_handler = taxi_signal_handler;
    sa_int.sa_flags = 0;

    sigprocmask(SIG_UNBLOCK, &mask, NULL);

    sigaction(SIGINT, &sa_int, NULL);
    sigaction(SIGQUIT, &sa_int, NULL);
}

void taxi_signal_handler(int signum){
    switch (signum){
        case SIGINT:
            
            if(!flag_sigint){
                if (shmdt(taxi_shd_mem) == -1) {
                    fprintf(stderr, "%s: %d. Errore in shmdt #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
                    exit(EXIT_FAILURE);
                }
                if (shmdt(shd_mem_returned_stats) == -1) {
                    fprintf(stderr, "%s: %d. Errore in shmdt #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
                    exit(EXIT_FAILURE);
                }
                exit(FINISH_SIGINT);
            }   
            taxi_cleanup();
            exit(TAXI_ABORTED);
            break;
        case SIGQUIT:
            if (shmdt(taxi_shd_mem) == -1) {
                fprintf(stderr, "%s: %d. Errore in shmdt #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
                exit(EXIT_FAILURE);
            }
            if (shmdt(shd_mem_returned_stats) == -1) {
                fprintf(stderr, "%s: %d. Errore in shmdt #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
                exit(EXIT_FAILURE);
            }
            exit(TAXI_ABORTED);
        default:
            printf("\nSegnale %d non gestito\n", signum);
            break;
    }
}

void taxi_cleanup(){
    sops[0].sem_num = (x * SO_WIDTH) + y; 
    sops[0].sem_op = 1;
    semop(sem_cells_cap_id, sops, 1);
    TEST_ERROR;

    sops[0].sem_num = 0; 
    sops[0].sem_op = 1;
    semop(sem_sync_id, sops, 1);
    TEST_ERROR;
    
    if (shmdt(taxi_shd_mem) == -1) {
        fprintf(stderr, "%s: %d. Errore in shmdt #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (shmdt(shd_mem_returned_stats) == -1) {
        fprintf(stderr, "%s: %d. Errore in shmdt #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void customer_research(){
    if((taxi_shd_mem + x * SO_WIDTH + y)->cell_value == 2){ 
        request_check();
    } else {
        taxi_cleanup();
        exit(TAXI_REPLACED);
    }
}

void request_check(){
    int num_bytes, x_s = x, y_s = y, received = 0;

    num_bytes = msgrcv(msgqueue_id, &my_msgbuf, MSG_LEN, ((x_s * SO_WIDTH) + y_s) + 1, IPC_NOWAIT);
    if (num_bytes >= 0){
        x_to_go = atoi(my_msgbuf.mtext) / SO_WIDTH;
        y_to_go = atoi(my_msgbuf.mtext) % SO_WIDTH;
        received = 1;
    } else if (errno==ENOMSG){
        errno = 0;
    } else {
        printf("Errore durante la lettura del messaggio: %d", errno);
    }

    if(received){
        if(!taxi_ride()){
            //resend
            my_msgbuf.mtype = (x_s * SO_WIDTH) + y_s + 1;
            sprintf(my_msgbuf.mtext, "%d", (x_to_go * SO_WIDTH) + y_to_go);
            msgsnd(msgqueue_id, &my_msgbuf, MSG_LEN, 0);
            TEST_ERROR;
            flag_sigint = 1;
            raise(SIGQUIT);
        }
    } else {
        taxi_cleanup();
        exit(TAXI_REPLACED);
    }
}
int in_bounds(int x_check, int y_check){
    if (x_check >= 0 && x_check < SO_HEIGHT && y_check >= 0 && y_check < SO_WIDTH){
        return 1;
    }
    return 0;
}

int choose_direction(){
    if (x < x_to_go && in_bounds(x + 1, y) && (taxi_shd_mem + x+1 * SO_WIDTH + y)->cell_value != 0){
        x++;
        return 0;
    } else if (x > x_to_go && in_bounds(x - 1, y) && (taxi_shd_mem + x-1 * SO_WIDTH + y)->cell_value != 0){
        x--;
        return 1;
    } else if (y < y_to_go && in_bounds(x, y + 1) && (taxi_shd_mem + x * SO_WIDTH + y+1)->cell_value != 0){
        y++;
        return 2;
    } else if (y > y_to_go && in_bounds(x, y - 1 && (taxi_shd_mem + x * SO_WIDTH + y-1)->cell_value != 0)){
        y--;
        return 3;
    }

    return -1;
}

int taxi_ride(){
    int move_choice = -1, trip_time = 0, crossed_cells = 0, arrived = 0, looping = 0;

    sops[0].sem_op = 1; 
    sops[1].sem_op = -1; 

    while(!arrived){
        sops[0].sem_num = (x * SO_WIDTH) + y;
        if (x == x_to_go && y == y_to_go){
            arrived = 1;
        } else if ((move_choice = choose_direction()) == -1){
            taxi_cleanup();
            exit(TAXI_REPLACED);
        }
        
        if(!arrived){
            if(looping >= 2){
                semop(sem_cells_cap_id, sops, 1);
                TEST_ERROR;
                return 0;
            }

            sops[1].sem_num = (x * SO_WIDTH) + y;
            if(semtimedop(sem_cells_cap_id, sops, 2, &timeout) == -1){
                if(errno == EAGAIN){
                    //timeout
                    errno = 0;
                    semop(sem_cells_cap_id, sops, 1);
                    TEST_ERROR;
                    return 0;
                } else if(errno != EINTR && errno != EAGAIN){
                    TEST_ERROR;
                } else {
                    //revert move
                    looping++;
                    switch (move_choice){
                        case 0:
                            x--;
                            break;
                        case 1:
                            x++;
                            break;
                        case 2:
                            y--;
                            break;
                        case 3:
                            y++;
                            break;
                        default:
                            break;
                    }
                }
            } else {
                shdmem_return_sem_reserve(sem_sync_id);
                shd_mem_returned_stats->top_cells_map[x * SO_WIDTH + y]++;
                shdmem_return_sem_release(sem_sync_id);
                crossed_cells++;
                
                trip_time += (taxi_shd_mem + x * SO_WIDTH + y)->cell_timensec_value;
                timer.tv_nsec = (taxi_shd_mem + x * SO_WIDTH + y)->cell_timensec_value;
                nanosleep(&timer, NULL);
            }
        } 
    }
    printf("fatto\n");
    taxi_completed_trips++;

    shdmem_return_sem_reserve(sem_sync_id);
    shd_mem_returned_stats->trips_completed++; 
    if (crossed_cells > shd_mem_returned_stats->longest_trip) {
        shd_mem_returned_stats->longest_trip = crossed_cells;
        shd_mem_returned_stats->pid_longest_trip = getpid();
    }
    if (trip_time > shd_mem_returned_stats->slowest_trip) {
        shd_mem_returned_stats->slowest_trip = trip_time;
        shd_mem_returned_stats->pid_slowest_trip = getpid();
    } 
    
    if (taxi_completed_trips > shd_mem_returned_stats->max_trips_completed){
        shd_mem_returned_stats->max_trips_completed = taxi_completed_trips;
        shd_mem_returned_stats->pid_max_trips_completed = getpid();
    }
    shdmem_return_sem_release(sem_sync_id);

    return 1;
}