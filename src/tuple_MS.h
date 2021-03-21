void show_tuple(unsigned n, unsigned tuple[][n]){
    indx b[N];
    unsigned i,j;
    for(i=0;i<8;++i){
        reset_board(b);
        for(j=0;j<n;++j){
            b[ tuple[i][j] ] = j+1;
        }
        print_board(b);
    }
}

unsigned tuples[ TUPLE_COUNT ][TUPLE_ISO][ TUPLE_N ];
unsigned tuplesLH [ TUPLE_ISO ] [ TUPLE_LH_N ];


void initialize_tuples(){
     
    unsigned k,i,j;
    // X X X X
    // X X O O
    // O O O O
    // O O O O
    tuples[0][0][0]=0;tuples[0][0][1]=1;tuples[0][0][2]=2;
    tuples[0][0][3]=3;tuples[0][0][4]=4;tuples[0][0][5]=5;
    // X X X X
    // X X O O
    // O O O O
    // O O O O
    tuples[1][0][0]=4;tuples[1][0][1]=5;tuples[1][0][2]=6;
    tuples[1][0][3]=7;tuples[1][0][4]=8;tuples[1][0][5]=9;
    // O O O O
    // X X X X
    // X X O O
    // O O O O
    tuples[2][0][0]=0;tuples[2][0][1]=1;tuples[2][0][2]=2;
    tuples[2][0][3]=4;tuples[2][0][4]=5;tuples[2][0][5]=6;
    // X X X 0
    // X X X O
    // O O O O
    // O O O O
    tuples[3][0][0]=4;tuples[3][0][1]=5;tuples[3][0][2]=6;
    tuples[3][0][3]=8;tuples[3][0][4]=9;tuples[3][0][5]=10;
    // X X X X
    // X O O O
    // O O O O
    // O O O O
    tuplesLH[0][0]=0;tuplesLH[0][1]=1;tuplesLH[0][2]=2;
    tuplesLH[0][3]=3;tuplesLH[0][4]=4;

    const unsigned isomorphic_map[7][16] = {{3u,2u,1u,0u,7u,6u,5u,4u,11u,10u,9u,8u,15u,14u,13u,12u},
                                    {3u,7u,11u,15u,2u,6u,10u,14u,1u,5u,9u,13u,0u,4u,8u,12u},
                                    {15u,11u,7u,3u,14u,10u,6u,2u,13u,9u,5u,1u,12u,8u,4u,0u},
                                    {15u,14u,13u,12u,11u,10u,9u,8u,7u,6u,5u,4u,3u,2u,1u,0u},
                                    {12u,13u,14u,15u,8u,9u,10u,11u,4u,5u,6u,7u,0u,1u,2u,3u},
                                    {12u,8u,4u,0u,13u,9u,5u,1u,14u,10u,6u,2u,15u,11u,7u,3u},
                                    {0u,4u,8u,12u,1u,5u,9u,13u,2u,6u,10u,14u,3u,7u,11u,15u}};
    
    for(k=0;k<TUPLE_COUNT;++k)
        for(i=1;i<TUPLE_ISO;++i)
            for(j=0;j<TUPLE_N;++j)
                tuples[k][i][j] = isomorphic_map[i-1][ tuples[k][0][j] ];

    for(i=1;i<TUPLE_ISO;++i)
        for(j=0;j<TUPLE_LH_N;++j)
            tuplesLH[i][j] = isomorphic_map[i-1][ tuplesLH[0][j] ];
}

unsigned get_LT_index(indx board[]){
    unsigned i, index = 0;
    static const int shift[5] = {0,2,4,6,8};
    for(i=0;i<N;++i){
        if( board[i] >= 9 && board[i]<14 ){
            index += 1u << shift[ board[i]-9 ];
        }
    }
    return index;
}
unsigned get_LT_index_bb(bitboard * b){
    unsigned i, index = 0;
    unsigned bv;
    static const int shift[5] = {0,2,4,6,8};
    for(i=0;i<N;++i){
        bv = bitboard_get(b,i);
        if( bv >= 9 && bv<14 ){
            index += 1u << shift[ bv-9 ];
        }
    }
    return index;
}
/*
unsigned get_M_index(indx board[]){
    unsigned B = 0,M = 0,L = 0,index;
    int i;
    bool not_last_row, not_last_column;
    for(i=N;i>=0;--i){
        if (board[i] == 0){ B++;continue; }
        not_last_row = (i>3);
        not_last_column = (i%4);
        switch( board[i] ){
            case 1:
            case 2:
                if (not_last_column && (board[i]+board[i-1] == 3))
                    M++;
                if (not_last_row && (board[i]+board[i-4] == 3))
                    M++;
                break;
            default:
                if (not_last_column && (board[i] == board[i-1]))
                    M++;
                if (not_last_row && (board[i] == board[i-4]))
                    M++;
                
                if (not_last_column && (abs(board[i]-board[i-1])==1) && (board[i-1]!=2))
                    L++;
                if (not_last_row && (abs(board[i]-board[i-4])==1) &&  (board[i-4]!=2))
                    L++;
        }
    }
    index = (B << 10) | (M << 5) | L;
    return index;
}
*/
typedef struct _network {
    weight wtable[TUPLE_COUNT][WEIGHT_SLOT];   // 4 * 16^6 = 4 * 16777216
    weight wtableLH[WEIGHT_SLOT];              // 16^6 = 16777216
    weight wtableLT[WEIGHT_LT];                // 2^10 = 1024
} network;

#define NETWORK_COUNT 3

network Networks[NETWORK_COUNT];
char *network_names[NETWORK_COUNT] = {"before 1536","after 1536","after 3072"};
unsigned current_network = 0;

void init_weight(){
    #define wtable Networks[0].wtable
    #define wtableLH Networks[0].wtableLH
    #define wtableLT Networks[0].wtableLT


    memset(wtable, 0, sizeof(weight) * TUPLE_COUNT * WEIGHT_SLOT);
    memset(wtableLH, 0, sizeof(weight)*WEIGHT_SLOT);
    memset(wtableLT, 0, sizeof(weight)*WEIGHT_LT);
}
void load_weight(const char* filename){
    #define wtable Networks[current_network].wtable
    #define wtableLH Networks[current_network].wtableLH
    #define wtableLT Networks[current_network].wtableLT

    FILE *fp;
    unsigned n_buffer, i, j;
    uint64_t n64_buffer;
    fp = fopen(filename, "rb");
    if(!fp){
        printf("File error!");
        return;
    }
    fread(&n_buffer, sizeof(n_buffer), 1, fp);
    if (n_buffer == (unsigned) ( WEIGHT_TABLE_COUNT ))
        printf("Weight table match!\n");
    else{
        printf("Weight table not match!\n It's %u.\n",n_buffer);
        return;
    }
    for(i=0; i< TUPLE_COUNT; ++i){
        fread(&n64_buffer, sizeof(n64_buffer), 1, fp);
        if (n64_buffer == WEIGHT_SLOT)
            printf("Weight slots number match!\n");
        else{
            printf("Weight slots number not match!\n");
            return;
        }
        fread(wtable[i] , sizeof(weight), WEIGHT_SLOT, fp);
    }
    // load LH
    fread(&n64_buffer, sizeof(n64_buffer), 1, fp);
    if (n64_buffer == WEIGHT_SLOT)
        printf("Weight LH slots number match!\n");
    else{
        printf("Weight LH slots number not match!\n");
        return;
    }
    fread(wtableLH , sizeof(weight), WEIGHT_SLOT, fp);
    // load LT
    fread(&n64_buffer, sizeof(n64_buffer), 1, fp);
    if (n64_buffer == WEIGHT_LT)
        printf("Weight LT slots number match!\n");
    else{
        printf("Weight LT slots number not match!\n");
        return;
    }
    fread(wtableLT , sizeof(weight), WEIGHT_LT, fp);
    fclose(fp);
}
void save_weight(const char* filename){
    #define wtable Networks[current_network].wtable
    #define wtableLH Networks[current_network].wtableLH
    #define wtableLT Networks[current_network].wtableLT


    FILE *fp;
    unsigned n_buffer, i, j;
    uint64_t n64_buffer;
    printf("Starting to save weight! %s\n",filename);
    fp = fopen(filename, "wb+");
    if(!fp){
        printf("File error!");
        return;
    }
    // write number of tuples(weight tables) (not including isomorphic tuple)
    // 4 6-tuple + Large Tile + L-Hint
    // Weight tables number -> 32 bits
    // Weight slots number -> 64 bits

    printf("Save 6 Tuple...");
    n_buffer = (unsigned) ( WEIGHT_TABLE_COUNT );

    fwrite(&n_buffer, sizeof(n_buffer), 1, fp);
    for(i=0; i< TUPLE_COUNT; ++i){
        n64_buffer = (uint64_t) WEIGHT_SLOT;
        fwrite(&n64_buffer, sizeof(n64_buffer), 1, fp);
        fwrite(wtable[i] , sizeof(weight), WEIGHT_SLOT, fp);
    }
    printf("Finished\n");
    //save LH
    printf("Save LH...");
    n64_buffer = (uint64_t) WEIGHT_SLOT;
    fwrite(&n64_buffer, sizeof(n64_buffer), 1, fp);
    fwrite(wtableLH , sizeof(weight), WEIGHT_SLOT, fp);
    printf("Finished\n");
    //save LT
    printf("Save LT...");
    n64_buffer = (uint64_t) WEIGHT_LT;
    fwrite(&n64_buffer, sizeof(n64_buffer), 1, fp);
    fwrite(wtableLT , sizeof(weight), WEIGHT_LT, fp);
    printf("Finished\n");
    fclose(fp);

}
void check_weight(){
    #define wtable Networks[current_network].wtable
    #define wtableLH Networks[current_network].wtableLH
    #define wtableLT Networks[current_network].wtableLT
    //check weight
    unsigned i,j;
    weight maxw,minw;
    for(i=0;i<TUPLE_COUNT;++i){
        maxw = minw = wtable[i][0];
        for(j=1;j<WEIGHT_SLOT;++j){
            if(wtable[i][j]>maxw)
                maxw = wtable[i][j];
            if(wtable[i][j]<minw)    
                minw = wtable[i][j];
        }
        printf("In Tuple #%d: Max Weight = %f, Min Weight = %f \n",i, maxw, minw);
    }

    maxw = minw = wtableLT[0];
    for(j=1;j<WEIGHT_LT;++j){
        if(wtableLT[j]>maxw)
            maxw = wtableLT[j];
        if(wtableLT[j]<minw)    
            minw = wtableLT[j];
    }
    printf("In LT Feature: Max Weight = %f, Min Weight = %f \n", maxw, minw);

    maxw = minw = wtableLH[0];
    for(j=1;j<WEIGHT_SLOT;++j){
        if(wtableLH[j]>maxw)
            maxw = wtableLH[j];
        if(wtableLH[j]<minw)    
            minw = wtableLH[j];
    }
    printf("In LH Feature: Max Weight = %f, Min Weight = %f \n", maxw, minw);
/*
    maxw = minw = wtableM[0];
    for(j=1;j<WEIGHT_M;++j){
        if(wtableM[j]>maxw)
            maxw = wtableM[j];
        if(wtableM[j]<minw)    
            minw = wtableM[j];
    }
    printf("In M Feature: Max Weight = %f, Min Weight = %f \n", maxw, minw);
*/
}