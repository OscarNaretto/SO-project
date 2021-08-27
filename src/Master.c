#include "Common.h"
#define PARAMETERS "./Parameters.txt"

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
int **master_map = 0;
int **master_cap_map = 0;


//message queue
int msgqueue_id = 0; 

//semaphores
union semun arg;
struct sembuf sops;
int sem_cells_cap_id = 0;
int sem_sync_id = 0; 

//shared memory
int shd_mem_to_source_id = 0;
int shd_mem_to_taxi_id = 0; 
int shd_mem_returned_stats_id = 0; 
source_value_struct *shd_mem_to_source = 0;
taxi_value_struct *shd_mem_to_taxi = 0;
returned_stats *shd_mem_returned_stats = 0;


//processes
pid_t *sources_pid_array = 0;
pid_t *taxis_pid_array = 0;

//stats
int execution_time = 0;
int total_requests = 0;
int unresolved_trips = 0; //unresolved_trips = total_requests - completed_trips
int taxi_aborted = 0;
int taxi_replaced = 0;
int killed_t = 0;

void setup();
void read_parameters();
void test_parameters();
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
void master_signal_actions();
void master_handle_signal(int signum);
void print_master_map();
void run();
void print_stats();
void master_map_free();
void master_cap_map_free();
void master_process_array_free();
void master_free_all();
void free_ipcs();
void memory_cleanup();
void processes_kill();
void load_top_cells();

int main(int argc, char *argv[]){
    setup();

    //syncing with taxi and sources processes
    sops.sem_num = 0;
    sops.sem_op = 0; 
    sops.sem_flg = 0;
    semop(sem_sync_id, &sops, 1);
    
    //all the processes are generated and ready to run
    run();
    
    //load_top_cells();        
    print_master_map();
    print_stats();

    memory_cleanup();
    return 0;
}

void setup(){
    int i;
    srand(getpid());

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
    for (i = 0; i< SO_SOURCES; i++){
        sources_pid_array[i] = 0;
    }
    source_processes_generator();

    taxis_pid_array = (pid_t *)malloc(SO_TAXI * sizeof(pid_t));
    for (i = 0; i < SO_TAXI; i++){
        taxis_pid_array[i] = 0;
    }
    taxi_processes_generator();

    master_signal_actions();
}

void read_parameters(){
    FILE *f;
    f = fopen(PARAMETERS , "r");

    if(f == NULL){
        printf("Errore  nell'apertura del file contenente i parametri\n");
		exit(EXIT_FAILURE);
    }
    
    fscanf(f, "%*s %d\n", &SO_HOLES);
    fscanf(f, "%*s %d\n", &SO_TOP_CELLS);
    fscanf(f, "%*s %d\n", &SO_SOURCES);
    fscanf(f, "%*s %d\n", &SO_TAXI);
    fscanf(f, "%*s %d\n", &SO_CAP_MIN);
    fscanf(f, "%*s %d\n", &SO_CAP_MAX);
    fscanf(f, "%*s %ld\n", &SO_TIMENSEC_MIN);
    fscanf(f, "%*s %ld\n", &SO_TIMENSEC_MAX);
    fscanf(f, "%*s %d\n", &SO_TIMEOUT);
    fscanf(f, "%*s %d\n", &SO_DURATION);
    fscanf(f, "%*s %d\n", &SO_INIT_REQUESTS_MIN);
    fscanf(f, "%*s %d\n", &SO_INIT_REQUESTS_MAX);

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
        }       
    }

    for (i = 0; i < SO_HEIGHT; i++){
        for(j = 0; j < SO_WIDTH; j++){
            if (master_map[i][j] == 0){
                master_cap_map[i][j] = 0;
            } else {
                 master_cap_map[i][j] = rand() % (SO_CAP_MAX + 1 - SO_CAP_MIN) + SO_CAP_MIN;
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
    msgqueue_id = msgget(IPC_PRIVATE, IPC_CREAT | IPC_EXCL | 0600);
    TEST_ERROR;
}

void semaphore_generator(){
    int offset = 0;
    //semaphore set for each cell; stores cell capability; matrix as an array
    sem_cells_cap_id = semget(IPC_PRIVATE, SO_HEIGHT * SO_WIDTH, IPC_CREAT | IPC_EXCL| 0600);
    TEST_ERROR;

    for(int x = 0; x < SO_HEIGHT; x++){
        for(int y = 0; y < SO_WIDTH; y++){
            arg.val = master_cap_map[x][y];
            semctl(sem_cells_cap_id, offset, SETVAL, arg);
            TEST_ERROR;
            offset++;
        } 
    } 

    //semaphore set used in order to sync processes and ipcs usage
    sem_sync_id = semget(IPC_PRIVATE, 2, IPC_CREAT | IPC_EXCL | 0600);
    TEST_ERROR;

    //processes sync semaphore
    arg.val = SO_SOURCES + SO_TAXI;
    semctl(sem_sync_id, 0, SETVAL, arg);
    TEST_ERROR;
    
    //returned values shd_memory mutex
    arg.val = 1;
    semctl(sem_sync_id, 1, SETVAL, arg);
    TEST_ERROR;
}

void shd_memory_generator(){
    //shd_mem used to pass master_map values to source processes
    shd_mem_to_source_id = shmget(IPC_PRIVATE, SO_HEIGHT * SO_WIDTH * sizeof(source_value_struct), IPC_CREAT | IPC_EXCL | 0600);
    TEST_ERROR;
    shd_mem_to_source = shmat(shd_mem_to_source_id, NULL, 0);
    TEST_ERROR;

    //shd_mem used to pass master_map values and timensec_map values to taxi processes
    shd_mem_to_taxi_id = shmget(IPC_PRIVATE, SO_HEIGHT * SO_WIDTH * sizeof(taxi_value_struct), IPC_CREAT | IPC_EXCL | 0600);
    TEST_ERROR;
    shd_mem_to_taxi = shmat(shd_mem_to_taxi_id, NULL, 0);
    TEST_ERROR;
    
    //shd_mem to return taxi stats
    shd_mem_returned_stats_id = shmget(IPC_PRIVATE, sizeof(returned_stats), IPC_CREAT | IPC_EXCL | 0600);
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
            (shd_mem_to_taxi+offset)->cell_timensec_value = rand() % (SO_TIMENSEC_MAX + 1 - SO_TIMENSEC_MIN) + SO_TIMENSEC_MIN;
            shd_mem_returned_stats->top_cells_map[x][y] = 0;
            offset++;
        }
    }
}

void source_processes_generator(){
    int i = -1, x, y, request_number, k;
    char *source_args[8];

    for (k = 0; k <= 7; k++){
        source_args[k] = malloc(30 * sizeof(char));
    }
    for (x = 0; x < SO_HEIGHT; x++){
        for (y = 0; y < SO_WIDTH; y++){
            if (master_map[x][y] == 2){
                i++;
                request_number = rand() % (SO_INIT_REQUESTS_MAX + 1 - SO_INIT_REQUESTS_MIN) + SO_INIT_REQUESTS_MIN;
                total_requests += request_number;
                sprintf(source_args[0], "%s", "Source");
                sprintf(source_args[1], "%d", x);
                sprintf(source_args[2], "%d", y);
                sprintf(source_args[3], "%d", msgqueue_id);
                sprintf(source_args[4], "%d", sem_sync_id);
                sprintf(source_args[5], "%d", shd_mem_to_source_id);
                sprintf(source_args[6], "%d", request_number);
                source_args[7] = NULL;

                switch(sources_pid_array[i] = fork()){
                    case -1:
                        fprintf(stderr,"Error #%03d: %s\n", errno, strerror(errno));
                        exit(EXIT_FAILURE);
                        break;
                    
                    case 0:                
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
    for (int k = 0; k <= 7; k++){
        free(source_args[k]);
    }
}

void taxi_processes_generator(){
    int k, i, x, y, generated;
    char *taxi_args[10];

    for (k = 0; k <= 9; k++){
        taxi_args[k] = malloc(30 * sizeof(char));
    }

    for (i = 0; i < SO_TAXI; i++){
        generated = 0;
        while(!generated){
            x = rand() % SO_HEIGHT;
            y = rand() % SO_WIDTH;
            if(master_map[x][y] != 0){
                sops.sem_num = (x * SO_WIDTH) + y; 
                sops.sem_op = -1;
                sops.sem_flg = IPC_NOWAIT;
                if(semop(sem_cells_cap_id, &sops, 1) == -1){
                    if(errno != EAGAIN && errno != EINTR){
                        TEST_ERROR;
                    }
                } else {
                    generated = 1;
                }
            }
        }
        sprintf(taxi_args[0], "%s", "Taxi");
        sprintf(taxi_args[1], "%d", x);
        sprintf(taxi_args[2], "%d", y);
        sprintf(taxi_args[3], "%d", SO_TIMEOUT);
        sprintf(taxi_args[4], "%d", msgqueue_id);
        sprintf(taxi_args[5], "%d", sem_sync_id);
        sprintf(taxi_args[6], "%d", sem_cells_cap_id);
        sprintf(taxi_args[7], "%d", shd_mem_to_taxi_id);
        sprintf(taxi_args[8], "%d", shd_mem_returned_stats_id);
        taxi_args[9] = NULL;
        switch(taxis_pid_array[i] = fork()){
            case -1:
                fprintf(stderr,"Error #%03d: %s\n", errno, strerror(errno));
                exit(EXIT_FAILURE);
                break;
                    
            case 0:
                execve("bin/Taxi", taxi_args, NULL);
	            fprintf(stderr, "%s: %d. Error #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
	            exit(EXIT_FAILURE);
                break;
                    
            default:
                break;
        }
    }
    for (k = 0; k <= 9; k++){
        free(taxi_args[k]);
    }
}

void taxi_processes_regenerator(pid_t to_regen){
    int i, x, y, generated = 0, k; 
    char *taxi_args[10];
    for (k = 0; k <= 9; k++){
        taxi_args[k] = malloc(30 * sizeof(char));
    }

    for(i = 0; i < SO_TAXI; i++){
        if(taxis_pid_array[i] == to_regen){
            break;
        }  
    }

    while(!generated){
        x = rand() % SO_HEIGHT;
        y = rand() % SO_WIDTH;
        if(master_map[x][y] != 0){
            sops.sem_num = (x * SO_WIDTH) + y; 
            sops.sem_op = -1;
            sops.sem_flg = IPC_NOWAIT;
            if(semop(sem_cells_cap_id, &sops, 1) == -1){
                if(errno != EAGAIN && errno != EINTR){
                    TEST_ERROR;
                }
            } else {
                generated = 1;
            }
        }
    }

    sprintf(taxi_args[0], "%s", "Taxi");
    sprintf(taxi_args[1], "%d", x);
    sprintf(taxi_args[2], "%d", y);
    sprintf(taxi_args[3], "%d", SO_TIMEOUT);
    sprintf(taxi_args[4], "%d", msgqueue_id);
    sprintf(taxi_args[5], "%d", sem_sync_id);
    sprintf(taxi_args[6], "%d", sem_cells_cap_id);
    sprintf(taxi_args[7], "%d", shd_mem_to_taxi_id);
    sprintf(taxi_args[8], "%d", shd_mem_returned_stats_id);
    taxi_args[9] = NULL;

    switch(taxis_pid_array[i] = fork()){
        case -1:
            fprintf(stderr,"Error #%03d: %s\n", errno, strerror(errno));
            exit(EXIT_FAILURE);
            break;
                    
        case 0:
            execve("bin/Taxi", taxi_args, NULL);
	        fprintf(stderr, "%s: %d. Error #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
	        exit(EXIT_FAILURE);
            break;
                    
        default:
            break;
    }
    for (k = 0; k <= 9; k++){
        free(taxi_args[k]);
    }
}

void master_signal_actions(){
    struct sigaction sa_alarm, sa_int;
    sigset_t mask;

    sigemptyset(&mask); 
    sigaddset(&mask, SIGALRM);
    sigaddset(&mask, SIGINT);

    sa_alarm.sa_mask = mask;
    sa_int.sa_mask = mask;

    sa_alarm.sa_handler = master_handle_signal;
    sa_alarm.sa_flags = SA_RESTART;
    
    sa_int.sa_handler = master_handle_signal; 
    sa_int.sa_flags = 0;
    
    sigaction(SIGINT, &sa_int, NULL);
    sigaction(SIGALRM, &sa_alarm, NULL);
}

void master_handle_signal(int signum){
    switch (signum){
        case SIGALRM:
            execution_time++;
            print_master_map();
            if(execution_time < SO_DURATION){
                alarm(1);
            } else {
                raise(SIGINT);
            }
            break;
        case SIGINT:
            processes_kill();
            alarm(0);
            break;
        default:
            printf("\nSegnale %d non gestito\n", signum);
            break;
    }
}

void print_master_map(){
    int x , y;
    
    printf("\nPID MASTER: %d\n", getpid());
    printf("Secondo: %d\n", execution_time);
    for ( x = 0; x < SO_HEIGHT; x++){
        for ( y = 0; y < SO_WIDTH; y++){
            printf(" \x1B[30m");
            switch (master_map[x][y]){
                case 0:
                    printf("\x1B[101m");
                    printf(" H");
                    printf("\x1B[0m");
                    break;
                case 1:
                    if ((master_cap_map[x][y] - semctl(sem_cells_cap_id, (x * SO_WIDTH) + y ,GETVAL)) > 0) {
                        printf("\x1b[43m");
                        printf(" %d", master_cap_map[x][y] - semctl(sem_cells_cap_id, (x * SO_WIDTH) + y ,GETVAL)); 
                        printf("\x1B[0m");
                    } else {
                        printf("\x1b[47m--\x1B[0m");
                    }
                    break;
                case 2:
                    if ((master_cap_map[x][y] - semctl(sem_cells_cap_id, (x * SO_WIDTH) + y ,GETVAL)) > 0) {
                        printf("\x1B[102m");
                        printf(" %d", master_cap_map[x][y] - semctl(sem_cells_cap_id, (x * SO_WIDTH) + y ,GETVAL)); 
                        printf("\x1B[0m");
                    } else {
                        printf("\x1B[102m");
                        printf(" S");
                        printf("\x1B[0m");
                    }
                    break;
                case -1:
                    //top cells



                default:
                    break;
            }
        }
        printf("\n\n");
    }
    printf("\x1B[0m");
}

void run(){
    pid_t terminatedPid = 0;
    int status = 0;
    
    //using a one sec alarm to print the map
    alarm(1);

    //collect stats from exit status of taxis or sources
    while ((terminatedPid = wait(&status)) > 0){
        if (WEXITSTATUS(status) == SOURCE_AUTOKILL){
            for(int i = 0; i < SO_SOURCES; i++){
                if(sources_pid_array[i] == terminatedPid){
                    sources_pid_array[i] = 0;
                    break;
                }
            }
        } else if(WEXITSTATUS(status) == TAXI_ABORTED){
            if (execution_time < SO_DURATION) {
                taxi_aborted++;
            } else {
                killed_t ++;
            }
        } else if (WEXITSTATUS(status) == TAXI_REPLACED){
            if (execution_time < SO_DURATION) {
                taxi_replaced++;
                taxi_processes_regenerator(terminatedPid);
            }
        } 
    }
}

void print_stats(){
    shdmem_return_sem_reserve(sem_sync_id);
    unresolved_trips = total_requests - shd_mem_returned_stats->trips_completed;
    printf("Statistiche della simulazione:\n");
    printf("\n\nEsecuzione conclusa. La durata totale è di %d secondi\n", execution_time);
    printf("Numero totale di richieste erogate: %d\n", total_requests);
    printf("Numero totale di viaggi eseguiti con successo: %d\n", shd_mem_returned_stats->trips_completed);
    printf("Numero totale di viaggi inevasi: %d\n", unresolved_trips);
    printf("Numero totale di taxi abortiti: %d\n", taxi_aborted);
    printf("Numero totale di taxi sostituiti: %d\n", taxi_replaced);
    printf("Numero totale di taxi uccisi in chiusura: %d\n", killed_t);
    printf("Il taxi con PID %d ha percorso più celle di tutti, per un totale di %d celle\n", shd_mem_returned_stats->pid_longest_trip, shd_mem_returned_stats->longest_trip);
    printf("Il taxi con PID %d ha impiegato un tempo maggiore di tutti gli altri, per un totale di %f secondi\n", shd_mem_returned_stats->pid_slowest_trip, shd_mem_returned_stats->slowest_trip/(double)1000000000);
    printf("Il taxi con PID %d ha raccolto il numero maggiore di clienti, per un totale di %d richieste\n", shd_mem_returned_stats->pid_max_trips_completed, shd_mem_returned_stats->max_trips_completed);
    shdmem_return_sem_release(sem_sync_id);
} 

void master_map_free(){
    for (int i = 0; i < SO_HEIGHT; i++){
        free(master_map[i]);
    }
    free(master_map);
}

void master_cap_map_free(){
    for (int i = 0; i < SO_HEIGHT; i++){
        free(master_cap_map[i]);
    }
    free(master_cap_map);
}

void master_process_array_free(){
    free(taxis_pid_array);
    free(sources_pid_array);
}

void master_free_all(){
    master_map_free();
    master_cap_map_free();
    master_process_array_free();
}

void free_ipcs(){
    //message queue
    while (msgctl(msgqueue_id, IPC_RMID, NULL)) {
		TEST_ERROR;
	}

    //semaphores
    for(int i = 0; i < SO_HEIGHT * SO_WIDTH; i++){
        semctl(sem_cells_cap_id, i, IPC_RMID);
    }
    semctl(sem_sync_id, 0, IPC_RMID);
    semctl(sem_sync_id, 1, IPC_RMID);

    //shared memory
    shmctl(shd_mem_to_source_id, IPC_RMID, NULL);
    shmctl(shd_mem_to_taxi_id, IPC_RMID, NULL);
    shmctl(shd_mem_returned_stats_id, IPC_RMID, NULL);

    if (shmdt(shd_mem_to_source) == -1) {
        fprintf(stderr, "%s: %d. Errore in shmdt #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (shmdt(shd_mem_to_taxi) == -1) {
        fprintf(stderr, "%s: %d. Errore in shmdt #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (shmdt(shd_mem_returned_stats) == -1) {
        fprintf(stderr, "%s: %d. Errore in shmdt #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void memory_cleanup(){
    free_ipcs();
    printf("IPCS deallocati\n");
    master_free_all();
    printf("Memoria dinamica deallocata\n");
}

void processes_kill(){
    int i;
    for (i = 0; i < SO_TAXI; i++){
        if (taxis_pid_array[i] > 0){
            kill(taxis_pid_array[i], SIGINT);
        }
    }

    for (i = 0; i < SO_SOURCES; i++){
        if (sources_pid_array[i] > 0){
            kill(sources_pid_array[i], SIGINT);
        }
    }
    printf("\nProcessi attivi eliminati\n");
}

void load_top_cells(){
    /* - code - */

    //try bucket method

    for (int x = 0; x < SO_HEIGHT; x++){
        for (int y = 0; y < SO_WIDTH; y++){
            if (1/*comparison with shdmem_topcellsmap*/){
                master_map[x][y] = TOP_CELLS_VALUE;
            }
        }
    }
}