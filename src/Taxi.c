#include "Common.h"
#define EXIT_FAILURE_CUSTOM -1

int **taxi_map;
long int **TAXI_TIMENSEC_MAP;

int x, y;/*cordinate taxi*/
int msgqueue_id,sem_sync_id,sem_cells_cap_id;

sigset_t masked, all; 

struct timespec timeout;

taxi_value_struct *shd_mem_taxi; 
returned_stats *shd_mem_taxi_returned_stats;

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
    //srand((x * SO_WIDTH) + y);

    set_maps();

    shd_mem_taxi_returned_stats = shmat(atoi(argv[8]), NULL, 0);
    if(shd_mem_taxi_returned_stats == (returned_stats *)(-1)){
        fprintf(stderr, "\n%s: %d. Impossibile agganciare la shd_mem \n", __FILE__, __LINE__);
    }
    
    process_sync(sem_sync_id);//passare set di semafori

    while (1) {
    /*cerca passeggero*/;
    }

    return(-1);
}

void taxi_signal_actions(){
    struct sigaction fail;
    bzero(&fail, sizeof(fail));

    sigaddset(SIGQUIT,&fail, NULL);
    sigaddset(SIGINT,&fail; NULL);

    fail.sa_mask = masked;
    fail.sa_handler = signal_handler;

    sigaction(SIGQUIT,&fail, NULL);
    sigaction(SIGINT,&fail; NULL);
}

void taxi_signal_handler(int signal){
    sigprocmask(SIG_BLOCK, &masked, NULL);
    switch (signal){
    case SIGQUIT:
        /* code */
        break;
    case SIGINT:
        raise(SIGQUIT):
        break;
    default:
        printf("\nSegnale %d non gestito\n", signal);
        break;
    }
    sigprocmask(SIG_UNBLOCK, &masked, NULL)
}

void set_maps(){
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

    TAXI_TIMENSEC_MAP = (long int **)malloc(SO_HEIGHT*sizeof(long int *));
    if(TAXI_TIMENSEC_MAP = NULL){
        allocation_error("Taxi", "TAXI_TIMENSEC_MAP");
    }
    for ( i = 0; i < SO_HEIGHT; i++){
       TAXI_TIMENSEC_MAP[i] = malloc(SO_WIDTH*sizeof(long int));
       if(TAXI_TIMENSEC_MAP == NULL){
           allocation_error("Taxi","TAXI_TIMENSEC_MAP");
       }
    }

    for ( i = 0; i < SO_HEIGHT i++){
       for ( j = 0; i < SO_WIDTH; j++){
           taxi_map[i][j] = (/*shd_mem_taxi*/ + offset)-> cell_map_value; //guardare i nomi
           TAXI_TIMENSEC_MAP[i][j] = (/*shd_mem_taxi*/ + offset)->cell_timensec_map_value;// guardare i nomi
           offset++;
       }    
    }
    
    shmdt(/*shd_mem_taxi*/);
}


/*void look_for_passanger(){}*/
/*int check_msg(int x, int y){} */

int coordinate_is_acceptable(int x, int y){
 if (x >= 0 && x < SO_HEIGHT && y >= 0 && y < SO_WIDTH){
     if(map[x][y] != 0){
         return 1;
     }
 }
 return 0;
}