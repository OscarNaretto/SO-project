#include "Common.h"
#define PARAMETERS "./ParametersLarge.txt"

void setup();
void read_parameters();
void maps_generator();
void master_map_initialization();
int can_be_placed(int x, int y);



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
int **SO_CAP_MAP;
long int **SO_TIMENSEC_MAP;


int main(int argc, char *argv[]){
    setup();

    //execution
    
    //print_map

    //free malloc and ipcs
}

void setup(){

    //parameters reading and check
    read_parameters();

    //maps generation
    maps_generator();

    //ipcs initialization

    //sources and taxis generation (move here source pid array and taxi pid array)
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

void maps_generator(){
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
    SO_CAP_MAP = (int **)malloc(SO_HEIGHT * sizeof(int *));
    if (SO_CAP_MAP == NULL){ allocation_error("Master", "SO_CAP_MAP"); }
    for (i = 0; i < SO_HEIGHT; i++){
        SO_CAP_MAP[i] = malloc(SO_WIDTH * sizeof(int));
        if (SO_CAP_MAP[i] == NULL){
            allocation_error("Master", "SO_CAP_MAP");
        } else {
            for(j = 0; j < SO_WIDTH; j++){
                SO_CAP_MAP[i][j] = rand() % (SO_CAP_MAX + 1 - SO_CAP_MIN) + SO_CAP_MIN;
            }
        }        
    }

    //memory allocation of maximum cell waiting time
    SO_TIMENSEC_MAP = (long int **)malloc(SO_HEIGHT * sizeof(long int *));
    if (SO_TIMENSEC_MAP == NULL){ allocation_error("Master", "SO_TIMENSEC_MAP"); }
    for (i = 0; i < SO_HEIGHT; i++){
        SO_TIMENSEC_MAP[i] = malloc(SO_WIDTH * sizeof(long int));
        if (SO_TIMENSEC_MAP[i] == NULL){
            allocation_error("Master", "SO_TIMENSEC_MAP");
        } else {
            for (j = 0; j < SO_WIDTH; j++){
                SO_TIMENSEC_MAP[i][j] = rand() % (SO_TIMENSEC_MAX + 1 - SO_TIMENSEC_MIN) + SO_TIMENSEC_MIN;
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
    if (x > 0 && y > 0 && map[x - 1][y - 1] == 0){ return 0;}
    if (x > 0 && map[x - 1][y] == 0){ return 0;}
    if (x > 0 && y < SO_WIDTH - 1 && map[x - 1][y + 1] == 0){ return 0;}
    if (x < SO_HEIGHT - 1 && y > 0 && map[x + 1][y - 1] == 0){ return 0;}
    if (x < SO_HEIGHT - 1 && map[x + 1][y] == 0){ return 0;}
    if (x < SO_HEIGHT - 1 && y < SO_WIDTH - 1 && map[x + 1][y + 1] == 0){ return 0;}
    if (y > 0 &&  map[x][y-1] == 0){ return 0;}
    if (y < SO_WIDTH - 1 && map[x][y + 1] == 0){ return 0;}

    return 1;
}





    