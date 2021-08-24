#include "Common.h"

//parameters
struct timespec timeout;

//maps
int **taxi_map;
long int **taxi_timensec_map;

//coordinates
int x = 0, y = 0;
int x_to_go = 0, y_to_go = 0;

//message queue
int msgqueue_id;

//semaphores
int sem_sync_id;
int sem_cells_cap_id;
struct sembuf sops[2];

//shared memory
taxi_value_struct *shd_mem_taxi; 

//signals
sigset_t mask;

//stats
int taxi_completed_trips = 0;

void taxi_signal_actions();
void taxi_signal_handler(int signum);
void taxi_maps_generator();
void taxi_map_free();
void taxi_timensec_map_free();
void taxi_free_all();
void customer_research();
int check_msg(int x, int y);
int in_bounds(int x, int y);
void taxi_ride();
void ride_stats();

int main(int argc, char *argv[]){
    if(argc != 9){
        fprintf(stderr, "\n%s: %d. NUMERO DI PARAMETRI ERRATO\n", __FILE__, __LINE__);
		exit(EXIT_FAILURE);
    }

    x = atoi(argv[1]); 
    y = atoi(argv[2]);
    if (x >= SO_HEIGHT){
        printf("Errore in Xt\n");
        exit(EXIT_FAILURE);
    }
    if (y >= SO_WIDTH){
        printf("Errore in Yt\n");
        exit(EXIT_FAILURE);
    }

    timeout.tv_sec = atoi(argv[3]);

    msgqueue_id = atoi(argv[4]);
    if (msgqueue_id == -1){
    printf("msgget error");
    }

    sem_sync_id = atoi(argv[5]);
    TEST_ERROR;
    sem_cells_cap_id = atoi(argv[6]);
    TEST_ERROR;

    shd_mem_taxi = shmat(atoi(argv[7]), NULL, 0);
    TEST_ERROR;
    
    shd_mem_returned_stats = shmat(atoi(argv[8]), NULL, 0);
    TEST_ERROR;

    taxi_maps_generator();
    taxi_signal_actions();
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
    if(sigaction(SIGINT, &sa_int, NULL) == -1){
        TEST_ERROR;
    }
}

void taxi_signal_handler(int signum){
    switch (signum){
    case SIGINT:
        ride_stats();
        taxi_free_all();
        exit(TAXI_ABORTED);
        break;
    default:
        printf("\nSegnale %d non gestito\n", signum);
        break;
    }
}

void taxi_maps_generator(){
    int x, y;
    int offset = 0;

    taxi_map = (int **)malloc(SO_HEIGHT * sizeof(int*));
    if(taxi_map == NULL){
        allocation_error("Taxi", "taxi_map");
    }
    for(x = 0; x < SO_HEIGHT; x++){
        taxi_map[x] = malloc(SO_WIDTH * sizeof(int));
        if(taxi_map[x] == NULL){
           allocation_error("Taxi", "taxi_map"); 
        }
    }

    taxi_timensec_map = (long int **)malloc(SO_HEIGHT * sizeof(long int *));
    if(taxi_timensec_map == NULL){
        allocation_error("Taxi", "taxi_timensec_map");
    }
    for (x = 0; x < SO_HEIGHT; x++){
       taxi_timensec_map[x] = malloc(SO_WIDTH*sizeof(long int));
       if(taxi_timensec_map == NULL){
           allocation_error("Taxi","taxi_timensec_map");
       }
    }

    for (x = 0; x < SO_HEIGHT; x++){
        for(y = 0; y < SO_WIDTH; y++){
            taxi_map[x][y] = (shd_mem_taxi + offset)->cell_value;
            taxi_timensec_map[x][y] = (shd_mem_taxi + offset)->cell_timensec_value;
            offset++;
        }
    }
    shmdt(shd_mem_taxi);
}

void taxi_map_free(){
    for (int i = 0; i < SO_HEIGHT; i++){
        free(taxi_map[i]);
    }
    free(taxi_map);
}

void taxi_timensec_map_free(){
    for (int i = 0; i < SO_HEIGHT; i++){
        free(taxi_timensec_map[i]);
    }
    free(taxi_timensec_map);
}

void taxi_free_all(){
    sops[0].sem_num = (x * SO_WIDTH) + y; 
    sops[0].sem_op = 1;
    semop(sem_cells_cap_id, sops, 1);

    taxi_map_free();
    taxi_timensec_map_free();
}

void customer_research(){
    if(check_msg(x,y)){ 
        printf("messaggio raccolto\n");
        taxi_ride();
    } else {
        ride_stats();
        taxi_free_all();
        exit(REPLACE_TAXI);
    }
}

int check_msg(int x, int y){
    int num_bytes;
    sigset_t masked;

    sigfillset(&masked);
    sigprocmask(SIG_BLOCK, &masked, NULL);

    num_bytes = msgrcv(msgqueue_id, &my_msgbuf, MSG_MAX_SIZE, ((x * SO_WIDTH) + y) + 1, IPC_NOWAIT);
    //printf(" bytes: %d\n", num_bytes);
    if (num_bytes > 0){
        printf(" prima è x = %d, y = %d\n", x_to_go, y_to_go);
        x_to_go = atoi(my_msgbuf.mtext) / SO_WIDTH;
        y_to_go = atoi(my_msgbuf.mtext) % SO_WIDTH;
        printf(" dopo è x = %d, y = %d\n", x_to_go, y_to_go);
        sigprocmask(SIG_UNBLOCK, &masked, NULL);
        if (x < SO_HEIGHT && y < SO_WIDTH){  
            return 1;
        }  
    }else if (num_bytes <= 0 && errno!=ENOMSG){
        printf("Errore durante la lettura del messaggio: %d", errno);
    }else if (errno == EINTR) {
		printf("the process caught a signal while sleeping\n");
        exit(0);
	}else if (errno == EIDRM) {
		printf("La coda di messagi (id: %d ) è stata rimossa\n", msgqueue_id);
		exit(0);
		}
    sigprocmask(SIG_UNBLOCK, &masked, NULL);

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
    int mov_choice, trip_time = 0, crossed_cells = 0, arrived = 0, res = -1;
    struct timespec timer;
    timer.tv_sec = 0;

    //0 -> used to release current cell; 1 -> //used to reserve the cell I'm moving to
    sops[0].sem_op = 1; 
    sops[1].sem_op = -1; 
printf("    CI SONO\n");
    while(!arrived){
        printf("comincio a muovermi\n");

        sops[0].sem_num = (x * SO_WIDTH) + y;
        //printf(" parte da x = %d, y = %d\n", x, y);
       // printf(" vado a x = %d, y = %d\n", x_to_go, y_to_go);
        if (y == y_to_go && x == x_to_go){
            printf("arrivato?\n");
            arrived = 1;
        } else if (x < x_to_go && in_bounds(x + 1, y)){
            x++;
            mov_choice = 0;
            printf("choice %d\n", mov_choice);
        } else if (x > x_to_go && in_bounds(x - 1, y)){
            x--;
            mov_choice = 1;
            printf("choice %d\n", mov_choice);
        } else if (y < y_to_go && in_bounds(x, y + 1)){
            y++;
            mov_choice = 2;
            printf("choice %d\n", mov_choice);
        } else if (y > y_to_go && in_bounds(x, y - 1)){
            y--;
            mov_choice = 3;
            printf("choice %d\n", mov_choice);
        } else {
            printf("sono fermo??\n\n");
        }

        if(!arrived){
            printf("semtimedop\n");
            sops[1].sem_num = (x * SO_WIDTH) + y;
            printf(" vado in x = %d, y = %d\n", x, y);

            if(semtimedop(sem_cells_cap_id, sops, 2, &timeout) == -1){
                if(errno == EAGAIN){
                    //over timeout
                    printf("timeout?\n");
                    raise(SIGINT);
                } else if(errno != EINTR && errno != EAGAIN){
                    printf("errore?\n\n\n\n\\n");  
                    TEST_ERROR;
                } else {
                    printf("riavvolgo?\n");
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
        } else {
            printf("Passeggero a destinazione\n\n\n\n");
        }
    }
    
    taxi_completed_trips++;
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

void ride_stats(){
    shdmem_return_sem_reserve(sem_sync_id);
    shd_mem_returned_stats->trips_completed += taxi_completed_trips;
    if (taxi_completed_trips > shd_mem_returned_stats->max_trips_completed){
        shd_mem_returned_stats->max_trips_completed = taxi_completed_trips;
        shd_mem_returned_stats->pid_max_trips_completed = getpid();
    }
    shdmem_return_sem_release(sem_sync_id);
}