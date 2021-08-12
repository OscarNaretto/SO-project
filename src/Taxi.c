#include "Common.h"
#define EXIT_FAILURE_CUSTOM -1

int **taxi_map;
long int **TAXI_TIMENSEC_MAP;

int main(int argc, char const *argv[]){
    if(argc != /*9*/){
        fprintf(stderr, "\n%s: %d. NUMERO DI PARAMETRI ERRATO\n", __FILE__, __LINE__);
		exit(EXIT_FAILURE_CUSTOM);
    }

    /*code*/

    while (1) {
    /*cerca passeggero*/();
    }

    return(-1);
}

void taxi_signal_actions(){
    /* code */
}

void signal_handler(int signal){
    /* code */
}ß

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
/* metodo sostitutivo movimento taxi 
 controllare se si può utilizzare 
bool validMove(int fromRow, int fromCol, int toRow, int toCol)
{
    int i;

    // Attempt to move to the same cell
    if (fromRow == toRow && fromCol == toCol)
        return false;

    // Collision detection
    if (fromRow == toRow) {
        // Horizontal move
        if (fromCol < toCol) {
            // Move right
            for (i = fromCol + 1; i <= toCol; ++i)
                if (pieceAt(fromRow, i) != EMPTY)
                    return false;
        } else {
            // Move left
            for (i = fromCol - 1; i >= toCol; --i)
                if (pieceAt(fromRow, i) != EMPTY)
                    return false;
        }
    } else if (fromCol == toCol) {
        // Vertical move
        if (fromRow < toRow) {
            // Move down
            for (i = fromRow + 1; i <= toRow; ++i)
                if (pieceAt(i, fromCol) != EMPTY)
                    return false;
        } else {
            // Move up
            for (i = fromRow - 1; i >= toRow; --i)
                if (pieceAt(i, fromCol) != EMPTY)
                    return false;
        }
    } else {
        // Not a valid rook move (neither horizontal nor vertical)
        return false;
    }

    return true;
}
*/