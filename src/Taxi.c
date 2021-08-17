#include "Common.h"
#define EXIT_FAILURE_CUSTOM -1

int **taxi_map;
long int **taxi_timensec_map;

int x, y;/*cordinate taxi*/
int X, Y;
int msgqueue_id,sem_sync_id,sem_cells_cap_id;



sigset_t mask, all; 

struct timespec timeout;

taxi_value_struct *shd_mem_taxi; 

//semaphores
struct sembuf sops[2];

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
    if(shd_mem_taxi == (taxi_value_struct *)(-1)){
        fprintf(stderr, "\n%s: %d. Impossibile agganciare la shd_mem \n", __FILE__, __LINE__);
    }
    taxi_maps_generator();

    shd_mem_returned_stats = shmat(atoi(argv[8]), NULL, 0);
    if(shd_mem_returned_stats == (returned_stats *)(-1)){
        fprintf(stderr, "\n%s: %d. Impossibile agganciare la shd_mem \n", __FILE__, __LINE__);
    }
    
    process_sync(sem_sync_id);//passare set di semafori

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
        exit(0);
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

    for ( i = 0; i < SO_HEIGHT i++){
       for ( j = 0; i < SO_WIDTH; j++){
           taxi_map[i][j] = (shd_mem_taxi + offset)-> cell_map_value; //guardare i nomi
           taxi_timensec_map[i][j] = (shd_mem_taxi + offset)->cell_timensec_map_value;// guardare i nomi
           offset++;
       }    
    }
    
    shmdt(shd_mem_taxi);
}

void customer_research(){
    if(check_masg(x,y)){ 
        start_a_trip();
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
    int msg_ready = 0, num_bytes;
    struct msgbuf msg_snd_buf;

    //lock_signals(all);

    num_bytes = msgrcv(msgqueue_id, &msg_snd_buf, MSG_LEN, ((x * SO_WIDTH) + y) + 1, IPC_NOWAIT);
        
    if (num_bytes >= 0){
        X = atoi(msg_snd_buf.mtext) / SO_WIDTH;
        Y = atoi(msg_snd_buf.mtext) % SO_WIDTH;
        return 1;
    } else if (num_bytes <= 0 && errno!=ENOMSG){
        printf("Errore durante la lettura del messaggio: %d", errno);
    }

    //unlock_signals(all);

    return 0;
}

int in_bounds(int x, int y){
    if (x >= 0 && x < SO_HEIGHT && y >= 0 && y < SO_WIDTH){
        if(map[x][y] != 0){
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
        
        if (y == y_dest && x == x_dest){
            arrived = 1;
        } else if (x < x_dest){
            if (coordinate_is_acceptable(x + 1, y)){
                x++;
                mov_choice = 0;
            }
        } else if (x > x_dest){
            if (coordinate_is_acceptable(x - 1, y)){
                x--;
                mov_choice = 1;
            }
        } else if (y < y_dest){
            if (coordinate_is_acceptable(x, y + 1)){
                y++;
                mov_choice = 2;
            }
        } else if (y > y_dest){
            if (coordinate_is_acceptable(x, y - 1)){
                y--;
                mov_choice = 3;
            }
        }

        if(!arrived){
            sops[1].sem_num = (x * SO_WIDTH) + y;
            if(semtimedop(sem_cells_cap_id, sops, 2, &timeout) == -1){
                if(errno == EAGAIN){
                    //over timeout
                    sops[0].sem_num = (x * SO_WIDTH) + y; 
                    sops[0].sem_op = 1;
                    semop(sem_cells_cap_id, sops, 1);
                    //stats
                    //malloc free
                    exit(TAXI_ABORTED);
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
                shd_mem_returned_stats->so_top_cells_map[x][y]++;
                trip_time += taxi_timensec_map[x][y];
                timer.tv_nsec = taxi_timensec_map[x][y];
                crossed_cells++;
                nanosleep(&timer, NULL);
            }
        }
    }
    completed_trips++;
    if (crossed_cells > shd_mem_returned_stats->longest_trip) {
        taxi_return_reserve_sem(sem_sync_id);
        shd_mem_returned_stats->longest_trip = crossed_cells;
        shd_mem_returned_stats->pid_longest_trip = getpid();
        taxi_return_release_sem(sem_sync_id);
    }
    if (trip_time > shd_mem_returned_stats->slowest_trip) {
        taxi_return_reserve_sem(sem_sync_id);
        shd_mem_returned_stats->slowest_trip = trip_time;
        shd_mem_returned_stats->pid_slowest_trip = getpid();
        taxi_return_release_sem(sem_sync_id);
    } 
}