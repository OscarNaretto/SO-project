#include "Common.h"
#define PARAMETERS "./ParametersLarge.txt"

void setup();
void read_parameters();
void master_maps_generator();
void master_map_initialization();
int can_be_placed(int x, int y);
void msgqueue_generator();
void semaphore_generator();
void shd_memory_generator();
void shd_memory_initialization();
void source_processes_generator();
void taxi_processes_generator();
void taxi_processes_regenerator(pid_t to_regen);
void master_signal_management();
void master_handle_signal(int signal);
//usr signal handler
void run();

//parameters
int SO_HOLES = -1;
int SO_TOP_CELLS = -1;
int SO_SOURCES = -1;
int SO_CAP_MIN = -1;
int SO_CAP_MAX = -1;
int SO_TAXI = -1;
long int SO_TIMENSEC_MIN = -1;
long int SO_TIMENSEC_MAX = -1;
int SO_TIMEOUT = -1;
int SO_DURATION = -1;
int SO_INIT_REQUESTS_MIN = -1;
int SO_INIT_REQUESTS_MAX  = -1;

//maps
int **master_map;
int **master_cap_map;
long int **master_timensec_map;

//message queue
int msgqueue_id; 

//semaphores
union semun arg;
struct sembuf sops;

int sem_cells_cap_id;
int sem_sync_id; 

//shared memory
int shd_mem_to_source_id;
int shd_mem_to_taxi_id; 
int shd_mem_returned_stats_id; 
source_value_struct *shd_mem_to_source;
taxi_value_struct *shd_mem_to_taxi;

//processes
pid_t *sources_pid_array;
pid_t *taxis_pid_array;

int main(int argc, char *argv[]){
    setup();

    //syncing with taxi and sources processes
    sops.sem_num = 0;
    sops.sem_op = 0; 
    sops.sem_flg = 0;
    semop(sem_sync_id, &sops, 0);

    //all the processes are generated and ready to run
    run();
    
    //print_map

    //free malloc and ipcs
}

void setup(){

    //parameters reading and check
    read_parameters();

    //maps generation
    master_maps_generator();

    //ipcs initialization
    msgqueue_generator();
    semaphore_generator();
    shd_memory_generator();

    //sources and taxis process generation; pids are stored in two arrays
    sources_pid_array = (pid_t *)malloc(SO_SOURCES * sizeof(pid_t));
    for (int i = 0; i< SO_SOURCES; i++){
        sources_pid_array[i] = 0;
    }
    source_processes_generator();

    taxis_pid_array = (pid_t *)malloc(SO_TAXI * sizeof(pid_t));
    for (int i = 0; i< SO_SOURCES; i++){
        taxis_pid_array[i] = 0;
    }
    taxi_processes_generator();

    master_signal_management();
}

void read_parameters(){
    FILE *f;
    f = fopen(PARAMETERS , "r");

    if(f == NULL){
        printf("Errore  nell'apertura del file contenente i parametri\n");
		exit(EXIT_FAILURE);
    }

    fscanf(f, "%*s %d\n", SO_HOLES);
    fscanf(f, "%*s %d\n", SO_TOP_CELLS);
    fscanf(f, "%*s %d\n", SO_SOURCES);
    fscanf(f, "%*s %d\n", SO_CAP_MIN);
    fscanf(f, "%*s %d\n", SO_CAP_MAX);
    fscanf(f, "%*s %d\n", SO_TAXI);
    fscanf(f, "%*s %d\n", SO_TIMENSEC_MIN);
    fscanf(f, "%*s %d\n", SO_TIMENSEC_MAX);
    fscanf(f, "%*s %d\n", SO_TIMEOUT);
    fscanf(f, "%*s %d\n", SO_DURATION);
    fscanf(f, "%*s %d\n", SO_INIT_REQUESTS_MIN);
    fscanf(f, "%*s %d\n", SO_INIT_REQUESTS_MAX);

    test_parameters();
}

void test_parameters(){
    if(SO_HOLES < 0){
        printf("Errore nel caricamento del parametro SO_HOLES: parametro negativo\n");
    }
    if(SO_TOP_CELLS < 0){
        printf("Errore nel caricamento del parametro SO_TOP_CELLS: parametro negativo\n");
    }
    if(SO_SOURCES < 0){
        printf("Errore nel caricamento del parametro SO_SOURCES: parametro negativo\n");
    }
    if((SO_SOURCES + SO_HOLES) > (SO_WIDTH * SO_HEIGHT)){
        printf("Errore nei parametri della mappa: troppi SO_SOURCES e SO_HOLES per la grandezza della mappa data");
    }
    if(SO_CAP_MIN <= 0){
        printf("Errore nel caricamento del parametro SO_CAP_MIN: il parametro deve essere maggiore di zero\n");
    }
    if(SO_CAP_MAX <= 0){
        printf("Errore nel caricamento del parametro SO_CAP_MAX: il parametro deve essere maggiore di zero\n");
    }
    if(SO_CAP_MAX < SO_CAP_MIN){
        printf("Errore: differenza tra capacità massima e capacità minima non valida\n");
    }
    if(SO_TAXI < 0){
        printf("Errore nel caricamento del parametro SO_TAXI: parametro negativo\n");
    }
    if(SO_TAXI > (SO_CAP_MIN * SO_HEIGHT * SO_WIDTH)){
        printf("Errore: SO_TAXI supera la dimensione massima consentita\n");
    }
    if(SO_TIMENSEC_MIN <= 0){
        printf("Errore nel caricamento del parametro SO_TIMENSEC_MIN: il parametro deve essere maggiore di zero\n");
    }
    if(SO_TIMENSEC_MAX <= 0){
        printf("Errore nel caricamento del parametro SO_TIMENSEC_MAX: il parametro deve essere maggiore di zero\n");
    }
    if(SO_TIMENSEC_MAX < SO_TIMENSEC_MIN){
        printf("Errore: differenza tra attesa massima e minima non valida\n");
    }
    if(SO_TIMEOUT <= 0){
        printf("Errore nel caricamento del parametro SO_TIMEOUT: il parametro deve essere maggiore di zero\n");
    }
    if(SO_DURATION <= 0){
        printf("Errore nel caricamento del parametro SO_DURATION: il parametro deve essere maggiore di zero\n");
    }
    if(SO_INIT_REQUESTS_MIN <= 0){
        printf("Errore nel caricamento del parametro SO_INIT_REQUESTS_MIN: il parametro deve essere maggiore di zero\n");
    }
    if(SO_INIT_REQUESTS_MAX <= 0){
        printf("Errore nel caricamento del parametro SO_INIT_REQUESTS_MAX: il parametro deve essere maggiore di zero\n");
    }
    if(SO_INIT_REQUESTS_MAX < SO_INIT_REQUESTS_MIN){
        printf("Errore: differenza tra il numero di richieste massime e minime per source non valida\n");
    }
}

void master_maps_generator(){
    int i, j;

    //memory allocation of the main map
    master_map = (int **)malloc(SO_HEIGHT * sizeof(int *));
    if (master_map == NULL){ allocation_error("Master", "map"); }
    for (i = 0; i < SO_HEIGHT; i++){
        master_map[i] = malloc(SO_WIDTH * sizeof(int));
        if (master_map[i] == NULL){
            allocation_error("Master", "map");
        } else {
            for (j = 0; j < SO_WIDTH; j++){
                master_map[i][j] = 1; 
            } 
        }
    }

    //holes and sources generator on main map
    master_map_initialization();

    //memory allocation of maximum cell capability
    master_cap_map = (int **)malloc(SO_HEIGHT * sizeof(int *));
    if (master_cap_map == NULL){ allocation_error("Master", "master_cap_map"); }
    for (i = 0; i < SO_HEIGHT; i++){
        master_cap_map[i] = malloc(SO_WIDTH * sizeof(int));
        if (master_cap_map[i] == NULL){
            allocation_error("Master", "master_cap_map");
        } else {
            for(j = 0; j < SO_WIDTH; j++){
                master_cap_map[i][j] = rand() % (SO_CAP_MAX + 1 - SO_CAP_MIN) + SO_CAP_MIN;
            }
        }        
    }

    //memory allocation of maximum cell waiting time
    master_timensec_map = (long int **)malloc(SO_HEIGHT * sizeof(long int *));
    if (master_timensec_map == NULL){ allocation_error("Master", "master_timensec_map"); }
    for (i = 0; i < SO_HEIGHT; i++){
        master_timensec_map[i] = malloc(SO_WIDTH * sizeof(long int));
        if (master_timensec_map[i] == NULL){
            allocation_error("Master", "master_timensec_map");
        } else {
            for (j = 0; j < SO_WIDTH; j++){
                master_timensec_map[i][j] = rand() % (SO_TIMENSEC_MAX + 1 - SO_TIMENSEC_MIN) + SO_TIMENSEC_MIN;
            }
        }
    }
}

void master_map_initialization(){
    int x, y, placed, not_placed;

    //holes
    placed = not_placed = 0;
    while(placed < SO_HOLES && not_placed < SO_HEIGHT * SO_WIDTH){
        x = rand() % SO_HEIGHT; 
        y = rand() % SO_WIDTH;

        if(master_map[x][y] != 0){         
            if(can_be_placed(x, y)){
                master_map[x][y] = 0;
                placed++;
            } else {
                not_placed++;
            }
        }
    }
    if (placed < SO_HOLES){
        printf("SO_HOLES: Parametro non valido; non è possibile piazzare tutti gli HOLES");
    }

    //sources
    placed = 0;
    not_placed = 1;
    while (placed < SO_SOURCES){
        while (not_placed){
            x = rand() % SO_HEIGHT;
            y = rand() % SO_WIDTH;
            if (master_map[x][y] == 1){
                master_map[x][y] = 2;
                not_placed = 0;
            }
        }
        not_placed = 1;
        placed++;
    }
}

int can_be_placed(int x, int y){ 
    if (x > 0 && y > 0 && master_map[x - 1][y - 1] == 0){ return 0;}
    if (x > 0 && master_map[x - 1][y] == 0){ return 0;}
    if (x > 0 && y < SO_WIDTH - 1 && master_map[x - 1][y + 1] == 0){ return 0;}
    if (x < SO_HEIGHT - 1 && y > 0 && master_map[x + 1][y - 1] == 0){ return 0;}
    if (x < SO_HEIGHT - 1 && master_map[x + 1][y] == 0){ return 0;}
    if (x < SO_HEIGHT - 1 && y < SO_WIDTH - 1 && master_map[x + 1][y + 1] == 0){ return 0;}
    if (y > 0 &&  master_map[x][y-1] == 0){ return 0;}
    if (y < SO_WIDTH - 1 && master_map[x][y + 1] == 0){ return 0;}

    return 1;
}

void msgqueue_generator(){
    msgqueue_id = msgget(IPC_PRIVATE, IPC_CREAT | IPC_EXCL | 0666);
    TEST_ERROR;
}

void semaphore_generator(){
    //semaphore set for each cell; stores cell capability; matrix as an array
    sem_cells_cap_id = semget(IPC_PRIVATE, SO_HEIGHT * SO_WIDTH, IPC_CREAT | IPC_EXCL| 0666);
    TEST_ERROR;

    for(int i = 0; i < SO_HEIGHT * SO_WIDTH; i++){
        arg.val = master_cap_map[i / SO_WIDTH][i % SO_WIDTH];
        semctl(sem_cells_cap_id, i, SETVAL, arg);
        TEST_ERROR;
    } 

    //semaphore set used in order to sync processes and ipcs usage
    sem_sync_id = semget(IPC_PRIVATE, 2, IPC_CREAT | IPC_EXCL | 0666);
    TEST_ERROR;

    //processes sync semaphore
    arg.val = SO_SOURCES + SO_TAXI;
    semctl(sem_sync_id, 0, SETVAL, arg);
    TEST_ERROR;
    
    //returned values shd_memory sync semaphore
    arg.val = 1;
    semctl(sem_sync_id, 1, SETVAL, arg);
    TEST_ERROR;
}

void shd_memory_generator(){
    //shd_mem used to pass master_map values to source processes
    shd_mem_to_source_id = shmget(IPC_PRIVATE, SO_HEIGHT * SO_WIDTH * sizeof(source_value_struct), IPC_CREAT | IPC_EXCL | 0666);
    TEST_ERROR;
    shd_mem_to_source = shmat(shd_mem_to_source_id, NULL, 0);
    TEST_ERROR;

    //shd_mem used to pass master_map values and timensec_map values to taxi processes
    shd_mem_to_taxi_id = shmget(IPC_PRIVATE, SO_HEIGHT * SO_WIDTH * sizeof(taxi_value_struct), IPC_CREAT | IPC_EXCL | 0666);
    TEST_ERROR;
    shd_mem_to_taxi = shmat(shd_mem_to_taxi_id, NULL, 0);
    TEST_ERROR;
    
    //shd_mem to return taxi stats
    shd_mem_returned_stats_id = shmget(IPC_PRIVATE, sizeof(returned_stats), IPC_CREAT | IPC_EXCL | 0666);
    TEST_ERROR;
    shd_mem_returned_stats = shmat(shd_mem_returned_stats_id, NULL, 0);
    TEST_ERROR;

    //initialization of shd_mem structs values
    shd_memory_initialization();
}

void shd_memory_initialization(){
    int x, y, offset = 0;

    shd_mem_returned_stats->trips_completed = 0;
    shd_mem_returned_stats->longest_trip = 0;
    shd_mem_returned_stats->slowest_trip = 0;
    shd_mem_returned_stats->max_trips_completed = 0;

    for(x = 0; x < SO_HEIGHT; x++){
        for(y = 0; y < SO_WIDTH; y++){
            (shd_mem_to_source + offset)->cell_value = master_map[x][y];
            (shd_mem_to_taxi+offset)->cell_value = master_map[x][y];
            (shd_mem_to_taxi+offset)->cell_timensec_value = master_timensec_map[x][y];
            shd_mem_returned_stats->top_cells_map[x][y] = 0;
            offset++;
        }
    }

    shmctl(shd_mem_to_source_id, IPC_RMID, NULL);
    shmctl(shd_mem_to_taxi_id, IPC_RMID, NULL);
    shmctl(shd_mem_returned_stats_id, IPC_RMID, NULL);
}

void source_processes_generator(){
    int i = -1, x, y;

    for (x = 0; x < SO_HEIGHT; x++){
        for (y = 0; y< SO_WIDTH; y++){
            if (master_map[x][y] == 2){
                i++;
                switch(sources_pid_array[i] = fork()){
                    case -1:
                        fprintf(stderr,"Error #%03d: %s\n", errno, strerror(errno));
                        exit(EXIT_FAILURE);
                        break;
                    
                    case 0:
                        char* source_args[] = {
                            "Source",
                            (char)x,
                            (char)y,
                            (char)msgqueue_id,
                            (char)sem_sync_id,
                            (char)shd_mem_to_source_id,
                            (char)SO_INIT_REQUESTS_MIN,
                            (char)SO_INIT_REQUESTS_MAX,
                            NULL
                        };

                        execve("bin/Source", source_args, NULL);

	                    fprintf(stderr, "%s: %d. Error #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
	                    exit(EXIT_FAILURE);
                        break;
                    
                    default:
                        break;
                }
            }
        }
    }
}

void taxi_processes_generator(){
    int i, x, y, generated;
    srand(time(NULL));

    for (i = 0; i < SO_TAXI; i++){
        generated = 0;
        while(!generated){
            x = rand() % SO_HEIGHT;
            y = rand() % SO_WIDTH;
            if(map[x][y] != 0){
                sops.sem_num = (x * SO_WIDTH) + y; 
                sops.sem_op = -1;
                sops.sem_flg = IPC_NOWAIT;
                if(semop(sem_sync_id, &sops, 1) == -1){
                    if(errno != EAGAIN && errno != EINTR){
                        TEST_ERROR;
                    }
                } else {
                    generated = 1;
                }
            }
        }
        
        switch(taxis_pid_array[i] = fork()){
            case -1:
                fprintf(stderr,"Error #%03d: %s\n", errno, strerror(errno));
                exit(EXIT_FAILURE);
                break;
                    
            case 0:
                char* taxi_args[] = {
                    "Taxi",
                    (char)x,
                    (char)y,
                    (char)SO_TIMEOUT,
                    (char)msgqueue_id,
                    (char)sem_sync_id,
                    (char)sem_cells_cap_id,
                    (char)shd_mem_to_taxi_id,
                    (char)shd_mem_returned_stats_id,
                    NULL
                };

                execve("bin/Taxi", taxi_args, NULL);

	            fprintf(stderr, "%s: %d. Error #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
	            exit(EXIT_FAILURE);
                break;
                    
            default:
                break;
        }
    }
}

void taxi_processes_regenerator(pid_t to_regen){
    int i, x, y, generated = 0; 
    srand(time(NULL));

    for(i = 0; i < SO_TAXI; i++){
        if(taxis_pid_array[i] == to_regen){
            break;
        }  
    }

    while(!generated){
        x = rand() % SO_HEIGHT;
        y = rand() % SO_WIDTH;
        if(map[x][y] != 0){
            sops.sem_num = (x * SO_WIDTH) + y; 
            sops.sem_op = -1;
            sops.sem_flg = IPC_NOWAIT;
            if(semop(sem_sync_id, &sops, 1) == -1){
                if(errno != EAGAIN && errno != EINTR){
                    TEST_ERROR;
                }
            } else {
                generated = 1;
            }
        }
    }

    switch(taxis_pid_array[i] = fork()){
        case -1:
            fprintf(stderr,"Error #%03d: %s\n", errno, strerror(errno));
            exit(EXIT_FAILURE);
            break;
                    
        case 0:
            char* taxi_args[] = {
                "Taxi",
                (char)x,
                (char)y,
                (char)SO_TIMEOUT,
                (char)msgqueue_id,
                (char)sem_sync_id,
                (char)sem_cells_cap_id,
                (char)shd_mem_to_taxi_id,
                (char)shd_mem_returned_stats_id,
                NULL
            };

            execve("bin/Taxi", taxi_args, NULL);

	        fprintf(stderr, "%s: %d. Error #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
	        exit(EXIT_FAILURE);
            break;
                    
        default:
            break;
    }
}

master_signal_management(){
    struct sigaction sa_alarm, sa_int; 

}

void run(){
    //using a one sec alarm to print the map
    alarm(1);

    //collect stats from exit status of taxis
    
    //print last map and final stats

}
  