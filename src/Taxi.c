#include "Common.h"
#define EXIT_FAILURE_CUSTOM -1

int **taxi_map;
long int **taxi_timensec_map;

int x, y;/*cordinate taxi*/
int X, Y;
int msgqueue_id,sem_sync_id,sem_cells_cap_id;

int completed_trips = 0;

sigset_t mask, all; 

struct timespec timeout;

taxi_value_struct *shd_mem_taxi; 
//semaphores
struct sembuf sops[2];

void taxi_signal_actions();
void taxi_signal_handler(int signum);
void taxi_maps_generator();
void customer_research();
int check_msg(int x, int y);
int in_bounds(int x, int y);
void taxi_ride();

int main(int argc, char const *argv[]){
    if(argc != 9){
        fprintf(stderr, "\n%s: %d. NUMERO DI PARAMETRI ERRATO\n", __FILE__, __LINE__);
		exit(EXIT_FAILURE_CUSTOM);
    }

    taxi_signal_actions();
    sigfillset(&all);

    x = atoi(argv[1]); 
    y = atoi(argv[2]);
    
    timeout.tv_sec = atoi(argv[3]);

    msgqueue_id = atoi(argv[4]);
    sem_sync_id = atoi(argv[5]);
    sem_cells_cap_id = atoi(argv[6]);

    shd_mem_taxi = shmat(atoi(argv[7]), NULL, 0);
    TEST_ERROR;
    
    taxi_maps_generator();

    shd_mem_returned_stats = shmat(atoi(argv[8]), NULL, 0);
    TEST_ERROR;
    
    processes_sync(sem_sync_id);

    while (1) {
        customer_research();
    }
}

void taxi_signal_actions(){
    struct sigaction sa_int;
    
    sigemptyset(&mask);
    sigaddset(&mask,SIGINT);

    sigprocmask(SIG_BLOCK, &mask, NULL);

    sa_int.sa_mask = mask;
    sa_int.sa_handler = taxi_signal_handler;
    sa_int.sa_flags = 0;

    sigprocmask(SIG_UNBLOCK, &mask, NULL);
    sigaction(SIGINT, &sa_int, NULL);
}

void taxi_signal_handler(int signum){
    switch (signum){
    case SIGINT:
        //stats     
        sops[0].sem_num = (x * SO_WIDTH) + y; 
        sops[0].sem_op = 1;
        semop(sem_cells_cap_id, sops, 1);
        //malloc free
        free_up_memory_of_taxi_map(taxi_map);
        free_up_memory_of_taxi_timensec_map(taxi_timensec_map);
        //
        exit(TAXI_ABORTED);
        break;
    default:
        printf("\nSegnale %d non gestito\n", signum);
        break;
    }
}

void taxi_maps_generator(){
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

    taxi_timensec_map = (long int **)malloc(SO_HEIGHT*sizeof(long int *));
    if(taxi_timensec_map = NULL){
        allocation_error("Taxi", "taxi_timensec_map");
    }
    for ( i = 0; i < SO_HEIGHT; i++){
       taxi_timensec_map[i] = malloc(SO_WIDTH*sizeof(long int));
       if(taxi_timensec_map == NULL){
           allocation_error("Taxi","taxi_timensec_map");
       }
    }

    for ( i = 0; i < SO_HEIGHT; i++){
       for ( j = 0; i < SO_WIDTH; j++){
           taxi_map[i][j] = (shd_mem_taxi + offset)-> cell_value; 
           taxi_timensec_map[i][j] = (shd_mem_taxi + offset)->cell_timensec_value;
           offset++;
       }    
    }
    
    shmdt(shd_mem_taxi);
}

void free_up_memory_of_taxi_map(int **taxi_map){
    for (int i = 0; i < SO_HEIGHT; i++){
        free(taxi_map[i]);
    }
    free(taxi_map);
}

void free_up_memory_of_taxi_timensec_map(long int **taxi_timensec_map){
    for (int i = 0; i < SO_HEIGHT; i++){
        free(taxi_timensec_map[i]);
    }
    free(taxi_timensec_map);
}

void customer_research(){
    if(check_msg(x,y)){ 
        taxi_ride();
    } else {
        sops[0].sem_num = (x * SO_WIDTH) + y; 
        sops[0].sem_op = 1;
        semop(sem_cells_cap_id, sops, 0);
        //stats
        //malloc free
        exit(REPLACE_TAXI);
    }
}

int check_msg(int x, int y){
    int num_bytes;

    num_bytes = msgrcv(msgqueue_id, &buf_msg, MSG_MAX_SIZE, ((x * SO_WIDTH) + y) + 1, IPC_NOWAIT);    
    if (num_bytes >= 0){
        X = atoi(buf_msg.mtext) / SO_WIDTH;
        Y = atoi(buf_msg.mtext) % SO_WIDTH;
        return 1;
    } else if (num_bytes <= 0 && errno!=ENOMSG){
        printf("Errore durante la lettura del messaggio: %d", errno);
    }

    return 0;
}

int in_bounds(int x, int y){
    if (x >= 0 && x < SO_HEIGHT && y >= 0 && y < SO_WIDTH){
        if(taxi_map[x][y] != 0){
            return 1;
        }
    }
    return 0;
}

void taxi_ride(){
    int mov_choice, trip_time = 0, crossed_cells = 0, arrived = 0;
    struct timespec timer;
    timer.tv_sec = 0;

    //used to release current cell
    sops[0].sem_op = 1; 
    //used to reserve the cell I'm moving to
    sops[1].sem_op = -1; 

    while(!arrived){
        sops[0].sem_num = (x * SO_WIDTH) + y;
        
        if (y == Y && x == X){
            arrived = 1;
        } else if (x < X){
            if (in_bounds(x + 1, y)){
                x++;
                mov_choice = 0;
            }
        } else if (x > X){
            if (in_bounds(x - 1, y)){
                x--;
                mov_choice = 1;
            }
        } else if (y < Y){
            if (in_bounds(x, y + 1)){
                y++;
                mov_choice = 2;
            }
        } else if (y > Y){
            if (in_bounds(x, y - 1)){
                y--;
                mov_choice = 3;
            }
        }

        if(!arrived){
            sops[1].sem_num = (x * SO_WIDTH) + y;
            if(semtimedop(sem_cells_cap_id, sops, 2, &timeout) == -1){
                if(errno == EAGAIN){
                    //over timeout
                    raise(SIGINT);
                } else if(errno != EINTR && errno != EAGAIN){
                    TEST_ERROR;
                } else {
                    switch (mov_choice){
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
                    }
                }
            } else {
                shd_mem_returned_stats->top_cells_map[x][y]++;
                trip_time += taxi_timensec_map[x][y];
                timer.tv_nsec = taxi_timensec_map[x][y];
                crossed_cells++;
                nanosleep(&timer, NULL);
            }
        }
    }
    completed_trips++;
    if (crossed_cells > shd_mem_returned_stats->longest_trip) {
        shdmem_return_sem_reserve(sem_sync_id);
        shd_mem_returned_stats->longest_trip = crossed_cells;
        shd_mem_returned_stats->pid_longest_trip = getpid();
        shdmem_return_sem_release(sem_sync_id);
    }
    if (trip_time > shd_mem_returned_stats->slowest_trip) {
        shdmem_return_sem_reserve(sem_sync_id);
        shd_mem_returned_stats->slowest_trip = trip_time;
        shd_mem_returned_stats->pid_slowest_trip = getpid();
        shdmem_return_sem_release(sem_sync_id);
    } 
}

void travel_information(){
    shdmem_return_sem_reserve(sem_sync_id);
    shd_mem_returned_stats->max_trips_completed = shd_mem_returned_stats->max_trips_completed + shd_mem_returned_stats->trips_completed;
    /*code*/
    shdmem_return_sem_release(sem_sync_id);
}